#! /usr/bin/env bash
$XGETTEXT `find . -name \*.cc -o -name \*.cpp -o -name \*.h -o -name \*.qml` -o $podir/kcm5_kded.pot
