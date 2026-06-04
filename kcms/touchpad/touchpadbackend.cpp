/*
    SPDX-FileCopyrightText: 2017 Roman Gilg <subdiff@gmail.com>
    SPDX-FileCopyrightText: 2013 Alexander Mezin <mezin.alexander@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "touchpadbackend.h"

#include "backends/kwin_wayland/kwinwaylandbackend.h"

#include "logging.h"

#include <QThreadStorage>

#include <KWindowSystem>

void TouchpadBackend::setMode(TouchpadInputBackendMode mode)
{
    m_mode = mode;
}

TouchpadBackend *TouchpadBackend::implementation()
{
    // TODO: test on kwin_wayland specifically? What about possibly other compositors under Wayland?
    if (KWindowSystem::isPlatformWayland()) {
        static QThreadStorage<std::shared_ptr<KWinWaylandBackend>> backend;
        if (!backend.hasLocalData()) {
            qCDebug(KCM_TOUCHPAD) << "Using KWin+Wayland backend";
            backend.setLocalData(std::shared_ptr<KWinWaylandBackend>(new KWinWaylandBackend()));
        }
        return backend.localData().get();
    }

    qCCritical(KCM_TOUCHPAD) << "Not able to select appropriate backend.";
    return nullptr;
}

#include "moc_touchpadbackend.cpp"
