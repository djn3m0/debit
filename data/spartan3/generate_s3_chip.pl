#! /usr/bin/perl -w

use strict;
use integer;

require "../intervals.pl";
require "../v2s3.pl";

my $x_offset = 2;

# This script generates chip_control and chip_data databases for the
# spartan3 family

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
		"50" => 12,
		"200" => 20,
		"400" => 28,
		"1000" => 40,
		"1500" => 52,
		"2000" => 64,
		"4000" => 72,
		"5000" => 80);

# Height -- in clb
my %clbheight = (
		"50" => 16,
		"200" => 24,
		"400" => 32,
		"1000" => 48,
		"1500" => 64,
		"2000" => 80,
		"4000" => 96,
		"5000" => 104);

# Then the number of bram blocks

my %brams = (
	     "50" => 1,
	     "200" => 2,
	     "400" => 2,
	     "1000" => 2,
	     "1500" => 2,
	     "2000" => 2,
	     "4000" => 4,
	     "5000" => 4);

# From this we infer the geometry
my $chip;

for $chip (keys %clbwidth) {
    # Generate the chip_data file
    # The width of the chip is 2 IOI + IOB + BRAM COLS
    # The height of the chip is 2 TIOI + IOB

    my $dirname = "xc3s${chip}";
    mkdir $dirname;
    open(CTRL, ">${dirname}/chip_control") or die "opening control";
    open(DESCR, ">${dirname}/chip_data") || die "opening data";

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
	    print_clb_width($brams{$chip}, $clbwidth{$chip},$type);
	    print_clb_height($clbheight{$chip},$type);
	} elsif ($type =~ /^BRAM$/) {
	    print_bram_width($brams{$chip}, $clbwidth{$chip},$type);
	    print_clb_height($clbheight{$chip},$type);
	} elsif ($type =~ /^TIOI$/) {
	    # They follow the CLB on the width
	    print_clb_width($brams{$chip}, $clbwidth{$chip},$type);
	    print_top_height($type);
	} elsif ($type =~ /^BIOI$/) {
	    print_clb_width($brams{$chip}, $clbwidth{$chip},$type);
	    print_bottom_height($clbheight{$chip},$type);
	} elsif ($type =~ /^TIOIBRAM$/) {
	    print_bram_width($brams{$chip}, $clbwidth{$chip},$type);
	    print_top_height($type);
	} elsif ($type =~ /^BIOIBRAM$/) {
	    print_bram_width($brams{$chip}, $clbwidth{$chip},$type);
	    print_bottom_height($clbheight{$chip},$type);
	} elsif ($type =~ /^TTERMBRAM$/) {
	    print_bram_width($brams{$chip}, $clbwidth{$chip},$type);
	    print_termtop_height($type);
	} elsif ($type =~ /^BTERMBRAM$/) {
	    print_bram_width($brams{$chip}, $clbwidth{$chip},$type);
	    print_termbottom_height($clbheight{$chip},$type);
	} elsif ($type =~ /^TTERM$/) {
	    print_clb_width($brams{$chip}, $clbwidth{$chip},$type);
	    print_termtop_height($type);
	} elsif ($type =~ /^BTERM$/) {
	    print_clb_width($brams{$chip}, $clbwidth{$chip},$type);
	    print_termbottom_height($clbheight{$chip},$type);
	} elsif ($type =~ /^LIOI$/) {
	    print_left_width($type);
	    print_clb_height($clbheight{$chip},$type);
	} elsif ($type =~ /^RIOI$/) {
	    print_right_width($clbwidth{$chip},$brams{$chip},$type);
	    print_clb_height( $clbheight{$chip},$type);
	} elsif ($type =~ /^LTERM$/) {
	    print_termleft_width($type);
	    print_clb_height($clbheight{$chip},$type);
	} elsif ($type =~ /^RTERM$/) {
	    print_termright_width($clbwidth{$chip}, $brams{$chip},$type);
	    print_clb_height( $clbheight{$chip},$type);
	} elsif ($type =~ /^LIOIBRAM$/) {
	    print_left_width($type);
	    print_bram_height($brams{$chip}, $clbheight{$chip},$type);
	} elsif ($type =~ /^RIOIBRAM$/) {
	    print_right_width($clbwidth{$chip},$brams{$chip},$type);
	    print_bram_height($brams{$chip}, $clbheight{$chip},$type);
	} elsif ($type =~ /^LTERMBRAM$/) {
	    print_termleft_width($type);
	    print_bram_height($brams{$chip}, $clbheight{$chip},$type);
	} elsif ($type =~ /^RTERMBRAM$/) {
	    print_termright_width($clbwidth{$chip},$brams{$chip},$type);
	    print_bram_height($brams{$chip}, $clbheight{$chip},$type);
	} else {
	    # Corner cases, haha
	    print DESCR "[$type]\n";
	    print DESCR "type=$C_enum{$type}\n";
	    print DESCR "\n";
	}
    }
    print_database_std(\*DESCR,\%C_enum);
    print_database_ctrl(\*CTRL);
    reset_database();
    close(DESCR);
    close(CTRL);
}

