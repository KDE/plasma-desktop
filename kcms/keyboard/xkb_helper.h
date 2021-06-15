/*
    SPDX-FileCopyrightText: 2010 Andriy Rysin <rysin@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef XKB_HELPER_H_
#define XKB_HELPER_H_

template<typename T>
class QList;
class LayoutUnit;
class QStringList;
class KeyboardConfig;

class XkbHelper
{
public:
    static bool initializeKeyboardLayouts(KeyboardConfig &config);
    static bool initializeKeyboardLayouts(const QList<LayoutUnit> &layouts);
    static bool runConfigLayoutCommand(const QStringList &setxkbmapCommandArguments);
};

#endif /* XKB_HELPER_H_ */
