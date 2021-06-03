/*
    SPDX-FileCopyrightText: 2021 Cyril Rossi <cyril.rossi@enioka.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kcm_keyboard.h"
#include "keyboardsettingsdata.h"

#include <KPluginFactory>

K_PLUGIN_FACTORY(KeyboardModuleFactory, registerPlugin<KCMKeyboard>(); registerPlugin<KeyboardSettingsData>();)

#include "kcmmain.moc"
