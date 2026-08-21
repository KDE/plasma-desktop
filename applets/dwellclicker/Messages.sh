#! /usr/bin/env bash

# SPDX-FileCopyrightText: 2026 Sebastian Sauer <dipesh@gmx.de>
# SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

$XGETTEXT `find . -name \*.js -o -name \*.qml -o -name \*.cpp` -o $podir/plasma_applet_org.kde.plasma.dwellclicker.pot
