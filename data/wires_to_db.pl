#!/usr/bin/perl -w

# This script takes one file in stdin:
# the database of wires
# it generates a .ini file for reading by the wiring.c code

use strict;
use integer;

my $invert_y_axis = shift @ARGV;

# We must fill in some things
my %dx;
my %dy;
# These will be merged at some point
my %otherend;
my %otherends;
my %type;
my %dir;
my %sit;

#wires ultimately are numbered
my %nums;
my @wires;


#define WIRE_NUM_NEUTRAL 32

my @det_dir = ( "WIRE_DIRECTION_NEUTRAL",
		"N",  "WN",  "NW",  "W",  "WS",   "SW",  "S",   "SE",
		"ES",  "E",   "EN",   "NE",  "DN",   "UP" );

my @det_sit = ( "WIRE_SITUATION_NEUTRAL",
		"BEG", "A", "B", "MID", "C", "D", "END",
		"ZERO", "ONE", "TWO", "THREE");

my @det_type = ( "WIRE_TYPE_NEUTRAL",
		 "DOUBLE", "HEX",    "OMUX",    "BX", "BY",
		 "BX_PINWIRE", "BY_PINWIRE",    "CE", "CIN",    "CLK",
		 "COUT",    "DX", "DY",    "F1", "F2", "F3", "F4",
		 "F1_PINWIRE", "F2_PINWIRE", "F3_PINWIRE", "F4_PINWIRE",
		 "F5", "FX", "FXINA", "FXINB",    "G1", "G2", "G3",
		 "G4",    "G1_PINWIRE", "G2_PINWIRE", "G3_PINWIRE",
		 "G4_PINWIRE",    "GCLK",    "GCLKC_GCLKB",
		 "GCLKC_GCLKL", "GCLKC_GCLKR", "GCLKC_GCLKT",
		 "GCLKH_GCLK_B",    "GCLKH_GCLK",    "LH", "LV",
		 "O0", "O1", "O2", "O3", "O4", "O5", "O6", "O7",
		 "I0", "I1", "I2", "I3", "I4", "I5", "I6", "I7",
		 "IQ1", "IQ2",
		 "SHIFTIN", "SHIFTOUT", "SR",    "TBUF", "TBUS",
		 "TI", "TOUT", "TS",    "VCC_PINWIRE",    "WF1_PINWIRE",
		 "WF2_PINWIRE", "WF3_PINWIRE", "WF4_PINWIRE",
		 "WG1_PINWIRE", "WG2_PINWIRE", "WG3_PINWIRE",
		 "WG4_PINWIRE",    "X", "XB", "XQ",    "Y", "YB", "YQ",
		 "NR_WIRE_TYPE");

my %type_h;
my %dir_h;
my %sit_h;

&array_to_hash(\%type_h,\@det_type);
&array_to_hash(\%dir_h,\@det_dir);
&array_to_hash(\%sit_h,\@det_sit);

#convert these to hashes




