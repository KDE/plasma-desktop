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
    explicit KeyboardConfig(KeyboardSettings *settings, QObject *parent) noexcept;

    const QList<LayoutUnit> &layouts() const;
    QList<LayoutUnit> &layouts();

    KeyboardSettings *keyboardSettings() const;

    bool isDefaults() const;
    bool isSaveNeeded() const;

private:
    bool layoutsSaveNeeded() const;
    bool isDefaultsLayouts() const;

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
