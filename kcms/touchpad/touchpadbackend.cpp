/*
    SPDX-FileCopyrightText: 2017 Roman Gilg <subdiff@gmail.com>
    SPDX-FileCopyrightText: 2013 Alexander Mezin <mezin.alexander@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
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
