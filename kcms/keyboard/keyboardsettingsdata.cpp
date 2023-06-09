/*
    SPDX-FileCopyrightText: 2021 Cyril Rossi <cyril.rossi@enioka.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "keyboardsettingsdata.h"

#include "keyboard_config.h"
#include "keyboardmiscsettings.h"

KeyboardSettingsData::KeyboardSettingsData(QObject *parent)
    : KCModuleData(parent)
    , m_keyboardConfig(new KeyboardConfig(this))
    , m_miscSettings(new KeyboardMiscSettings(this))
{
    autoRegisterSkeletons();
}

KeyboardConfig *KeyboardSettingsData::keyboardConfig() const
{
    return m_keyboardConfig;
}

KeyboardMiscSettings *KeyboardSettingsData::miscSettings() const
{
    return m_miscSettings;
}