sub print_clb_width
{
    my ($nbrams, $ncols, $type) = @_;
    if ($nbrams == 1) {
	my $inter = 2;
	register_interval_horiz($type, $x_offset,$inter);
	register_interval_horiz($type, $x_offset+$inter+1, ($ncols - $inter));
    } elsif ($nbrams == 2) {
	my $inter = $ncols - 4;
	register_interval_horiz($type, $x_offset, 2);
	register_interval_horiz($type, $x_offset+3, $inter);
	register_interval_horiz($type, $x_offset+$inter+4, 2);
    } elsif ($nbrams == 4) {
	my $inter = ((($ncols - 4) / 2) / 3) * 2;
	my $middle = $ncols - 4 - 2 * $inter;
	register_interval_horiz($type,$x_offset, 2);
	register_interval_horiz($type,$x_offset+3, $inter);
	register_interval_horiz($type,$x_offset+$inter+4, $middle);
	register_interval_horiz($type,$x_offset+$inter+$middle+5,$inter);
	register_interval_horiz($type,$x_offset+2*$inter+$middle+6,2);
    } else {
	#nbrams on colomns
	my $inter = ($ncols - 8) / 4;
	#first small col
	register_interval_horiz($type,$x_offset, 2);
	register_interval_horiz($type,$x_offset+3, $inter);
	register_interval_horiz($type,$x_offset+$inter+4, $inter);
	register_interval_horiz($type,$x_offset+2*$inter+5,4);
	register_interval_horiz($type,$x_offset+2*$inter+10,$inter);
	register_interval_horiz($type,$x_offset+3*$inter+11,$inter);
	register_interval_horiz($type,$x_offset+4*$inter+12,2);
    }
}

sub print_bram_width
{
    my ($nbrams, $ncols, $type) = @_;
    if ($nbrams == 1) {
	my $inter = 2;
	register_interval_horiz($type,$x_offset+$inter,1);
    } elsif ($nbrams == 2) {
	my $inter = $ncols - 4;
	register_interval_horiz($type,$x_offset+2,1);
	register_interval_horiz($type,$x_offset+$inter+3,1);
    } elsif ($nbrams == 4) {
	my $inter = ((($ncols - 4) / 2) / 3) * 2;
	my $middle = $ncols - 4 - 2 * $inter;
	register_interval_horiz($type,$x_offset+2,1);
	register_interval_horiz($type,$x_offset+$inter+3,1);
	register_interval_horiz($type,$x_offset+$inter+$middle+4,1);
	register_interval_horiz($type,$x_offset+2*$inter+$middle+5,1);
    } else {
	#nbrams on 6 columns
	my $inter = ($ncols - 8) / 4;
	register_interval_horiz($type,$x_offset+2,1);
	register_interval_horiz($type,$x_offset+$inter+3,1);
	register_interval_horiz($type,$x_offset+2*$inter+4,1);
	register_interval_horiz($type,$x_offset+2*$inter+9,1);
	register_interval_horiz($type,$x_offset+3*$inter+10,1);
	register_interval_horiz($type,$x_offset+4*$inter+11,1);
    }
}
