#! /usr/bin/perl -w

use strict;
require "../intervals.pl";
require "../v2s3.pl";

# This script generates chip_control and chip_data databases for the
# virtex-II family

my $x_offset = 2;

# It could be worse, but not much more... DSPs-enabled V4 and V2P will be fun !
# Type => num
my $i = 0;

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

# From this we infer the geometry
for my $chip (keys %clbwidth) {
    # Generate the chip_data file
    # The width of the chip is 2 IOI + IOB + BRAM COLS
    # The height of the chip is 2 TIOI + IOB

    my $dirname = "xc2v${chip}";
    mkdir $dirname;
    open(CTRL, ">${dirname}/chip_control") or die "opening control";
    open(NEW, ">${dirname}/chip_data") or die "Opening data";

    # The CLBs are interleaved with other things
    for my $type (keys %C_enum) {

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
	} else {
	    # Corner cases, haha
	    print NEW "[$type]\n";
	    print NEW "type=$C_enum{$type}\n";
	    print NEW "\n";
	}
    }
    print_database_std(\*NEW,\%C_enum);
    print_database_ctrl(\*CTRL);
    reset_database();
    close(NEW);
    close(CTRL);
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
