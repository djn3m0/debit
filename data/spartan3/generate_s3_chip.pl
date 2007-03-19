#! /usr/bin/perl -w

use strict;
use integer;

# This script generates chip_control and chip_data databases for the
# spartan3 family

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
	} elsif ($type =~ /^BRAM$/) {
	    print_bram_width($brams{$chip}, $clbwidth{$chip}, \*DESCR);
	    print_clb_height($clbheight{$chip}, \*DESCR);
	} elsif ($type =~ /^TIOI$/) {
	    # They follow the CLB on the width
	    print_clb_width($brams{$chip}, $clbwidth{$chip}, \*DESCR);
	    print_top_height(\*DESCR);
	} elsif ($type =~ /^BIOI$/) {
	    print_clb_width($brams{$chip}, $clbwidth{$chip}, \*DESCR);
	    print_bottom_height($clbheight{$chip},\*DESCR);
	} elsif ($type =~ /^TIOIBRAM$/) {
	    print_bram_width($brams{$chip}, $clbwidth{$chip}, \*DESCR);
	    print_top_height(\*DESCR);
	} elsif ($type =~ /^BIOIBRAM$/) {
	    print_bram_width($brams{$chip}, $clbwidth{$chip}, \*DESCR);
	    print_bottom_height($clbheight{$chip},\*DESCR);
	} elsif ($type =~ /^TTERMBRAM$/) {
	    print_bram_width($brams{$chip}, $clbwidth{$chip}, \*DESCR);
	    print_termtop_height(\*DESCR);
	} elsif ($type =~ /^BTERMBRAM$/) {
	    print_bram_width($brams{$chip}, $clbwidth{$chip}, \*DESCR);
	    print_termbottom_height($clbheight{$chip},\*DESCR);
	} elsif ($type =~ /^TTERM$/) {
	    print_clb_width($brams{$chip}, $clbwidth{$chip}, \*DESCR);
	    print_termtop_height(\*DESCR);
	} elsif ($type =~ /^BTERM$/) {
	    print_clb_width($brams{$chip}, $clbwidth{$chip}, \*DESCR);
	    print_termbottom_height($clbheight{$chip},\*DESCR);
	} elsif ($type =~ /^LIOI$/) {
	    print_left_width(\*DESCR);
	    print_clb_height($clbheight{$chip}, \*DESCR);
	} elsif ($type =~ /^RIOI$/) {
	    print_right_width($clbwidth{$chip},$brams{$chip},\*DESCR);
	    print_clb_height( $clbheight{$chip}, \*DESCR);
	} elsif ($type =~ /^LTERM$/) {
	    print_termleft_width(\*DESCR);
	    print_clb_height($clbheight{$chip}, \*DESCR);
	} elsif ($type =~ /^RTERM$/) {
	    print_termright_width($clbwidth{$chip}, $brams{$chip}, \*DESCR);
	    print_clb_height( $clbheight{$chip}, \*DESCR);
	} elsif ($type =~ /^LIOIBRAM$/) {
	    print_left_width(\*DESCR);
	    print_bram_height($brams{$chip}, $clbheight{$chip}, \*DESCR);
	} elsif ($type =~ /^RIOIBRAM$/) {
	    print_right_width($clbwidth{$chip},$brams{$chip},\*DESCR);
	    print_bram_height($brams{$chip}, $clbheight{$chip}, \*DESCR);
	} elsif ($type =~ /^LTERMBRAM$/) {
	    print_termleft_width(\*DESCR);
	    print_bram_height($brams{$chip}, $clbheight{$chip}, \*DESCR);
	} elsif ($type =~ /^RTERMBRAM$/) {
	    print_termright_width($clbwidth{$chip},$brams{$chip},\*DESCR);
	    print_bram_height($brams{$chip}, $clbheight{$chip}, \*DESCR);
	}

	# Miss right, left,
	# brams need their own function
	# print_bram_width
	# print_bram_height

	# Corner cases, haha
	print DESCR "type=$C_enum{$type}\n";
	print DESCR "\n";
    }

    close(DESCR);
}

sub print_clb_height
{
    my ($ncols, $output) = @_;
    my $last_y = $y_offset + $ncols;
    print $output "y=$y_offset;$last_y\n";
}

