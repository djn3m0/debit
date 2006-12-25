#! /usr/bin/perl -w

use strict;

# This script generates chip_control and chip_data databases for the
# virtex-II family

# It could be worse, but not much more... DSPs-enabled V4 and V2P will be fun !
# Type => num
my $i = 0;

my $y_offset = 0;
my $x_offset = 0;

my %C_enum = (
	      SITE_TYPE_NEUTRAL => $i++,
	      IOB => $i++,
	      CLB => $i++,
	      DSP48 => $i++,
	      GCLK => $i++,
	      BRAM => $i++,
#	      PAD => $i++,
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

    # The CLBs are interleaved with other things
    my $type;
    for $type (keys %C_enum) {

	if ($type =~ /^NR_SITE_TYPE$/) {
	    next;
	}
	if ($type =~ /^SITE_TYPE_NEUTRAL$/) {
	    next;
	}

	print DESCR "[$type]\n";
	# No switch...
	# CLBs for starters
	if ($type =~ /^CLB$/) {
	    #function print_clb_height
	    #hard, according to type...
	    #print_clb_width();
	    print_clb_width($brams{$chip}, $clbwidth{$chip}, \*DESCR);
	    print_clb_height($clbheight{$chip}, \*DESCR);
	} elsif ($type =~ /^IOB$/) {
	    print_iob_width($brams{$chip}, $clbwidth{$chip}, \*DESCR);
	    print_clb_height($clbheight{$chip}, \*DESCR);
	} elsif ($type =~ /^DSP48$/) {
	    # They follow the CLB on the width
	    print_dsp_width($brams{$chip}, $clbwidth{$chip}, \*DESCR);
	    print_clb_height($clbheight{$chip},\*DESCR);
	} elsif ($type =~ /^GCLK$/) {
	    print_gclk_width($brams{$chip}, $clbwidth{$chip}, \*DESCR);
	    print_clb_height($clbheight{$chip},\*DESCR);
	} elsif ($type =~ /^BRAM$/) {
	    print_bram_width($brams{$chip}, $clbwidth{$chip}, \*DESCR);
	    print_clb_height($clbheight{$chip},\*DESCR);
	}
    }

    close(DESCR);
}

sub print_clb_height
{
    my ($ncols, $output) = @_;
    my $last_y = $y_offset + 2*16*$ncols;
    print $output "y=$y_offset;$last_y\n";
}

#   switch (type) {
#   case V4_TYPE_CLB:
#     {
#       const int col = addr->col;
#       const unsigned end = bitdescr[chiptype].col_count[V4_TYPE_CLB] - 1;
#       const unsigned middle = end >> 1;
#       const unsigned dsp = DSP_V4_OF_END(end);
#       /* Let's be more intelligent.
# 	 Middle, extremities: IO.
# 	 DSP is at fixed position and the rest is CLB */
#       if (col == 0 || col == end || col == middle)
# 	return V4C_IOB;
#       if (col == (middle + 1))
# 	return V4C_GCLK;
#       if (col == dsp)
# 	return V4C_DSP48;
#       return V4C_CLB;
#     }

#   case V4C_IOB:
#     return 3;
#   case V4C_GCLK:
#     return 1;
#   case V4C_DSP48:
#     return 1;
#   case V4C_CLB:
#     return col_count[V4_TYPE_CLB] - 5;

sub print_dsp_width
{
    my ($nbrams, $ncols, $output) = @_;
    print $output "x=";
    my $inter = $ncols;
    #define DSP_V4_OF_END(x) ((x) > 50 ? 13 : 9)
    my $dsp_offset = $ncols > 50 ? 13 : 9;
    print_interval($output,$dsp_offset,$dsp_offset+1);
    print $output "\n";
}

sub print_gclk_width
{
    my ($nbrams, $ncols, $output) = @_;
    print $output "x=";
    my $inter = $ncols;
    my $gclk_offset = ($ncols >> 1) + 1;
    print_interval($output,$gclk_offset,$gclk_offset+1);
    print $output "\n";
}

sub print_iob_width
{
    my ($nbrams, $ncols, $output) = @_;
    print $output "x=";
    my $inter = $ncols;
    my $iob_offset = $ncols >> 1;
    my $endcol = $ncols - 1;
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

    #We lack bram for now, will be done later
    my $dsp_offset = $ncols > 50 ? 13 : 9;
    my $iob_offset = $ncols >> 1;

    # Up to the first DSP
    print_interval($output,1,$dsp_offset);
    print_sep($output);

    # Then up to the first IOB / GCLK
    print_interval($output,$dsp_offset+1,$iob_offset);
    print_sep($output);

    # Then up to the END
    print_interval($output,$iob_offset+2,$ncols-1);

    # But the brams !!!! ???? !!!!
    print $output "\n";
}

# 8 + 4 + 12 = 24

sub print_bram_width
{
    my ($nbrams, $ncols, $output) = @_;
    print $output "x=";
    print_interval($output,$ncols,$ncols+$nbrams);
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
