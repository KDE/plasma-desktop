/*
    SPDX-FileCopyrightText: 2017 Roman Gilg <subdiff@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "touchpadconfigplugin.h"
#include "touchpadconfigcontainer.h"

TouchpadConfigPlugin::TouchpadConfigPlugin(KCModule *parent, TouchpadBackend *backend)
    : QWidget(parent->widget())
    , m_parent(dynamic_cast<TouchpadConfigContainer *>(parent))
    , m_backend(backend)
{
}