sub print_clb_width
{
    my ($nbrams, $ncols, $output) = @_;
    print $output "x=";
    if ($nbrams == 1) {
	my $inter = 2;
	print_interval($output,$x_offset,$x_offset+$inter);
	print_sep( $output );
	print_interval($output,$x_offset+$inter+1,$x_offset+$ncols+1);
    } elsif ($nbrams == 2) {
	my $inter = $ncols - 4;
	print_interval($output,$x_offset,$x_offset+2);
	print_sep( $output );
	print_interval($output,$x_offset+3,$x_offset+3+$inter);
	print_sep( $output );
	print_interval($output,$x_offset+4+$inter,$x_offset+$inter+6);
    } elsif ($nbrams == 4) {
	my $inter = ((($ncols - 4) / 2) / 3) * 2;
	my $middle = $ncols - 4 - 2 * $inter;
	#first small col
	print_interval($output,$x_offset,$x_offset+2);
	print_sep( $output );
	#first big
	print_interval($output,$x_offset+3,$x_offset+$inter+3);
	print_sep( $output );
	#the middle
	print_interval($output,$x_offset+$inter+4,$x_offset+$middle+$inter+4);
	print_sep( $output );
	#the second big
	print_interval($output,$x_offset+$middle+$inter+5,$x_offset+$middle+2*$inter+5);
	print_sep( $output );
	#the last small
	print_interval($output,$x_offset+$middle+2*$inter+6,$x_offset+$middle+2*$inter+8);
    } else {
	#nbrams on colomns
	my $inter = ($ncols - 8) / 4;
	#first small col
	print_interval($output,$x_offset,$x_offset+2);
	print_sep( $output );
	#first big
	print_interval($output,$x_offset+3,$x_offset+$inter+3);
	print_sep( $output );
	print_interval($output,$x_offset+$inter+4,$x_offset+2*$inter+4);
	print_sep( $output );
	#the middle
	print_interval($output,$x_offset+2*$inter+5,$x_offset+2*$inter+9);
	print_sep( $output );
	#the second big
	print_interval($output,$x_offset+2*$inter+10,$x_offset+3*$inter+10);
	print_sep( $output );
	print_interval($output,$x_offset+3*$inter+11,$x_offset+4*$inter+11);
	print_sep( $output );
	#the last small
	print_interval($output,$x_offset+4*$inter+12,$x_offset+4*$inter+14);
    }

    print $output "\n";
}

sub print_bram_width
{
    my ($nbrams, $ncols, $output) = @_;
    print $output "x=";
    if ($nbrams == 1) {
	my $inter = 2;
	print_interval($output,$x_offset+$inter,$x_offset+$inter+1);
    } elsif ($nbrams == 2) {
	my $inter = $ncols - 4;
	print_interval($output,$x_offset+2,$x_offset+3);
	print_sep( $output );
	print_interval($output,$x_offset+3+$inter,$x_offset+4+$inter);
    } elsif ($nbrams == 4) {
	my $inter = ((($ncols - 4) / 2) / 3) * 2;
	my $middle = $ncols - 4 - 2 * $inter;
	#first small col
	print_interval($output,$x_offset+2,$x_offset+3);
	print_sep( $output );
	#first big
	print_interval($output,$x_offset+$inter+3,$x_offset+$inter+4);
	print_sep( $output );
	#the middle
	print_interval($output,$x_offset+$inter+$middle+4,$x_offset+$middle+$inter+5);
	print_sep( $output );
	#the second big
	print_interval($output,$x_offset+2*$inter+$middle+5,$x_offset+2*$inter+$middle+6);
    } else {
	#nbrams on 6 columns
	my $inter = ($ncols - 8) / 4;
	#first small col
	print_interval($output,$x_offset+2,$x_offset+3);
	print_sep( $output );
	#first big
	print_interval($output,$x_offset+$inter+3,$x_offset+$inter+4);
	print_sep( $output );
	print_interval($output,$x_offset+2*$inter+4,$x_offset+2*$inter+5);
	print_sep( $output );
	#the middle
	print_interval($output,$x_offset+2*$inter+9,$x_offset+2*$inter+10);
	print_sep( $output );
	#the second big
	print_interval($output,$x_offset+3*$inter+10,$x_offset+3*$inter+11);
	print_sep( $output );
	print_interval($output,$x_offset+4*$inter+11,$x_offset+4*$inter+12);
    }

    print $output "\n";
}

# This prints a 1-width thing
sub print_isolated
{
    my ($output, $start, $dir) = @_;
    my $end = $start + 1;
    print $output "$dir=$start;$end\n";
}

sub print_top_height
{
    my ($output) = @_;
    print_isolated($output, $y_offset - 1, "y");
}

sub print_bottom_height
{
    my ($ncols, $output) = @_;
    my $last_y = $y_offset + $ncols;
    print_isolated($output, $last_y, "y");
}

sub print_left_width
{
    my ($output) = @_;
    print_isolated($output, $x_offset - 1, "x");
}

sub print_right_width
{
    my ($ncols, $brams, $output) = @_;
    my $last_x = $x_offset + $brams + $ncols;
    print_isolated($output, $last_x, "x");
}

sub print_termtop_height
{
    my ($output) = @_;
    print_isolated($output, "0", "y");
}

sub print_termbottom_height
{
    my ($ncols, $output) = @_;
    my $last_y = $y_offset + $ncols + 1;
    print_isolated($output, $last_y, "y");
}

sub print_termleft_width
{
    my ($output) = @_;
    print_isolated($output, "0", "x");
}

sub print_termright_width
{
    my ($ncols, $brams, $output) = @_;
    my $last_x = $x_offset + $ncols + $brams + 1;
    print_isolated($output, $last_x, "x");
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
