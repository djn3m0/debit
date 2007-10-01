#! /usr/bin/perl -w

use strict;
require "../intervals.pl";

# This script generates chip_control and chip_data databases for the
# virtex-II family

# It could be worse, but not much more... DSPs-enabled V4 and V2P will be fun !
# Type => num
my $i = 0;

my $y_offset = 2;
my $x_offset = 2;

my %C_enum = (
	      SITE_TYPE_NEUTRAL => $i++,
	      CLB => $i++,
	      TTERM => $i++,
	      LTERM => $i++,
	      BTERM => $i++,
	      RTERM => $i++,
	      TLTERM => $i++,
	      LTTERM => $i++,
	      LBTERM => $i++,
	      BLTERM => $i++,
	      BRTERM => $i++,
	      RBTERM => $i++,
	      RTTERM => $i++,
	      TRTERM => $i++,
	      TTERMBRAM => $i++,
	      BTERMBRAM => $i++,
	      TIOI => $i++,
	      LIOI => $i++,
	      BIOI => $i++,
	      RIOI => $i++,
	      TIOIBRAM => $i++,
	      BIOIBRAM => $i++,
	      BRAM => $i++,
	      BM => $i++,
	      TL => $i++,
	      BL => $i++,
	      BR => $i++,
	      TR => $i++,
	      M  => $i++,
	      CLKT => $i++,
	      CLKB => $i++,
	      GCLKC => $i++,
	      GCLKH => $i++,
	      GCLKHBRAM => $i++,
	      NR_SITE_TYPE => $i++
	      );

# First the raw data, width and height
# Width -- in clb
my %clbwidth = (
    "40" => 8,
    "80" => 8,
    "250" => 16,
    "500" => 24,
    "1000" => 32,
    "1500" => 40,
    "2000" => 48,
    "3000" => 56,
    "4000" => 72,
    "6000" => 88,
    "8000" => 104
		);

my %flr = (
    "40" => 26,
    "80" => 46,
    "250" => 66,
    "500" => 86,
    "1000" => 106,
    "1500" => 126,
    "2000" => 146,
    "3000" => 166,
    "4000" => 206,
    "6000" => 246,
    "8000" => 286
	   );

# Height -- in clb
my %clbheight = (
    "40" => 8,
    "80" => 16,
    "250" => 24,
    "500" => 32,
    "1000" => 40,
    "1500" => 48,
    "2000" => 56,
    "3000" => 64,
    "4000" => 80,
    "6000" => 96,
    "8000" => 112
		 );

# Then the number of bram blocks

my %brams = (
    "40" => 2,
    "80" => 2,
    "250" => 4,
    "500" => 4,
    "1000" => 4,
    "1500" => 4,
    "2000" => 4,
    "3000" => 6,
    "4000" => 6,
    "6000" => 6,
    "8000" => 6
);

# my %idcode = (
#     "40" => 0x10008093,
#     "80" => ,
#     "250" => ,
#     "500" => ,
#     "1000" => ,
#     "1500" => ,
#     "2000" => ,
#     "3000" => ,
#     "4000" => ,
#     "6000" => ,
#     "8000" =>
# )

# From this we infer the geometry
my $chip;

