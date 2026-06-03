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

std::unique_ptr<InputBackend> InputBackend::create()
{
#if BUILD_KCM_MOUSE_KWIN_WAYLAND
    if (KWindowSystem::isPlatformWayland()) {
        qCDebug(KCM_MOUSE) << "Using KWin+Wayland backend";
        return std::make_unique<KWinWaylandBackend>();
    }
#endif

    qCCritical(KCM_MOUSE) << "Not able to select appropriate backend.";
    return nullptr;
}

void InputBackend::registerImplementationTypes(const char *uri)
{
#if BUILD_KCM_MOUSE_KWIN_WAYLAND
    qmlRegisterUncreatableType<KWinWaylandBackend>(uri, 1, 0, "KWinWaylandBackend", QString());
#endif
}

#include <fixx11h.h>

#include "moc_inputbackend.cpp"
