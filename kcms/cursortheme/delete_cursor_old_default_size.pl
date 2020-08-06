#! /usr/bin/perl

use strict;

while (<>)
{
    chomp;
    s/cursorSize=0/# DELETE cursorSize/;
    print "$_\n";
}
