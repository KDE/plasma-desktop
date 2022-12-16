/*
    SPDX-FileCopyrightText: 2021 Cyril Rossi <cyril.rossi@enioka.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QObject>

#include <KCModuleData>

class KeyboardConfig;
class KeyboardMiscSettings;

class KeyboardSettingsData : public KCModuleData
{
    Q_OBJECT

public:
    explicit KeyboardSettingsData(QObject *parent = nullptr, const QVariantList &args = QVariantList());

    KeyboardConfig *keyboardConfig() const;
    KeyboardMiscSettings *miscSettings() const;

private:
    KeyboardConfig *m_keyboardConfig;
    KeyboardMiscSettings *m_miscSettings;
};
