/*
 * Copyright 2017 Xuetian Weng <wengxt@gmail.com>
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
#include "inputbackend.h"

#include "backends/x11/x11_backend.h"
#include "backends/kwin_wl/kwin_wl_backend.h"
#include "logging.h"

#include <KWindowSystem/kwindowsystem.h>

InputBackend *InputBackend::implementation(QObject *parent)
{
    //There are multiple possible backends
    if (KWindowSystem::isPlatformX11()) {
        qCDebug(KCM_MOUSE) << "Using X11 backend";
        return X11Backend::implementation(parent);
    }
    else if (KWindowSystem::isPlatformWayland()) {
        qCDebug(KCM_MOUSE) << "Using KWin+Wayland backend";
        return new KWinWaylandBackend(parent);
    }
    else {
        qCCritical(KCM_MOUSE) << "Not able to select appropriate backend.";
        return nullptr;
    }
}
