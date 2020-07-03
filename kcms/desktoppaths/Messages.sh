#! /usr/bin/env bash
$EXTRACTRC `find . -name "*.ui"` >> rc.cpp || exit 11
$XGETTEXT *.cpp -o $podir/kcm_desktoppaths.pot
