#! /usr/bin/perl -w

use strict;

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
    my $filename = "${dirname}/chip_control";
    mkdir $dirname;
    open(CTRL, ">$filename") or die "opening $filename";
    print CTRL "[DIMENTIONS]\n";
    my $width = $clbwidth{$chip} + $brams{$chip};
    print CTRL "WIDTH=$width\n";
    my $height = 2 * 16 * $clbheight{$chip};
    print CTRL "HEIGHT=$height\n";
    close(CTRL);

    # Generate the chip_descr file
    $filename = "${dirname}/chip_data";
    open(DESCR, ">$filename") || die "opening $filename";

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

	print DESCR "[$type]\n";
	#No switch...
	# CLBs for starters
	if ($type =~ /^CLB$/) {
	    #function print_clb_height
	    #hard, according to type...
	    #print_clb_width();
	    print_clb_width($brams{$chip}, $clbwidth{$chip} - 1, \*DESCR);
	    print_clb_height($clbheight{$chip}, \*DESCR);
	} elsif ($type =~ /^IOB$/) {
	    print_iob_width($brams{$chip}, $clbwidth{$chip} - 1, \*DESCR);
	    print_clb_height($clbheight{$chip}, \*DESCR);
	} elsif ($type =~ /^DSP48$/) {
	    # They follow the CLB on the width
	    print_dsp_width($brams{$chip}, $clbwidth{$chip} - 1, \*DESCR);
	    print_clb_height($clbheight{$chip},\*DESCR);
	} elsif ($type =~ /^BRAM$/) {
	    print_bram_width($brams{$chip}, $clbwidth{$chip} - 1, \*DESCR);
	    print_clb_height($clbheight{$chip},\*DESCR);
	}
	print DESCR "type=$C_enum{$type}\n";
    }
    close(DESCR);
}

sub print_clb_height
{
    my ($ncols, $output) = @_;
    my $last_y = $y_offset + 2*16*$ncols;
    print $output "y=$y_offset;$last_y\n";
}

sub print_dsp_width
{
    my ($nbrams, $ncols, $output) = @_;
    print $output "x=";
    my $inter = $ncols;
    #define DSP_V4_OF_END(x) ((x) > 50 ? 13 : 9)
    my $dsp_offset = $ncols > 50 ? (1+12+2) : (1+8+1);
    print_interval($output,$dsp_offset,$dsp_offset+1);
    print $output "\n";
}

sub print_iob_width
{
    my ($nbrams, $ncols, $output) = @_;
    print $output "x=";
    my $inter = $ncols + $nbrams;
    #XXX Not sure about this !
    my $iob_offset = ($inter) >> 1;
    my $endcol = $inter - 1;
    print_interval($output,0,1);
    print_sep($output);
    print_interval($output,$iob_offset,$iob_offset+1);
    print_sep($output);
    print_interval($output,$endcol,$endcol+1);
    print $output "\n";
}

sub print_clb_width
{
    my ($nbrams, $ncols, $output) = @_;
    print $output "x=";
    my $inter = $ncols;

    #BRAM is present after 4 clbs, then before 4 clbs

    #We lack bram for now, will be done later
    my $dsp_offset = $ncols > 50 ? 13+2 : 9+1;
    my $iob_offset = ($ncols+$nbrams) >> 1;
    my $endcol = $ncols+$nbrams-1;
    my $running_start;

    #up to first bram
    print_interval($output,1,5);
    print_sep($output);
    $running_start = 6;

    # Second bram row
    if ($nbrams >= 5) {
	print_interval($output, 6, 10);
	print_sep($output);
	$running_start = 11;
    }

    # Up to the first DSP
    print_interval($output,$running_start,$dsp_offset);
    print_sep($output);
    $running_start = $dsp_offset+1;

    #possibly interrupted by brams again, after the DSP
    if ($nbrams >= 6) {
	print_interval($output, $running_start, $running_start+4);
	$running_start = $running_start + 5;
    }

    # Then up to the first IOB / GCLK
    print_interval($output,$running_start,$iob_offset);
    print_sep($output);
    $running_start = $iob_offset+1;

    # More complex
    if ($nbrams >= 7) {
	print_interval($output, $running_start, $endcol - 20);
	print_sep($output);

	print_interval($output, $endcol - 19, $endcol - 15);
	print_sep($output);

    } elsif ($nbrams >= 5) {
	print_interval($output,$running_start,$endcol - 15);
	print_sep($output);
	$running_start = $endcol - 15 + 1;
    }

    # The last two bram colomn are always present
    print_interval($output, $running_start, $endcol - 10);
    print_sep($output);

    print_interval($output, $endcol - 9, $endcol - 5);
    print_sep($output);

    # Then up to the END
    print_interval($output,$endcol - 4,$endcol);

    print $output "\n";
}

# 8 + 4 + 12 = 24

sub print_bram_width
{
    my ($nbrams, $ncols, $output) = @_;
    my $endcol = $ncols + $nbrams;
    print $output "x=";

    #First bram row
    print_interval($output,5,6);
    print_sep($output);

    # Second bram row
    if ($nbrams >= 5) {
	print_interval($output, 10, 11);
	print_sep($output);
    }

    if ($nbrams >= 6) {
	print_interval($output, 20, 21);
	print_sep($output);
    }

    # Middle
    if ($nbrams >= 7) {
	print_interval($output, $endcol-21, $endcol-20);
	print_sep($output);
    }

    if ($nbrams >= 5) {
	print_interval($output, $endcol-16, $endcol-15);
	print_sep($output);
    }

    #Last two bram rows
    print_interval($output,$endcol-11,$endcol-10);
    print_sep($output);

    print_interval($output,$endcol-6,$endcol-5);

#    print_interval($output,$ncols,$ncols+$nbrams);
    print $output "\n";    
}

sub print_interval
{
    my ($out, $start, $end) = @_;
    print $out $start;
    print $out ";";
    print $out $end;
}

sub print_sep
{
    my ($out) = @_;
    print $out ";";
}
