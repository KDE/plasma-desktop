#! /usr/bin/perl

use strict;

while (<>)
{
    chomp;
    s/widgetStyle=breeze/widgetStyle=Breeze/;
    print "$_\n";
}
