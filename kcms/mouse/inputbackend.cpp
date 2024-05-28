/*
    SPDX-FileCopyrightText: 2017 Xuetian Weng <wengxt@gmail.com>
    SPDX-FileCopyrightText: 2018 Roman Gilg <subdiff@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "inputbackend.h"
#include "logging.h"

#include <KWindowSystem>

#include <qqml.h>

#if BUILD_KCM_MOUSE_KWIN_WAYLAND
#include "backends/kwin_wl/kwin_wl_backend.h"
#endif
#if BUILD_KCM_MOUSE_X11
#include "backends/x11/x11_libinput_backend.h"
#include <X11/Xlib.h>
#include <libinput-properties.h>
#include <private/qtx11extras_p.h>
#endif

InputBackend *InputBackend::implementation(QObject *parent)
{
    // There are multiple possible backends
#if BUILD_KCM_MOUSE_X11
    if (KWindowSystem::isPlatformX11()) {
        qCDebug(KCM_MOUSE) << "Using X11 backend";

        Atom testAtom = XInternAtom(QX11Info::display(), LIBINPUT_PROP_ACCEL, True);

        if (testAtom) {
            qCDebug(KCM_MOUSE) << "Using libinput driver on X11.";
            return new X11LibinputBackend(parent);
        }
    }
#endif
#if BUILD_KCM_MOUSE_KWIN_WAYLAND
    if (KWindowSystem::isPlatformWayland()) {
        qCDebug(KCM_MOUSE) << "Using KWin+Wayland backend";
        return new KWinWaylandBackend(parent);
    }
#endif

    qCCritical(KCM_MOUSE) << "Not able to select appropriate backend.";
    return nullptr;
}

void InputBackend::registerImplementationTypes(const char *uri)
{
#if BUILD_KCM_MOUSE_X11
    qmlRegisterUncreatableType<X11LibinputBackend>(uri, 1, 0, "X11LibinputBackend", QString());
#endif
#if BUILD_KCM_MOUSE_KWIN_WAYLAND
    qmlRegisterUncreatableType<KWinWaylandBackend>(uri, 1, 0, "KWinWaylandBackend", QString());
#endif
}

#include <fixx11h.h>

#include "moc_inputbackend.cpp"
