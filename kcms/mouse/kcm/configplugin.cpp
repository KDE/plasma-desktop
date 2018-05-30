/*
 * Copyright 2018 Roman Gilg <subdiff@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "configplugin.h"
#include "configcontainer.h"
#include "inputbackend.h"

#include "libinput/libinput_config.h"
#include "xlib/xlib_config.h"

#include <logging.h>

ConfigPlugin* ConfigPlugin::implementation(ConfigContainer *parent)
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
    : QWidget(parent),
      m_parent(parent)
{
}
