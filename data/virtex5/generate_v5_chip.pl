#! /usr/bin/perl -w

use strict;
require "../intervals.pl";

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

my %xc5vlx50 = %xc5vlx30;
my $xc5vlx50_rows = $xc5vlx30_rows + 1;
$width_refs{"xc5vlx50"} = $i;

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


my @sizes = ( 30, 50, 85, 110, 220, 330 );
my $root = "xc5vlx";

my %dat_refs = (
		"xc5vlx30" => \%xc5vlx30,
		"xc5vlx50" => \%xc5vlx50,
		"xc5vlx85" => \%xc5vlx85,
		"xc5vlx110" => \%xc5vlx110,
		"xc5vlx220" => \%xc5vlx220,
		"xc5vlx330" => \%xc5vlx330,
		);

my %height_refs = (
		"xc5vlx30" => 2,
		"xc5vlx50" => 3,
		"xc5vlx85" => 3,
		"xc5vlx110" => 4,
		"xc5vlx220" => 4,
		"xc5vlx330" => 5,
		);


sub print_ranges
{
    my ($ncol_type, $pfx) = @_;
    my $varname = "$root"."$pfx";
    my ($last_col, $start_of_col);
    my %ref = %{$dat_refs{$varname}};

    for (my $i = 0; ;$i++) {
	my $cur_col = $ref{$i};
	if ( ! defined($cur_col) ) {
	    if ( $last_col eq $ncol_type ) {
		register_interval_horiz($ncol_type, $start_of_col, $i-$start_of_col);
	    }
	    last;
	}

	if (! defined($last_col)) {
	    if ($cur_col eq $ncol_type) {
		#very beginning
		$start_of_col = $i;
	    }
	} elsif ("$cur_col" eq "$last_col") {
	    # two adjascent same column types
	} else {
            # two adjascent columns of differing type
	    if ("$cur_col" eq "$ncol_type") {
		# the new column is what we look for
		$start_of_col = $i;
	    } elsif ("$last_col" eq "$ncol_type") {
		# end-of row of what we look for
		register_interval_horiz($ncol_type, $start_of_col, $i-$start_of_col);
	    }
	}

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
    open(CTRL, ">${dirname}/chip_control") or die "opening control";
    open(DESCR, ">${dirname}/chip_data") || die "opening data";

    for $key (keys %C_enum) {
	my $number = $C_enum{$key};
	if ($number != $C_enum{NR_SITE_TYPE} &&
	    $number != $C_enum{SITE_TYPE_NEUTRAL} &&
	    $number != $C_enum{PAD} &&
	    $number != $C_enum{GCLK}) {
	    print_ranges $key, $chip;
	    register_interval_vert($key,0,$ncols);
	}
    }

    print_database_std(\*DESCR,\%C_enum);
    print_database_ctrl(\*CTRL);
    reset_database();

    close CTRL;
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
