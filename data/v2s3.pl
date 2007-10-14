#! /usr/bin/perl -w

# This script is the common reg fucntions for v2 and s3 chips

use strict;

my $y_offset = 2;
my $x_offset = 2;

sub print_clb_height
{
    my ($ncols, $type) = @_;
    register_interval_vert($type,$y_offset,$ncols);
}

# This prints a 1-width thing
sub print_isolated
{
    my ($type, $start, $dir) = @_;
    my $end = $start + 1;
    if ($dir =~ "y") {
	register_interval_vert($type,$start,$end-$start);
    } else {
	register_interval_horiz($type,$start,$end-$start);
    }
}

sub print_top_height
{
    my ( $type) = @_;
    print_isolated($type, $y_offset - 1, "y");
}

sub print_bottom_height
{
    my ($ncols, $type) = @_;
    my $last_y = $y_offset + $ncols;
    print_isolated($type, $last_y, "y");
}

sub print_left_width
{
    my ( $type) = @_;
    print_isolated($type, $x_offset - 1, "x");
}

sub print_right_width
{
    my ($ncols, $brams, $type) = @_;
    my $last_x = $x_offset + $brams + $ncols;
    print_isolated($type, $last_x, "x");
}

sub print_termtop_height
{
    my ( $type) = @_;
    print_isolated($type, "0", "y");
}

sub print_termbottom_height
{
    my ($ncols, $type) = @_;
    my $last_y = $y_offset + $ncols + 1;
    print_isolated($type, $last_y, "y");
}

sub print_termleft_width
{
    my ( $type) = @_;
    print_isolated($type, "0", "x");
}

sub print_termright_width
{
    my ($ncols, $brams, $type) = @_;
    my $last_x = $x_offset + $ncols + $brams + 1;
    print_isolated($type, $last_x, "x");
}

1;