while (<STDIN>) {
    chomp;
    my $wire = $_;
    my $mdx = 0;
    my $mdy = 0;
    my $end;

    my $wtype = "WIRE_TYPE_NEUTRAL";
    my $wsit = "WIRE_SITUATION_NEUTRAL";
    my $wdir = "WIRE_DIRECTION_NEUTRAL";

    # all kind of wires ! This is sooooo cool
    # doubles & hexes
    if (m/^(.*_?)([ENSW])([26])(BEG|A|B|MID|C|D|END)([0-9])$/) {
	my $prefix = $1;
	my $orientation = $2;
	my $length = $3;
	my $step = 0;
	my $middle = $4;
	my $rank = $5;

	if ($length == 6) {
	    # HEX
	    # A, B, C, D only makes sense with hex
	    my %step_ref = ("BEG" => 0,
			    "A" => 1,
			    "B" => 2,
			    "MID" => 3,
			    "C" => 4,
			    "D" => 5,
			    "END" => 6);
	    $step = $step_ref{$middle};
	    $wtype = "HEX";
	} else {
	    #DOUBLE
	    my %step_ref = ("BEG" => 0,
			    "MID" => 1,
			    "END" => 2);
	    $step = $step_ref{$middle};
	    $wtype = "DOUBLE";
	}

	$wsit = $middle;

	$mdx = &dx_of_dir($orientation);
	$mdy = &dy_of_dir($orientation);
	$mdx = $mdx * $step;
	$mdy = $mdy * $step;

	if ($prefix =~ /TBTERM_/) {
	    if ($mdy < 0) {
		$mdy -= 1;
	    }
	    else {
		$mdy += 1;
	    }
	    $end = $orientation.$length."BEG".$rank;
	} elsif ($prefix =~ /LRTERM_/) {
	    if ($mdx < 0) {
		$mdx -= 1;
	    }
	    else {
		$mdx += 1;
	    }
	    $end = $orientation.$length."BEG".$rank;
	} elsif ($prefix =~ /LR_IOIS_/) {
	    $end = $orientation.$length."BEG".$rank;
	} elsif ($prefix =~ /TB_IOIS_/) {
	    $end = $orientation.$length."BEG".$rank;
	} else {
	    $end = $prefix.$orientation.$length."BEG".$rank;
	}

	#add endpoints in very special case for now
	#It would be much better to have this into the array directly,
	#with a correct ordering. However, this will only be possible
	#once the array does not have to be sorted -- which will happen
	#as soon as we write a fast name-to-index function
	if ($wire =~ /(N|S)6BEG([0-9]*)/) {
	    my $dir = $1;
	    my $rank = $2;
	    my @endpoints = ("TBTERM_${dir}6BEG${rank}",
			     "TBTERM_${dir}6A${rank}",
			     "TBTERM_${dir}6B${rank}", "TBTERM_${dir}6MID${rank}",
			     "TBTERM_${dir}6C${rank}", "TBTERM_${dir}6D${rank}",
			     "TBTERM_${dir}6END${rank}");
	    &register_wire_corked($wire, $mdx, $mdy, $end, $wtype, $wsit,
				  $orientation, \@endpoints);
	} elsif ($wire =~ /(E|W)6BEG([0-9]*)/) {
	    my $dir = $1;
	    my $rank = $2;
	    my @endpoints = ("LRTERM_${dir}6BEG${rank}",
			     "LRTERM_${dir}6A${rank}", "LRTERM_${dir}6B${rank}",
			     "LRTERM_${dir}6MID${rank}", "LRTERM_${dir}6C${rank}",
			     "LRTERM_${dir}6D${rank}", "LRTERM_${dir}6END${rank}");
	    &register_wire_corked($wire, $mdx, $mdy, $end, $wtype, $wsit,
				  $orientation, \@endpoints);
	} else {
	    &register_wire($wire, $mdx, $mdy, $end, $wtype, $wsit,
	    $orientation);
	}
	next;
    }

    #double / hexes with some jumping around
    if (m/^(.*_?)([ENSW])([26])END_([ENSW])([0-9])$/) {
	my $prefix = $1;
	my $orientation = $2;
	my $suborientation = $4;
	my $length = $3;
	my $step = 0;
	my $rank = $5;

	$wsit = "END";

	$step = $length;

	if ($length == 6) {
	    # HEX
	    $wtype = "HEX";
	} else {
	    #DOUBLE
	    $wtype = "DOUBLE";
	}

	$mdx = &dx_of_dir($orientation);
	$mdy = &dy_of_dir($orientation);
	$mdx = $mdx * $step;
	$mdy = $mdy * $step;

	my $m2dx = &dx_of_dir($suborientation);
	my $m2dy = &dy_of_dir($suborientation);
	$mdx += $m2dx;
	$mdy += $m2dy;

	$end = $prefix.$orientation.$length."BEG".$rank;
	&register_wire($wire, $mdx, $mdy, $end, $wtype, $wsit, $orientation);
	next;
    }

    #start omuxes
    if (m/^OMUX[0-9]*$/) {
	#well, there's the start, aren't they
	$wtype = "OMUX";
	&register_wire($wire, 0, 0, $wire, $wtype, $wsit, $wdir);
	next;
    }

    #end omuxes
    if (m/^OMUX_([NWSE]*)([0-9]*)$/) {
	my $orientation = $1;
	my $rank = $2;

	my $mdx = &dx_of_dir($orientation);
	my $mdy = &dy_of_dir($orientation);

	$wtype = "OMUX";

	$end = "OMUX".$rank;
	&register_wire($wire, $mdx, $mdy, $end, $wtype, $wsit, $orientation);
	next;
    }

    my @coucou = ( "ZERO", "ONE", "TWO", "THREE" );
    #many others
    if (m/^([FG])([1-4])_B([0-3])/) {
	#local wires
	my $wiretype = $1 . $2;
	my $index = $3;
	my $wiresit = $coucou[$index];
#	print STDERR "Warning, wiresit $wiresit\n";
	# Direction; input, output
	&register_wire($wire, 0, 0, $wire, $wiretype, $wiresit, $wdir);
	next;
    }

    if (m/^(CE)_B([0-3])/) {
	#local wires
	my $wiretype = $1;
	my $index = $2;
	my $wiresit = $coucou[$index];
	# Direction; input, output
	&register_wire($wire, 0, 0, $wire, $wiretype, $wiresit, $wdir);
	next;
    }

    if (m/^(SR)([0-3])/) {
	#local wires
	my $wiretype = $1;
	my $index = $2;
	my $wiresit = $coucou[$index];
	# Direction; input, output
	&register_wire($wire, 0, 0, $wire, $wiretype, $wiresit, $wdir);
	next;
    }

    if (m/^([XY])([BQ]?)([0-3])/) {
	#local wires
	my $wiretype = $1 . $2;
	&register_wire($wire, 0, 0, $wire, $wiretype, $wsit, $wdir);
	next;
    }

    if (m/^B([XY])([0-3])/) {
	#local wires
	my $wiretype = "B" . $1;
	&register_wire($wire, 0, 0, $wire, $wiretype , $wsit, $wdir);
	next;
    }

    if (m/^(CLK)([0-3])/) {
	#local wires
	&register_wire($wire, 0, 0, $wire, "CLK", $wsit, $wdir);
	next;
    }

    if (m/^(I|O)([0-7])(.*)/) {
	#IO wires
	my $wiretype = $1 . $2;
	&register_wire($wire, 0, 0, $wire, $wiretype, $wsit, $wdir);
	next;
    }

    if (m/^(I_Q)([12])([0-7])(.*)/) {
	#IQ wires, whatever these are
	my $wiretype = "IQ" . $2;
	&register_wire($wire, 0, 0, $wire, $wiretype, $wsit, $wdir);
	next;
    }

    if (m/^.*_?L([VH])([0-9]*)/) {
	my $mydir = $1;
	my $mysit = $2;
	my $mytype= "L" . $mydir;
	&register_wire($wire, 0, 0, $wire, $mytype, $wsit, $wdir);
	#long wires. Not sure what to do: fall through
	#Patch situation, which encodes the index
	$sit{$wire} = $mysit;
	next;
    }

    print STDERR "Warning, unmatched wire $wire\n";
    &register_wire($wire, 0, 0, $wire, $wtype, $wsit, $wdir);
}

