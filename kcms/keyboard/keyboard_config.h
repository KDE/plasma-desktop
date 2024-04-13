/*
    SPDX-FileCopyrightText: 2010 Andriy Rysin <rysin@kde.org>
    SPDX-FileCopyrightText: 2021 Cyril Rossi <cyril.rossi@enioka.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QObject>

class KeyboardSettings;
class LayoutUnit;

class KeyboardConfig final : public QObject
{
    Q_OBJECT

public:
    static const int NO_LOOPING; // = -1;

    enum SwitchingPolicy {
        SWITCH_POLICY_GLOBAL = 0,
        SWITCH_POLICY_DESKTOP = 1,
        SWITCH_POLICY_APPLICATION = 2,
        SWITCH_POLICY_WINDOW = 3,
    };

public:
    explicit KeyboardConfig(KeyboardSettings *settings, QObject *parent) noexcept;

    SwitchingPolicy switchingPolicy() const;
    void setSwitchingPolicy(SwitchingPolicy switchingPolicy);

    const QList<LayoutUnit> &layouts() const;
    QList<LayoutUnit> &layouts();

    QList<LayoutUnit> defaultLayouts() const;
    QList<LayoutUnit> extraLayouts() const;

    KeyboardSettings *keyboardSettings() const;

    bool isDefaults() const;
    bool isSaveNeeded() const;

    // Initialize layouts list when activating 'Configure Layouts' option and there is none.
    QList<LayoutUnit> getDefaultLayouts() const;
    QList<LayoutUnit> getExtraLayouts() const;

    static QString getSwitchingPolicyString(SwitchingPolicy switchingPolicy);

private:
    SwitchingPolicy policyFromString(const QString &string) const;

    bool layoutsSaveNeeded() const;
    bool isDefaultsLayouts() const;

Q_SIGNALS:
    void switchingPolicyChanged();

public Q_SLOTS:
    void save();
    void load();
    void defaults();

private:
    KeyboardSettings *const m_settings;

    QList<LayoutUnit> m_layouts;
    QList<LayoutUnit> m_referenceLayouts;
    int m_referenceLayoutLoopCount;
};
