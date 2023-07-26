/*
    SPDX-FileCopyrightText: 2018 Roman Gilg <subdiff@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "configplugin.h"
#include "configcontainer.h"
#include "inputbackend.h"

#include "libinput/libinput_config.h"

#include <config-build-options.h>

#if BUILD_KCM_MOUSE_X11
#include "xlib/xlib_config.h"
#endif

#include <logging.h>

ConfigPlugin *ConfigPlugin::implementation(ConfigContainer *parent)
{
    InputBackend *backend = InputBackend::implementation(parent);
    InputBackendMode mode = backend->mode();

#if BUILD_KCM_MOUSE_KWIN_WAYLAND
    if (mode == InputBackendMode::KWinWayland) {
        qCDebug(KCM_MOUSE) << "With libinput user interface.";
        return new LibinputConfig(parent, backend);
    }
#endif
#if BUILD_KCM_MOUSE_X11
    if (mode == InputBackendMode::XLibinput) {
        qCDebug(KCM_MOUSE) << "With libinput user interface.";
        return new LibinputConfig(parent, backend);
    }
    if (mode == InputBackendMode::XEvdev) {
        qCDebug(KCM_MOUSE) << "With X11 evdev user interface.";
        return new XlibConfig(parent, backend);
    }
#endif
    qCCritical(KCM_MOUSE) << "Not able to select appropriate backend.";
    return nullptr;
}

ConfigPlugin::ConfigPlugin(ConfigContainer *parent)
    : QWidget(parent->widget())
    , m_parent(parent)
{
}
