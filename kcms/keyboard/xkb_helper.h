/*
    SPDX-FileCopyrightText: 2010 Andriy Rysin <rysin@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QStringList>

class LayoutUnit;
class KeyboardConfig;

class XkbHelper
{
public:
    static bool initializeKeyboardLayouts(KeyboardConfig &config);
    static bool initializeKeyboardLayouts(const QList<LayoutUnit> &layouts);
    static bool runConfigLayoutCommand(const QStringList &setxkbmapCommandArguments);
};
