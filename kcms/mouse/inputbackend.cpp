/*
    SPDX-FileCopyrightText: 2017 Xuetian Weng <wengxt@gmail.com>
    SPDX-FileCopyrightText: 2018 Roman Gilg <subdiff@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "inputbackend.h"

#include "backends/kwin_wl/kwin_wl_backend.h"
#include "backends/x11/x11_backend.h"
#include "logging.h"

#include <KWindowSystem/kwindowsystem.h>

InputBackend *InputBackend::implementation(QObject *parent)
{
    // There are multiple possible backends
    if (KWindowSystem::isPlatformX11()) {
        qCDebug(KCM_MOUSE) << "Using X11 backend";
        return X11Backend::implementation(parent);
    } else if (KWindowSystem::isPlatformWayland()) {
        qCDebug(KCM_MOUSE) << "Using KWin+Wayland backend";
        return new KWinWaylandBackend(parent);
    } else {
        qCCritical(KCM_MOUSE) << "Not able to select appropriate backend.";
        return nullptr;
    }
}