sub dx_of_dir() {
    my $dir = shift;

    if ($dir =~ m/E/) {
	return 1;
    }
    if ($dir =~ m/W/) {
	return -1;
    }
    return 0;
}

sub dy_of_dir() {
    my $dir = shift;

    if ($dir =~ m/S/) {
	return 1;
    }
    if ($dir =~ m/N/) {
	return -1;
    }
    return 0;
}

#sort the wires in string order
@wires = sort (@wires);

#assign them their number
my $i = 0;
my $wire;
foreach $wire (@wires) {
    $nums{$wire} = $i;
    $i++;
}

&dump_ini;

sub register_wire_corked() {
    my $wire = shift;
    my $dx = shift;
    my $dy = shift;
    my $end = shift;

    my $wtype = shift;
    my $wsit = shift;
    my $wdir = shift;

    my $ends = shift;

    &register_wire($wire, $dx, $dy, $end, $wtype, $wsit, $wdir);
    $otherends{$wire} = $ends;
}

sub register_wire() {
    my $wire = shift;
    my $dx = shift;
    my $dy = shift;
    my $end = shift;

    my $wtype = shift;
    my $wsit = shift;
    my $wdir = shift;

    # in case we found nothing, only give a number to the thing and backoff
    push @wires, $wire;
    $dx{$wire} = $dx;
    $dy{$wire} = $dy;
    $otherend{$wire} = $end;

    # XXX These need to be filled
    $type{$wire} = $type_h{$wtype};
    $dir{$wire} = $dir_h{$wdir};
    $sit{$wire} = $sit_h{$wsit};
}

sub array_to_hash() {
    my $r_hash = shift;
    my @array = @{(shift)};
    my $i = 0;
    my $elem;

    foreach $elem (@array) {
	$$r_hash{$elem} = $i;
#	print "$elem has number $i";
	$i++;
    }
}

#TODO: add some information there, about the min bit, the max bit,
#the red/black coloring, what else -- maybe in C.

#we use the C preprocessor to format the file. So, don't try to be
#formatting-clever here, dump the data wrapped in a macro, and let CPP
#or m4 handle the rest

sub tonum {
    my $val = shift;
    if (defined $nums{$val}) {
	return "$nums{$val}";
    } else {
	return "-1";
    }
}

sub dump_ini {
    my $output;
    for $output (@wires) {
	my $mydy = $dy{$output};
	my $nwires = 0;

	if (defined $otherends{$output}) {
	    my @ends=@{$otherends{$output}};
	    $nwires += $#ends;
	}

	if ($invert_y_axis) {
	    $mydy = - $mydy;
	}

	print "_WIRE_ENTRY($output, $nums{$output}, $dx{$output}, ";
	print "$mydy, ";
	print "$nums{$otherend{$output}}, ";
	print "$nwires, ";

	print "_WIRE_ENDS(";
	if (defined $otherends{$output}) {
	    my @ends=@{$otherends{$output}};
	    print join ", ", map { &tonum($_) } @ends;
	}
	print "), ";

	print "$type{$output}, $dir{$output}, $sit{$output}";

	print ")\n";
    }
}
