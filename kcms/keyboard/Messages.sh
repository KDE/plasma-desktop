#! /usr/bin/env bash
$EXTRACTRC kcm*.ui >> rc.cpp
$XGETTEXT rc.cpp kcmmisc.cpp preview/*.cpp -o $podir/kcmkeyboard.pot
$XGETTEXT kcm_*.cpp keyboard_*.cpp layout_*.cpp flags.cpp layouts_menu.cpp bindings.cpp -o $podir/kxkb.pot
rm -f rc.cpp
