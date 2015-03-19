#! /usr/bin/env bash
### TODO: why do we need 2 POT files for a single directory?
$EXTRACTRC *.ui >> rc.cpp
$XGETTEXT kaccess.cpp main.cpp rc.cpp -o $podir/kaccess.pot
$XGETTEXT kcmaccess.cpp rc.cpp -o $podir/kcmaccess.pot
