#! /usr/bin/env bash
# customkeys=`grep "^.include .\.\." keyconfig.cpp | sed -e "s#.*\"\(.*\)\"#\1#"`
# $XGETTEXT *.cpp $customkeys -o $podir/kcmkeys.pot
$EXTRACTRC *.ui >> rc.cpp
$XGETTEXT *.cpp -o $podir/kcmkeys.pot
rm -f rc.cpp

