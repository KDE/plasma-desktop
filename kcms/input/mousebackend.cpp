/*
 * Copyright 2017 Xuetian Weng <wengxt@gmail.com>
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

#include "mousebackend.h"

#include "backends/x11/x11mousebackend.h"
#include "logging.h"

#include <QThreadStorage>
#include <QSharedPointer>

#include <QGuiApplication>

MouseBackend *MouseBackend::implementation()
{
    //There are multiple possible backends, always use X11 backend for now.
    static QThreadStorage<QSharedPointer<X11MouseBackend>> backend;
    if (!backend.hasLocalData()) {
        qCDebug(KCM_INPUT) << "Using X11 backend";
        backend.setLocalData(QSharedPointer<X11MouseBackend>(new X11MouseBackend));
    }
    return backend.localData().data();

#if 0
    qCCritical(KCM_INPUT) << "Not able to select appropriate backend.";
    return nullptr;
#endif
}
