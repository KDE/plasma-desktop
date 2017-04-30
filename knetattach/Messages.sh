#! /usr/bin/env bash
$EXTRACTRC *.ui >> rc.cpp
$XGETTEXT *.cpp *.h -o $podir/knetattach5.pot
rm -f rc.cpp
