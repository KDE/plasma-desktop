/*
    SPDX-FileCopyrightText: 2010 Andriy Rysin <rysin@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KEYBOARD_CONFIG_H_
#define KEYBOARD_CONFIG_H_

#include "x11_helper.h"

#include <QList>
#include <QMap>
#include <QPair>
#include <QString>
#include <QStringList>

/**
 * This class provides configuration options for keyboard module
 */
class KeyboardConfig
{
public:
    static const int MAX_LABEL_LEN = 3;
    static const int NO_LOOPING; // = -1;

    enum SwitchingPolicy {
        SWITCH_POLICY_GLOBAL = 0,
        SWITCH_POLICY_DESKTOP = 1,
        SWITCH_POLICY_APPLICATION = 2,
        SWITCH_POLICY_WINDOW = 3,
    };

    QString keyboardModel;
    // resetOldXkbOptions is now also "set xkb options"
    bool resetOldXkbOptions;
    QStringList xkbOptions;

    // init layouts options
    bool configureLayouts;
    QList<LayoutUnit> layouts;
    int layoutLoopCount;

    // switch control options
    SwitchingPolicy switchingPolicy;
    //	bool stickySwitching;
    //	int stickySwitchingDepth;

    KeyboardConfig();

    QList<LayoutUnit> getDefaultLayouts() const;
    QList<LayoutUnit> getExtraLayouts() const;

    void setDefaults();
    void load();
    void save();

    static QString getSwitchingPolicyString(SwitchingPolicy switchingPolicy);
};

#endif /* KEYBOARD_CONFIG_H_ */
