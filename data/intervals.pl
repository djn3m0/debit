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
    for my $type ( keys %horiz_types ) {
	my $start;
	print $output "[$type]\n";
	print $output "x=";
	# Sort things...
	my @intervals = sort {$a <=> $b} ( keys %{$horiz_types{$type}} );
	my $first = 1;
	for $start (@intervals) {
	    my $len = $horiz_types{$type}{$start};
	    if ($first) {
		$first = 0;
	    } else {
		print_sep($output);
	    }
	    print_interval($output,$start,$start+$len);
	}
	print $output "\n";
	print $output "y=";
	$first = 1;
	for $start ( keys %{$vert_types{$type}} ) {
	    my $len = $vert_types{$type}{$start};
	    if ($first) {
		$first = 0;
	    } else {
		print_sep($output);
	    }
	    print_interval($output,$start,$start+$len);
	}
	print $output "\n";
	print $output "type=$C_enum{$type}\n";
	print $output "\n";
    }
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

sub print_sep
{
    my ($out) = @_;
    print $out ";";
}

1;
