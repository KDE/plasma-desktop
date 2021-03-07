/*
 * Copyright 2017 Roman Gilg <subdiff@gmail.com>
 * Copyright 2013 Alexander Mezin <mezin.alexander@gmail.com>
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

#include "touchpadbackend.h"

#include "backends/kwin_wayland/kwinwaylandbackend.h"
#include "backends/x11/xlibbackend.h"
#include "logging.h"
#include "touchpadparameters.h"

#include <QSharedPointer>
#include <QThreadStorage>

#include <KWindowSystem>

void touchpadApplySavedConfig()
{
    TouchpadBackend *backend = TouchpadBackend::implementation();
    if (!backend) {
        return;
    }

    TouchpadParameters config;
    backend->applyConfig(config.values());
}

void TouchpadBackend::setMode(TouchpadInputBackendMode mode)
{
    m_mode = mode;
}

TouchpadBackend *TouchpadBackend::implementation()
{
    // There are multiple possible backends
    if (KWindowSystem::isPlatformX11()) {
        static QThreadStorage<QSharedPointer<XlibBackend>> backend;
        if (!backend.hasLocalData()) {
            qCDebug(KCM_TOUCHPAD) << "Using X11 backend";
            backend.setLocalData(QSharedPointer<XlibBackend>(XlibBackend::initialize()));
        }
        return backend.localData().data();
    }
    // TODO: test on kwin_wayland specifically? What about possibly other compositors under Wayland?
    else if (KWindowSystem::isPlatformWayland()) {
        qCDebug(KCM_TOUCHPAD) << "Using KWin+Wayland backend";
        return (new KWinWaylandBackend());
    } else {
        qCCritical(KCM_TOUCHPAD) << "Not able to select appropriate backend.";
        return nullptr;
    }
}
