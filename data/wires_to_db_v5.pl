#!/usr/bin/perl -w

# This script takes one file in stdin:
# the database of wires
# it generates a .ini file for reading by the wiring.c code

use strict;
use integer;

my $prefix = shift @ARGV;

# We must fill in some things
my %dx;
my %dy;
my %otherend;
my %type;
my %dir;
my %sit;

#wires ultimately are numbered
my %nums;

#define WIRE_NUM_NEUTRAL 32

my @det_dir = ( "WIRE_DIRECTION_NEUTRAL",
		"N",  "WN",  "NW",  "W",  "WS",   "SW",  "S",   "SE",
		"ES",  "E",   "EN",   "NE",  "DN",   "UP" );

my @det_sit = ( "WIRE_SITUATION_NEUTRAL",
		"BEG", "A", "B", "MID", "C", "D", "END" );

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
		 "SHIFTIN", "SHIFTOUT", "SR",    "TBUF", "TBUS",
		 "TI", "TOUT", "TS",    "VCC_PINWIRE",    "WF1_PINWIRE",
		 "WF2_PINWIRE", "WF3_PINWIRE", "WF4_PINWIRE",
		 "WG1_PINWIRE", "WG2_PINWIRE", "WG3_PINWIRE",
		 "WG4_PINWIRE",    "X", "XB", "XQ",    "Y", "YB", "YQ",
		 "NR_WIRE_TYPE");

my %type_h;
my %dir_h;
my %sit_h;

# my %patchtable = (
# 		  "ER2BEG0" => "ER2BEG_S0",
# 		  "NL2BEG0" => "NL2BEG_S0",
# 		  "NR5BEG2" => "NR5BEG_N2",
# 		  "SL2BEG2" => "SL2BEG_N2",
# 		  "WL5BEG0" => "WL5BEG_S0",
# 		  "WR2BEG2" => "WR2BEG_N2",
# );

my %patchtable;
my %patchtable_dx;
my %patchtable_dy;


&array_to_hash(\%type_h,\@det_type);
&array_to_hash(\%dir_h,\@det_dir);
&array_to_hash(\%sit_h,\@det_sit);

#convert these to hashes