for $chip (keys %clbwidth) {
    # Generate the chip_data file
    # The width of the chip is 2 IOI + IOB + BRAM COLS
    # The height of the chip is 2 TIOI + IOB

    my $dirname = "xc2v${chip}";
    my $filename = "${dirname}/chip_control";
    mkdir $dirname;
    open(CTRL, ">$filename") or die "opening $filename";
    print CTRL "[DIMENTIONS]\n";
    my $width = $clbwidth{$chip} + $brams{$chip} + 4;
    print CTRL "WIDTH=$width\n";
    my $height = $clbheight{$chip} + 4;
    print CTRL "HEIGHT=$height\n";
    close(CTRL);

    # Generate the chip_descr file
    $filename = "${dirname}/chip_data";
    open(NEW, ">$filename") or die "Opening $filename.new";

    # The CLBs are interleaved with other things
    my $type;
    for $type (keys %C_enum) {

	if ($type =~ /^NR_SITE_TYPE$/) {
	    next;
	}
	if ($type =~ /^SITE_TYPE_NEUTRAL$/) {
	    next;
	}

	# No switch...
	# CLBs for starters
	if ($type =~ /^CLB$/) {
	    #function print_clb_height
	    #hard, according to type...
	    print_clb_width($brams{$chip}, $clbwidth{$chip}, $type);
	    print_clb_height($clbheight{$chip}, $type);
	} elsif ($type =~ /^BRAM$/) {
	    print_bram_width($brams{$chip}, $clbwidth{$chip}, $type);
	    print_clb_height($clbheight{$chip}, $type);
	} elsif ($type =~ /^TIOI$/) {
	    # They follow the CLB on the width
	    print_clb_width($brams{$chip}, $clbwidth{$chip}, $type);
	    print_top_height($type);
	} elsif ($type =~ /^BIOI$/) {
	    print_clb_width($brams{$chip}, $clbwidth{$chip}, $type);
	    print_bottom_height($clbheight{$chip},$type);
	} elsif ($type =~ /^TIOIBRAM$/) {
	    print_bram_width($brams{$chip}, $clbwidth{$chip}, $type);
	    print_top_height($type);
	} elsif ($type =~ /^BIOIBRAM$/) {
	    print_bram_width($brams{$chip}, $clbwidth{$chip}, $type);
	    print_bottom_height($clbheight{$chip},$type);
	} elsif ($type =~ /^TTERMBRAM$/) {
	    print_bram_width($brams{$chip}, $clbwidth{$chip}, $type);
	    print_termtop_height($type);
	} elsif ($type =~ /^BTERMBRAM$/) {
	    print_bram_width($brams{$chip}, $clbwidth{$chip}, $type);
	    print_termbottom_height($clbheight{$chip},$type);
	} elsif ($type =~ /^TTERM$/) {
	    print_clb_width($brams{$chip}, $clbwidth{$chip}, $type);
	    print_termtop_height($type);
	} elsif ($type =~ /^BTERM$/) {
	    print_clb_width($brams{$chip}, $clbwidth{$chip}, $type);
	    print_termbottom_height($clbheight{$chip},$type);
	} elsif ($type =~ /^LIOI$/) {
	    print_left_width($type);
	    print_clb_height($clbheight{$chip}, $type);
	} elsif ($type =~ /^RIOI$/) {
	    print_right_width($clbwidth{$chip},$brams{$chip},$type);
	    print_clb_height( $clbheight{$chip}, $type);
	} elsif ($type =~ /^LTERM$/) {
	    print_termleft_width($type);
	    print_clb_height($clbheight{$chip}, $type);
	} elsif ($type =~ /^RTERM$/) {
	    print_termright_width($clbwidth{$chip}, $brams{$chip}, $type);
	    print_clb_height( $clbheight{$chip}, $type);
	} elsif ($type =~ /^LIOIBRAM$/) {
	    print_left_width($type);
	    print_bram_height($brams{$chip}, $clbheight{$chip}, $type);
	} elsif ($type =~ /^RIOIBRAM$/) {
	    print_right_width($clbwidth{$chip},$brams{$chip},$type);
	    print_bram_height($brams{$chip}, $clbheight{$chip}, $type);
	} elsif ($type =~ /^LTERMBRAM$/) {
	    print_termleft_width($type);
	    print_bram_height($brams{$chip}, $clbheight{$chip}, $type);
	} elsif ($type =~ /^RTERMBRAM$/) {
	    print_termright_width($clbwidth{$chip},$brams{$chip},$type);
	    print_bram_height($brams{$chip}, $clbheight{$chip}, $type);
	} #corners
	elsif ($type =~ /^BR$/) {
	    #ICAP is here. Bottom is top
	    print_right_width($clbwidth{$chip},$brams{$chip},$type);
	    print_bottom_height($clbheight{$chip},$type);
	} elsif ($type =~ /^BL$/) {
	    print_left_width($type);
	    print_bottom_height($clbheight{$chip},$type);
	} elsif ($type =~ /^TR$/) {
	    #BSCAN is here
	    print_right_width($clbwidth{$chip},$brams{$chip},$type);
	    print_top_height($type);
	} elsif ($type =~ /^TL$/) {
	    print_left_width($type);
	    print_top_height($type);
	} elsif ($type =~ /^(R|L)(T|B)TERM$/) {
	    my $lr = $1;
	    my $tb = $2;
	    if ($lr =~ /R/) {
		print_termright_width($clbwidth{$chip},$brams{$chip},$type);
	    } else {
		print_termleft_width($type);
	    }
	    if ($tb =~ /T/) {
		print_top_height($type);
	    } else {
		print_bottom_height($clbheight{$chip},$type);
	    }
	} elsif ($type =~ /^(T|B)(L|R)TERM$/) {
	    my $lr = $2;
	    my $tb = $1;
	    if ($lr =~ /R/) {
		print_right_width($clbwidth{$chip},$brams{$chip},$type);
	    } else {
		print_left_width($type);
	    }
	    if ($tb =~ /T/) {
		print_termtop_height($type);
	    } else {
		print_termbottom_height($clbheight{$chip}, $type);
	    }
	}
	#corner terms

	# Miss right, left,
	# brams need their own function
	# print_bram_width
	# print_bram_height

	# Corner cases, haha
	print_database_std(\*NEW,\%C_enum);
	reset_database();
    }
    close(NEW);
}

