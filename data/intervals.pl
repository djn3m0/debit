#! /usr/bin/perl -w

# This script is the common glue used to generate chip_control and
# chip_data databases.

use strict;

my %vert_types;
my %horiz_types;
my %vert_length;
my %horiz_length;

sub register_interval_vert
{
    my ($type, $start, $leng) = @_;
    $vert_length{$start} = $leng;
    $vert_types{ $type }{ $start } = $leng;
}

sub register_interval_horiz
{
    my ($type, $start, $leng) = @_;
#    print "Registering interval $start for $type\n";
    $horiz_length{$start} = $leng;
    $horiz_types{$type}{$start} = $leng;
}

sub print_database_std
{
    my ($output, $db) = @_;
    my %C_enum = %{ $db };
    # Standard database printing
    # Iterate over types...

    # Fill horiz_pos and vert_pos
    my %horiz_pos;
    my @intervals = sort {$a <=> $b} ( keys %horiz_length );
    my $i = 0;
    for my $long ( @intervals ) {
	$horiz_pos{$long} = $i++;
    }
    $i = 0;
    my %vert_pos;
    @intervals = sort {$a <=> $b} ( keys %vert_length );
    for my $long ( @intervals ) {
	$vert_pos{$long} = $i++;
    }

    for my $type ( keys %horiz_types ) {
	print $output "[$type]\n";
	print $output "x=";
	# Sort things...
	@intervals = sort {$a <=> $b} ( keys %{$horiz_types{$type}} );
	my $first = 1;
	for my $start (@intervals) {
	    my $len = $horiz_types{$type}{$start};
	    if ($first) {
		$first = 0;
	    } else {
		print_sep($output);
	    }
	    print_interval($output,$start,$start+$len);
	}
	print $output "\n";
	print $output "xpos=";
	$first = 1;
	for my $start ( @intervals ) {
	    my $pos = $horiz_pos{$start};
	    if ($first) {
		$first = 0;
	    } else {
		print_sep($output);
	    }
	    print_pos($output,$pos);
	}
	print $output "\n";
	print $output "y=";
	@intervals = sort {$a <=> $b} ( keys %{$vert_types{$type}} );
	$first = 1;
	for my $start ( @intervals ) {
	    my $len = $vert_types{$type}{$start};
	    if ($first) {
		$first = 0;
	    } else {
		print_sep($output);
	    }
	    print_interval($output,$start,$start+$len);
	}
	print $output "\n";
	print $output "ypos=";
	$first = 1;
	for my $start ( @intervals ) {
	    my $pos = $vert_pos{$start};
	    if ($first) {
		$first = 0;
	    } else {
		print_sep($output);
	    }
	    print_pos($output,$pos);
	}
	print $output "\n";
	print $output "type=$C_enum{$type}\n";
	print $output "\n";
    }
}

sub integrate
{
    my $tlen = 0;
    for my $len ( @_ ) {
	$tlen += $len;
    }
    return $tlen;
}

sub print_database_ctrl
{
    my ($output) = @_;
    # print control things
    # XXX fix this horror
    print $output "[DIMENTIONS]\n";
    my $width = integrate (values %horiz_length);
    print $output "WIDTH=$width\n";
    my $height = integrate (values %vert_length);
    print $output "HEIGHT=$height\n";

    my $nwidth = scalar (keys %horiz_length);
    my $nheight = scalar (keys %vert_length);
    print $output "CWIDTH=$nwidth\n";
    print $output "CHEIGHT=$nheight\n";
}

sub reset_database
{
 %vert_types = ();
 %horiz_types = ();
 %vert_length = ();
 %horiz_length = ();
}

sub print_interval
{
    my ($out, $start, $end) = @_;
    print $out $start;
    print $out ";";
    print $out $end;
}

sub print_pos
{
    my ($out, $pos) = @_;
    print $out $pos;
}

sub print_sep
{
    my ($out) = @_;
    print $out ";";
}

1;
