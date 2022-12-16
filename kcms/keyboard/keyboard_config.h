/*
    SPDX-FileCopyrightText: 2010 Andriy Rysin <rysin@kde.org>
    SPDX-FileCopyrightText: 2021 Cyril Rossi <cyril.rossi@enioka.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "x11_helper.h"

#include <QList>
#include <QMap>
#include <QPair>
#include <QString>
#include <QStringList>

#include "keyboardsettings.h"

/**
 * This class provides configuration options for keyboard module
 */
class KeyboardConfig : public KeyboardSettingsBase
{
public:
    static const int NO_LOOPING; // = -1;

    enum SwitchingPolicy {
        SWITCH_POLICY_GLOBAL = 0,
        SWITCH_POLICY_DESKTOP = 1,
        SWITCH_POLICY_APPLICATION = 2,
        SWITCH_POLICY_WINDOW = 3,
    };

    QList<LayoutUnit> layouts;

    KeyboardConfig(QObject *parent);

    // Getter/setter for switchMode options with SwitchingPolicy enum type values
    SwitchingPolicy switchingPolicy() const;
    void setSwitchingPolicy(SwitchingPolicy switchingPolicy);
    SwitchingPolicy defaultSwitchingPolicyValue() const;

    bool layoutsSaveNeeded() const;

    // Initialize layouts list when activating 'Configure Layouts' option and there is none.
    QList<LayoutUnit> getDefaultLayouts() const;
    QList<LayoutUnit> getExtraLayouts() const;

    void setDefaults() override;
    void load();
    void save();

    static QString getSwitchingPolicyString(SwitchingPolicy switchingPolicy);

private:
    QList<LayoutUnit> m_referenceLayouts;
    int m_referenceLayoutLoopCount;
};
