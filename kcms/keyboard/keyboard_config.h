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
    void notifyLayoutsChanged();

    void resetLayouts(); // call after settings load() and defaults()
    void applyLayoutsToSettings(); // call before settings save()

    KeyboardSettings *keyboardSettings() const;

private:
    KeyboardSettings *const m_settings;

    QList<LayoutUnit> m_layouts;
};
