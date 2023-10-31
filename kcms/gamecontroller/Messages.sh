# SPDX-FileCopyrightText: None
# SPDX-License-Identifier: CC0-1.0

#! /usr/bin/env bash
$EXTRACTRC `find . -name \*.kcfg` >> rc.cpp
$XGETTEXT `find . -name "*.cpp" -o -name "*.qml"` -o $podir/kcm_gamecontroller.pot
