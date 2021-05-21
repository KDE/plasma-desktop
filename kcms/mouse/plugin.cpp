/*
    SPDX-FileCopyrightText: 2018 Roman Gilg <subdiff@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "kcm/configcontainer.h"

#include <KPluginFactory>

K_PLUGIN_FACTORY(MousePluginFactory, registerPlugin<ConfigContainer>();)

#include <plugin.moc>
