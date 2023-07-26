/*
    SPDX-FileCopyrightText: 2017 Xuetian Weng <wengxt@gmail.com>
    SPDX-FileCopyrightText: 2018 Roman Gilg <subdiff@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "inputbackend.h"

#if BUILD_KCM_MOUSE_KWIN_WAYLAND
#include "backends/kwin_wl/kwin_wl_backend.h"
#endif
#if BUILD_KCM_MOUSE_X11
#include "backends/x11/x11_backend.h"
#endif

#include <logging.h>

#include <KWindowSystem>

InputBackend *InputBackend::implementation(QObject *parent)
{
    // There are multiple possible backends
#if BUILD_KCM_MOUSE_X11
    if (KWindowSystem::isPlatformX11()) {
        qCDebug(KCM_MOUSE) << "Using X11 backend";
        return X11Backend::implementation(parent);
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
