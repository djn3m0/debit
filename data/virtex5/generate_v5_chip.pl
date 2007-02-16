#! /usr/bin/perl -w

use strict;

# This script generates chip_control and chip_data databases for the
# virtex-5 family
# If you feel like rewriting this, please go ahead. Say things once
# says: it should also rewrite the bitstream_v5.h file.
my $i = 0;
my %C_enum = (SITE_TYPE_NEUTRAL => $i++,
	      IOB => $i++,
	      CLB => $i++,
	      DSP48 => $i++,
	      GCLK => $i++,
	      BRAM => $i++,
	      PAD => $i++,
	      NR_SITE_TYPE => $i++
	      );

my %width_refs;

$i = 0;
my %xc5vlx30 = (
		$i++ => "IOB",
		$i++ => "CLB",
		$i++ => "CLB",
		$i++ => "CLB",
		$i++ => "CLB",
		$i++ => "BRAM",
		$i++ => "CLB",
		$i++ => "CLB",
		$i++ => "DSP48",
		$i++ => "CLB",
		$i++ => "CLB",
		$i++ => "CLB",
		$i++ => "CLB",
		$i++ => "CLB",
		$i++ => "CLB",
		$i++ => "CLB",
		$i++ => "CLB",
		$i++ => "IOB",
		$i++ => "CLB",
		$i++ => "CLB",
		$i++ => "CLB",
		$i++ => "CLB",
		$i++ => "CLB",
		$i++ => "CLB",
		$i++ => "CLB",
		$i++ => "CLB",
		$i++ => "BRAM",
		$i++ => "CLB",
		$i++ => "CLB",
		$i++ => "CLB",
		$i++ => "CLB",
		$i++ => "IOB",
		$i++ => "CLB",
		$i++ => "CLB",
		$i++ => "CLB",
		$i++ => "CLB",
);

$width_refs{"xc5vlx30"} = $i;
my $xc5vlx30_rows = 2;

$i = 0;
my %xc5vlx85 = (
		$i++ => "IOB",
		$i++ => "CLB",
		$i++ => "CLB",
		$i++ => "CLB",
		$i++ => "CLB",
		$i++ => "BRAM",
		$i++ => "CLB",
		$i++ => "CLB",
		$i++ => "CLB",
		$i++ => "CLB",
		$i++ => "CLB",
		$i++ => "CLB",
		$i++ => "CLB",
		$i++ => "CLB",
		$i++ => "CLB",
		$i++ => "CLB",
		$i++ => "BRAM",
		$i++ => "CLB",
		$i++ => "CLB",
		$i++ => "DSP48",
		$i++ => "CLB",
		$i++ => "CLB",
		$i++ => "CLB",
		$i++ => "CLB",
		$i++ => "CLB",
		$i++ => "CLB",
		$i++ => "CLB",
		$i++ => "CLB",
		$i++ => "IOB",
		$i++ => "CLB",
		$i++ => "CLB",
		$i++ => "CLB",
		$i++ => "CLB",
		$i++ => "CLB",
		$i++ => "CLB",
		$i++ => "CLB",
		$i++ => "CLB",
		$i++ => "CLB",
		$i++ => "CLB",
		$i++ => "CLB",
		$i++ => "CLB",
		$i++ => "BRAM",
		$i++ => "CLB",
		$i++ => "CLB",
		$i++ => "CLB",
		$i++ => "CLB",
		$i++ => "CLB",
		$i++ => "CLB",
		$i++ => "CLB",
		$i++ => "CLB",
		$i++ => "CLB",
		$i++ => "CLB",
		$i++ => "BRAM",
		$i++ => "CLB",
		$i++ => "CLB",
		$i++ => "CLB",
		$i++ => "CLB",
		$i++ => "IOB",
		$i++ => "CLB",
		$i++ => "CLB",
		$i++ => "CLB",
		$i++ => "CLB",
);

$width_refs{"xc5vlx85"} = $i;
my $xc5vlx85_rows = 3;

my %xc5vlx110 = %xc5vlx85;
my $xc5vlx110_rows = $xc5vlx85_rows + 1;
$width_refs{"xc5vlx110"} = $i;

