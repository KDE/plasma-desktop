#! /usr/bin/env bash
$XGETTEXT `find . -name "*.cpp" -o -name "*.h"` -o $podir/kcm_search.pot

$XGETTEXT `find . -name "*.qml"` -o $podir/kcm_krunnersettings.pot
