#! /usr/bin/env bash
$XGETTEXT `find plasmasearch/ plugininstaller -name "*.cpp" -o -name "*.h" -o -name "*.qml"` -o $podir/kcm_plasmasearch.pot