$i = 0;
my %xc5vlx220 = (
    $i++ => "IOB",
    $i++ => "CLB",
    $i++ => "CLB",
    $i++ => "CLB",
    $i++ => "CLB",
    $i++ => "BRAM",
    $i++ => "CLB",
    $i++ => "CLB",
    $i++ => "CLB",
    $i++ => "CLB",
    $i++ => "CLB",
    $i++ => "CLB",
    $i++ => "CLB",
    $i++ => "CLB",
    $i++ => "CLB",
    $i++ => "CLB",
    $i++ => "CLB",
    $i++ => "CLB",
    $i++ => "CLB",
    $i++ => "CLB",
    $i++ => "CLB",
    $i++ => "CLB",
    $i++ => "CLB",
    $i++ => "CLB",
    $i++ => "CLB",
    $i++ => "CLB",
    $i++ => "CLB",
    $i++ => "CLB",
    $i++ => "BRAM",
    $i++ => "CLB",
    $i++ => "CLB",
    $i++ => "DSP48",
    $i++ => "CLB",
    $i++ => "CLB",
    $i++ => "DSP48",
    $i++ => "CLB",
    $i++ => "CLB",
    $i++ => "BRAM",
    $i++ => "CLB",
    $i++ => "CLB",
    $i++ => "CLB",
    $i++ => "CLB",
    $i++ => "CLB",
    $i++ => "CLB",
    $i++ => "CLB",
    $i++ => "CLB",
    $i++ => "CLB",
    $i++ => "CLB",
    $i++ => "CLB",
    $i++ => "CLB",
    $i++ => "CLB",
    $i++ => "CLB",
    $i++ => "CLB",
    $i++ => "CLB",
    $i++ => "CLB",
    $i++ => "CLB",
    $i++ => "CLB",
    $i++ => "CLB",
    $i++ => "IOB",
    $i++ => "CLB",
    $i++ => "CLB",
    $i++ => "CLB",
    $i++ => "CLB",
    $i++ => "CLB",
    $i++ => "CLB",
    $i++ => "CLB",
    $i++ => "CLB",
    $i++ => "CLB",
    $i++ => "CLB",
    $i++ => "CLB",
    $i++ => "CLB",
    $i++ => "CLB",
    $i++ => "CLB",
    $i++ => "CLB",
    $i++ => "CLB",
    $i++ => "CLB",
    $i++ => "CLB",
    $i++ => "CLB",
    $i++ => "CLB",
    $i++ => "BRAM",
    $i++ => "CLB",
    $i++ => "CLB",
    $i++ => "CLB",
    $i++ => "CLB",
    $i++ => "CLB",
    $i++ => "CLB",
    $i++ => "BRAM",
    $i++ => "CLB",
    $i++ => "CLB",
    $i++ => "CLB",
    $i++ => "CLB",
    $i++ => "CLB",
    $i++ => "CLB",
    $i++ => "CLB",
    $i++ => "CLB",
    $i++ => "CLB",
    $i++ => "CLB",
    $i++ => "CLB",
    $i++ => "CLB",
    $i++ => "CLB",
    $i++ => "CLB",
    $i++ => "CLB",
    $i++ => "CLB",
    $i++ => "CLB",
    $i++ => "CLB",
    $i++ => "CLB",
    $i++ => "CLB",
    $i++ => "CLB",
    $i++ => "CLB",
    $i++ => "BRAM",
    $i++ => "CLB",
    $i++ => "CLB",
    $i++ => "CLB",
    $i++ => "CLB",
    $i++ => "IOB",
    $i++ => "CLB",
    $i++ => "CLB",
    $i++ => "CLB",
    $i++ => "CLB",
);

my $xc5vlx220_rows = 4;
$width_refs{"xc5vlx220"} = $i;

my %xc5vlx330 = %xc5vlx220;
my $xc5vlx330_rows = $xc5vlx220_rows + 1;
$width_refs{"xc5vlx330"} = $i;


my @sizes = ( 30, 85, 110, 220, 330 );
my $root = "xc5vlx";

my %dat_refs = (
		"xc5vlx30" => \%xc5vlx30,
		"xc5vlx85" => \%xc5vlx85,
		"xc5vlx110" => \%xc5vlx110,
		"xc5vlx220" => \%xc5vlx220,
		"xc5vlx330" => \%xc5vlx330,
		);

my %height_refs = (
		"xc5vlx30" => 2,
		"xc5vlx85" => 3,
		"xc5vlx110" => 4,
		"xc5vlx220" => 4,
		"xc5vlx330" => 5,
		);


sub print_ranges
{
    my ($ncol_type, $pfx, $output) = @_;
    my $varname = "$root"."$pfx";
    my $last_col;
    my $first = 1;
    my $i;
    my %ref = %{$dat_refs{$varname}};

#    print $output "printing range $ncol_type for chip $pfx";

    for ($i = 0; ;$i++) {
	my $cur_col = $ref{$i};
	if ( ! defined($cur_col) ) {
	    if ( $last_col eq $ncol_type ) {
		#print the thing
		print $output "$i";
	    }
	    last;
	}

#	print $output "current col is $cur_col, last col $last_col";

	if (! defined($last_col)) {
	    if ($cur_col eq $ncol_type) {
		#very beginning
		print $output "$i;";
		$first = 0;
	    }
	} elsif ("$cur_col" eq "$last_col") { } else {
#	    print $output "Chainsaw btw $cur_col and $ncol_type";
	    if ("$cur_col" eq "$ncol_type") {
#		print $output "Got it !";
		#beginning
		if ($first == 1) {
		    print $output "$i;";
		    $first = 0;
		} else {
		    print $output ";$i;";
		}
	    } elsif ("$last_col" eq "$ncol_type") {
		#end
		print $output "$i";
	    }
	}
	# if equal, do nothing
	$last_col = $cur_col;
    }
}

sub print_chip
{
    my $chip = $_[0];
    my $varname = "$root"."$chip";
    my $dirname = $varname;
    my $key;
    my $ncols = $height_refs{$varname} * 2 * 20;
    my $nrows = $width_refs{$varname};

    mkdir $dirname;
    my $filename = "${dirname}/chip_control";

    open(CTRL, ">$filename") or die "opening $filename";
    print CTRL "[DIMENTIONS]\n";
    print CTRL "WIDTH=$nrows\n";
    print CTRL "HEIGHT=$ncols\n";
    close(CTRL);

    # Generate the chip_descr file
    $filename = "${dirname}/chip_data";
    open(DESCR, ">$filename") || die "opening $filename";

    for $key (keys %C_enum) {
	my $number = $C_enum{$key};
	if ($number != $C_enum{NR_SITE_TYPE} &&
	    $number != $C_enum{SITE_TYPE_NEUTRAL} &&
	    $number != $C_enum{PAD} &&
	    $number != $C_enum{GCLK}) {
	    print DESCR "[$key]\n";
	    print DESCR "x=";
	    print_ranges $key, $chip, \*DESCR;
	    print DESCR "\n";
	    print DESCR "y=0;$ncols\n";
	    print DESCR "type=$C_enum{$key}\n";
	}
    }

    close DESCR;
}

sub print_family
{
    my $key;
    for $key (@sizes) {
	#open file
	print STDOUT "printing chip $key\n";
	print_chip $key;
    }
}

print_family;