sub print_clb_height
{
    my ($ncols, $type) = @_;
    my $last_y = $y_offset + $ncols;
    register_interval_vert($type,$y_offset,$ncols);
}

sub print_clb_width
{
    my ($nbrams, $ncols, $type) = @_;
    if ($nbrams == 2) {
	my $inter = $ncols / 4;
	register_interval_horiz($type, $x_offset,$inter);
	register_interval_horiz($type, $x_offset+$inter+1,2*$inter);
	register_interval_horiz($type, $x_offset+3*$inter+2,$inter);
    } elsif ($nbrams == 4) {
	my $inter = ($ncols - 8) / 2;
	register_interval_horiz($type, $x_offset,2);
	register_interval_horiz($type, $x_offset+3,$inter);
	register_interval_horiz($type, $x_offset+$inter+4,4);
	register_interval_horiz($type, $x_offset+$inter+9,$inter);
	register_interval_horiz($type, $x_offset+2*$inter+10,2);
    } else {
	#nbrams on colomns
	my $inter = ($ncols - 8) / 4;
	register_interval_horiz($type, $x_offset,2);
	register_interval_horiz($type, $x_offset+3,$inter);
	register_interval_horiz($type, $x_offset+$inter+4,$inter);
	register_interval_horiz($type, $x_offset+2*$inter+5,4);
	register_interval_horiz($type, $x_offset+2*$inter+10,$inter);
	register_interval_horiz($type, $x_offset+3*$inter+11,$inter);
	register_interval_horiz($type, $x_offset+4*$inter+12,2);
    }
}

sub print_bram_width
{
    my ($nbrams, $ncols, $type) = @_;
    if ($nbrams == 2) {
	my $inter = $ncols / 4;
	register_interval_horiz($type, $x_offset+$inter,1);
	register_interval_horiz($type, $x_offset+3*$inter+1,1);
    } elsif ($nbrams == 4) {
	my $inter = ($ncols - 8) / 2;
	register_interval_horiz($type, $x_offset+2,1);
	register_interval_horiz($type, $x_offset+$inter+3,1);
	register_interval_horiz($type, $x_offset+$inter+8,1);
	register_interval_horiz($type, $x_offset+2*$inter+9,1);
    } else {
	#nbrams on 6 columns
	my $inter = ($ncols - 8) / 4;
	register_interval_horiz($type, $x_offset+2,1);
	register_interval_horiz($type, $x_offset+$inter+3,1);
	register_interval_horiz($type, $x_offset+2*$inter+4,1);
	register_interval_horiz($type, $x_offset+2*$inter+9,1);
	register_interval_horiz($type, $x_offset+3*$inter+10,1);
	register_interval_horiz($type, $x_offset+4*$inter+11,1);
    }
}

# This prints a 1-width thing
sub print_isolated
{
    my ($type, $start, $dir) = @_;
    my $end = $start + 1;
    if ($dir =~ "y") {
	register_interval_vert($type,$start,$end-$start);
    } else {
	register_interval_horiz($type,$start,$end-$start);
    }
}

sub print_top_height
{
    my ( $type) = @_;
    print_isolated($type, $y_offset - 1, "y");
}

sub print_bottom_height
{
    my ($ncols, $type) = @_;
    my $last_y = $y_offset + $ncols;
    print_isolated($type, $last_y, "y");
}

sub print_left_width
{
    my ( $type) = @_;
    print_isolated($type, $x_offset - 1, "x");
}

sub print_right_width
{
    my ($ncols, $brams, $type) = @_;
    my $last_x = $x_offset + $brams + $ncols;
    print_isolated($type, $last_x, "x");
}

sub print_termtop_height
{
    my ( $type) = @_;
    print_isolated($type, "0", "y");
}

sub print_termbottom_height
{
    my ($ncols, $type) = @_;
    my $last_y = $y_offset + $ncols + 1;
    print_isolated($type, $last_y, "y");
}

sub print_termleft_width
{
    my ( $type) = @_;
    print_isolated($type, "0", "x");
}

sub print_termright_width
{
    my ($ncols, $brams, $type) = @_;
    my $last_x = $x_offset + $ncols + $brams + 1;
    print_isolated($type, $last_x, "x");
}
