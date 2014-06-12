#! /usr/bin/env bash
$EXTRACTRC *.ui >> rc.cpp
$XGETTEXT -ktranslate:1,1t -ktranslate:1c,2,2t *.cpp -o $podir/kcmtranslations.pot
rm -f rc.cpp
