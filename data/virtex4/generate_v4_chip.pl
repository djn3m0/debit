#! /usr/bin/perl -w

use strict;
require "../intervals.pl";

# This script generates chip_control and chip_data databases for the
# virtex-4 family
# Yeah, I know. If you feel like rewriting this, please go ahead.

my $i = 0;

my $y_offset = 0;
my $x_offset = 0;

my %C_enum = (SITE_TYPE_NEUTRAL => $i++,
	      IOB => $i++,
	      CLB => $i++,
	      DSP48 => $i++,
	      GCLK => $i++,
	      BRAM => $i++,
	      PAD => $i++,
	      NR_SITE_TYPE => $i++
	      );

# First the raw data, width and height
# Width -- in clb
my %clbwidth = (
		"15" => 29,
		"25" => 33,
		"40" => 41,
		"60" => 57,
		"80" => 61,
		"100" => 69,
		"160" => 93,
		"200" => 121,
		);

# Height -- in clb
my %clbheight = (
		"15" => 2,
		"25" => 3,
		"40" => 4,
		"60" => 4,
		"80" => 5,
		"100" => 6,
		"160" => 6,
		"200" => 6,
		 );

# Then the number of bram blocks

my %brams = (
	     "15" => 3,
	     "25" => 3,
	     "40" => 3,
	     "60" => 5,
	     "80" => 5,
	     "100" => 5,
	     "160" => 6,
	     "200" => 7,
);

# From this we infer the geometry
my $chip;

for $chip (keys %clbwidth) {
    # Generate the chip_data file
    # The width of the chip is 2 IOI + IOB + BRAM COLS
    # The height of the chip is 2 TIOI + IOB

    my $dirname = "xc4vlx${chip}";
    mkdir $dirname;
    open(CTRL, ">${dirname}/chip_control") or die "opening control";
    open(DESCR, ">${dirname}/chip_data") || die "opening data";

    # The CLBs are interleaved with other things
    my $type;
    for $type (keys %C_enum) {
	if ($type =~ /^NR_SITE_TYPE$/) {
	    next;
	}
	if ($type =~ /^SITE_TYPE_NEUTRAL$/) {
	    next;
	}
	if ($type =~ /^PAD$/) {
	    next;
	}
	if ($type =~ /^GCLK$/) {
	    next;
	}

	if ($type =~ /^CLB$/) {
	    print_clb_width($brams{$chip}, $clbwidth{$chip} - 1, $type);
	    print_clb_height($clbheight{$chip}, $type);
	} elsif ($type =~ /^IOB$/) {
	    print_iob_width($brams{$chip}, $clbwidth{$chip} - 1, $type);
	    print_clb_height($clbheight{$chip}, $type);
	} elsif ($type =~ /^DSP48$/) {
	    # They follow the CLB on the width
	    print_dsp_width($brams{$chip}, $clbwidth{$chip} - 1, $type);
	    print_clb_height($clbheight{$chip},$type);
	} elsif ($type =~ /^BRAM$/) {
	    print_bram_width($brams{$chip}, $clbwidth{$chip} - 1, $type);
	    print_clb_height($clbheight{$chip},$type);
	}
    }

    print_database_std(\*DESCR,\%C_enum);
    print_database_ctrl(\*CTRL);
    reset_database();

    close(CTRL);
    close(DESCR);
}

sub print_clb_height
{
    my ($ncols, $type) = @_;
    register_interval_vert($type,$y_offset,2*16*$ncols);
}

sub print_dsp_width
{
    my ($nbrams, $ncols, $type) = @_;
    my $inter = $ncols;
    my $dsp_offset = $ncols > 50 ? (1+12+2) : (1+8+1);
    register_interval_horiz($type, $dsp_offset,1);
}

sub print_iob_width
{
    my ($nbrams, $ncols, $type) = @_;
    my $inter = $ncols + $nbrams;
    #XXX Not sure about this !
    my $iob_offset = ($inter) >> 1;
    my $endcol = $inter - 1;
    register_interval_horiz($type,0,1);
    register_interval_horiz($type,$iob_offset,1);
    register_interval_horiz($type,$endcol,1);
}

sub print_clb_width
{
    my ($nbrams, $ncols, $type) = @_;
    my $inter = $ncols;

    #BRAM is present after 4 clbs, then before 4 clbs

    #We lack bram for now, will be done later
    my $dsp_offset = $ncols > 50 ? 13+2 : 9+1;
    my $iob_offset = ($ncols+$nbrams) >> 1;
    my $endcol = $ncols+$nbrams-1;
    my $running_start;

    #up to first bram
    register_interval_horiz($type,1,4);
    $running_start = 6;

    # Second bram row
    if ($nbrams >= 5) {
	register_interval_horiz($type,6,4);
	$running_start = 11;
    }

    # Up to the first DSP
    register_interval_horiz($type,$running_start,$dsp_offset - $running_start);
    $running_start = $dsp_offset+1;

    #possibly interrupted by brams again, after the DSP
    if ($nbrams >= 6) {
	register_interval_horiz($type,$running_start,4);
	$running_start = $running_start + 5;
    }

    # Then up to the first IOB / GCLK
    register_interval_horiz($type,$running_start,$iob_offset - $running_start);
    $running_start = $iob_offset+1;

    # More complex
    if ($nbrams >= 7) {
	register_interval_horiz($type,$running_start,$endcol - 20 - $running_start);
	register_interval_horiz($type,$endcol - 19, 4);
	$running_start = $endcol - 15 + 1;
    } elsif ($nbrams >= 5) {
	register_interval_horiz($type,$running_start,$endcol - 15 - $running_start);
	$running_start = $endcol - 15 + 1;
    }

    # The last two bram colomn are always present
    register_interval_horiz($type,$running_start,$endcol - 10 - $running_start);
    register_interval_horiz($type,$endcol - 9,4);

    # Then up to the END
    register_interval_horiz($type,$endcol - 4,4);
}

# 8 + 4 + 12 = 24

sub print_bram_width
{
    my ($nbrams, $ncols, $type) = @_;
    my $endcol = $ncols + $nbrams;

    #First bram row
    register_interval_horiz($type,5,1);

    # Second bram row
    if ($nbrams >= 5) {
	register_interval_horiz($type,10,1);
    }

    if ($nbrams >= 6) {
	register_interval_horiz($type,20,1);
    }

    # Middle
    if ($nbrams >= 7) {
	register_interval_horiz($type,$endcol-21,1);
    }

    if ($nbrams >= 5) {
	register_interval_horiz($type,$endcol-16,1);
    }

    #Last two bram rows
    register_interval_horiz($type,$endcol-11,1);
    register_interval_horiz($type,$endcol-6,1);
}
