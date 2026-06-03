/*
    SPDX-FileCopyrightText: 2017 Xuetian Weng <wengxt@gmail.com>
    SPDX-FileCopyrightText: 2018 Roman Gilg <subdiff@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "inputbackend.h"
#include "logging.h"

#include <KWindowSystem>

#include <qqml.h>

#include "backends/kwin_wl/kwin_wl_backend.h"

std::unique_ptr<InputBackend> InputBackend::create()
{
    if (KWindowSystem::isPlatformWayland()) {
        qCDebug(KCM_MOUSE) << "Using KWin+Wayland backend";
        return std::make_unique<KWinWaylandBackend>();
    }

    qCCritical(KCM_MOUSE) << "Not able to select appropriate backend.";
    return nullptr;
}

void InputBackend::registerImplementationTypes(const char *uri)
{
    qmlRegisterUncreatableType<KWinWaylandBackend>(uri, 1, 0, "KWinWaylandBackend", QString());
}

#include <fixx11h.h>

#include "moc_inputbackend.cpp"