sub standard_wire() {
    my $wire = shift;

    my $mdx = 0;
    my $mdy = 0;
    my $wtype = "WIRE_TYPE_NEUTRAL";
    my $wsit = "WIRE_SITUATION_NEUTRAL";
    my $wdir = "WIRE_DIRECTION_NEUTRAL";


    $wire =~ m/^([ENSW])([LRENSW])([25])(BEG|MID|END)([0-2])$/;

    my $mainor = $1;
    my $subor = $2;
    my $length = $3;
    my $middle = $4;
    my $rank = $5;

    my %step_first = ("BEG" => 0,
		      "MID" => 1,
		      "END" => 1);

    my %step_end = ("BEG" => 0,
		    "MID" => 0,
		    "END" => 1);

    my %step_first_val = (5 => 3,
			  2 => 1);
    my %step_last_val = (5 => 2,
			 2 => 1);

    my $first_move = $step_first{$middle} * $step_first_val{$length};
    my $second_move = $step_end{$middle} * $step_last_val{$length};

    # OK, now let's try to see where these fit
    if ($mainor =~ m/E/) {
	$mdx -= $first_move;
	if ($subor =~ m/R/ || $subor =~ m/L/) {
	    $mdx -= $second_move;
	}
    } elsif ($mainor =~ m/W/) {
	$mdx += $first_move;
	if ($subor =~ m/R/ || $subor =~ m/L/) {
	    $mdx += $second_move;
	}
    } elsif ($mainor =~ m/N/) {
	$mdy -= $first_move;
	if ($subor =~ m/R/ || $subor =~ m/L/) {
	    $mdy -= $second_move;
	}
    } elsif ($mainor =~ m/S/) {
	$mdy += $first_move;
	if ($subor =~ m/R/ || $subor =~ m/L/) {
	    $mdy += $second_move;
	}
    }

    # OK, now let's try to see where these fit
    if ($subor =~ m/E/) {
	$mdx -= $second_move;
    } elsif ($subor =~ m/W/) {
	$mdx += $second_move;
    } elsif ($subor =~ m/N/) {
	$mdy -= $second_move;
    } elsif ($subor =~ m/S/) {
	$mdy += $second_move;
    }

    # XXX Not necessarily
    my $end = $mainor.$subor.$length."BEG".$rank;
    &register_wire($wire, $mdx, $mdy, $end, $wtype, $wsit, $mainor);
}


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
    # doubles & fifth
    if (m/^([ENSW])([LRENSW])([25])(BEG|MID|END)([0-2])$/) {
	&standard_wire($wire);
	next;
    }

    # To be continued
    if (m/^([ENSW])([LRENSW])([25])(BEG|MID|END)_([ENSW])([0-2])$/) {
	my $mainor = $1;
	my $subor = $2;
	my $length = $3;
	my $middle = $4;
	my $subsubor = $5;
	my $rank = $6;
	#Something like this
	my $stdwire = $mainor.$subor.$length.$middle.$rank;

	if ($middle =~ m/BEG/) {
	    print STDERR "Inserting patch $wire for $stdwire\n";
	    $patchtable{$stdwire} = $wire;
	    $patchtable_dx{$stdwire} = 0;
	    $patchtable_dy{$stdwire} = 0;

	    if ($subsubor =~ m/E/) {
		$patchtable_dx{$stdwire} = 1;
	    } elsif ($subsubor =~ m/W/) {
		$patchtable_dx{$stdwire} = -1;
	    } elsif ($subsubor =~ m/N/) {
		$patchtable_dy{$stdwire} = 1;
	    } elsif ($subsubor =~ m/S/) {
		$patchtable_dy{$stdwire} = -1;
	    }

	    &register_wire($wire, 0, 0, $wire, $wtype, $wsit, $mainor);
	    next;
	}

	&standard_wire($stdwire);
	# now, inverted wrt the standard thing, as the sublocation gives
	# the location of the startpoint wrt the standard location
	if ($subsubor =~ m/E/) {
	    $mdx += 1;
	} elsif ($subsubor =~ m/W/) {
	    $mdx -= 1;
	} elsif ($subsubor =~ m/N/) {
	    $mdy += 1;
	} elsif ($subsubor =~ m/S/) {
	    $mdy -= 1;
	}

	&register_wire($wire, $dx{$stdwire}+$mdx, $dy{$stdwire}+$mdy, $otherend{$stdwire},
		       $wtype, $wsit, $mainor);
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

    #many others
    if (m/^([FG])([1-4])_B([0-3])/) {
	#local wires
	&register_wire($wire, 0, 0, $wire, $wtype, $wsit, $wdir);
	next;
    }

    if (m/^([XY])([BQ]?)([0-3])/) {
	#local wires
	&register_wire($wire, 0, 0, $wire, $wtype, $wsit, $wdir);
	next;
    }

    if (m/^B([XY])([0-3])/) {
	#local wires
	&register_wire($wire, 0, 0, $wire, $wtype, $wsit, $wdir);
	next;
    }

    if (m/^.*_?L([VH])([0-9]*)/) {
	&register_wire($wire, 0, 0, $wire, $wtype, $wsit, $wdir);
	#long wires. Not sure what to do: fall through
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
my @wires = sort (keys %otherend);

#assign them their number
my $i = 0;
my $wire;
foreach $wire (@wires) {
    $nums{$wire} = $i;
    $i++;
}

&dump_ini;

sub register_wire() {
    my $wire = shift;
    my $dx = shift;
    my $dy = shift;
    my $end = shift;

    my $wtype = shift;
    my $wsit = shift;
    my $wdir = shift;

    # Type is not redundant
    my $type = shift;
    # dir is redundant with dx, dy
    my $dir = shift;
    # sit is redundant with type + dx, dy, I think
    my $sit = shift;

    # in case we found nothing, only give a number to the thing and backoff
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

sub dump_ini {
    my $output;
    for $output (@wires) {
	my $endpoint = $otherend{$output};
	my $nendpoints = 0;
	print "_WIRE_ENTRY($output, $nums{$output}, ";
	if (defined $nums{$endpoint}) {
	    my $newdy = - $dy{$output};
	    my $newdx = - $dx{$output};
	    print "$newdx, $newdy, $nums{$endpoint}, ";
	}
	else {
	    my $patch = $patchtable{$endpoint};
	    print STDERR "Otherend $endpoint not defined\n";
	    if (defined $patch) {
		print STDERR "Patching for $patch\n";
		my $newdx = - ($dx{$output} + $patchtable_dx{$endpoint});
		my $newdy = - ($dy{$output} + $patchtable_dy{$endpoint});
		print "$newdx, $newdy, $nums{$patch}, ";
	    } else {
		print STDERR "No patch found !\n";
		print "$nums{$output}, ";
	    }
	}

	print "$nendpoints, _WIRE_ENDS(), ";
	print "$type{$output}, $dir{$output}, $sit{$output})\n";
    }
}
