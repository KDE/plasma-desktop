/*
    SPDX-FileCopyrightText: 2018 Roman Gilg <subdiff@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "configplugin.h"
#include "configcontainer.h"
#include "inputbackend.h"

#include "libinput/libinput_config.h"
#include "xlib/xlib_config.h"

#include <logging.h>

ConfigPlugin *ConfigPlugin::implementation(ConfigContainer *parent)
{
    InputBackend *backend = InputBackend::implementation(parent);
    InputBackendMode mode = backend->mode();

    if (mode == InputBackendMode::KWinWayland || mode == InputBackendMode::XLibinput) {
        qCDebug(KCM_MOUSE) << "With libinput user interface.";
        return new LibinputConfig(parent, backend);
    } else if (mode == InputBackendMode::XEvdev) {
        qCDebug(KCM_MOUSE) << "With X11 evdev user interface.";
        return new XlibConfig(parent, backend);
    } else {
        qCCritical(KCM_MOUSE) << "Not able to select appropriate backend.";
        return nullptr;
    }
}

ConfigPlugin::ConfigPlugin(ConfigContainer *parent)
    : QWidget(parent)
    , m_parent(parent)
{
}
