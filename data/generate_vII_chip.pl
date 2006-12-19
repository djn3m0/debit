#! /usr/bin/perl -w

use strict;

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
    if ($nbrams == 2) {
	my $inter = $ncols / 4;
	print_interval($output,$x_offset,$x_offset+$inter);
	print_sep( $output );
	print_interval($output,$x_offset+$inter+1,$x_offset+3*$inter+1);
	print_sep( $output );
	print_interval($output,$x_offset+3*$inter+2,$x_offset+4*$inter+2);
    } elsif ($nbrams == 4) {
	my $inter = ($ncols - 8) / 2;
	#first small col
	print_interval($output,$x_offset,$x_offset+2);
	print_sep( $output );
	#first big
	print_interval($output,$x_offset+3,$x_offset+$inter+3);
	print_sep( $output );
	#the middle
	print_interval($output,$x_offset+$inter+4,$x_offset+$inter+8);
	print_sep( $output );
	#the second big
	print_interval($output,$x_offset+$inter+9,$x_offset+2*$inter+9);
	print_sep( $output );
	#the last small
	print_interval($output,$x_offset+2*$inter+10,$x_offset+2*$inter+12);
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
    if ($nbrams == 2) {
	my $inter = $ncols / 4;
	print_interval($output,$x_offset+$inter,$x_offset+$inter+1);
	print_sep( $output );
	print_interval($output,$x_offset+3*$inter+1,$x_offset+3*$inter+2);
    } elsif ($nbrams == 4) {
	my $inter = ($ncols - 8) / 2;
	#first small col
	print_interval($output,$x_offset+2,$x_offset+3);
	print_sep( $output );
	#first big
	print_interval($output,$x_offset+$inter+3,$x_offset+$inter+4);
	print_sep( $output );
	#the middle
	print_interval($output,$x_offset+$inter+8,$x_offset+$inter+9);
	print_sep( $output );
	#the second big
	print_interval($output,$x_offset+2*$inter+9,$x_offset+2*$inter+10);
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
