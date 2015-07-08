/*
 * Copyright (C) 2013 Alexander Mezin <mezin.alexander@gmail.com>
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

#include <QThreadStorage>
#include <QSharedPointer>
#include <QX11Info>

#include "backends/x11/xlibbackend.h"

TouchpadBackend::TouchpadBackend(QObject *parent) : QObject(parent)
{
}

TouchpadBackend *TouchpadBackend::implementation()
{
    //There will be multiple backends later
    static QThreadStorage<QSharedPointer<XlibBackend> > backend;
    if (!backend.hasLocalData()) {
        if (QX11Info::isPlatformX11()) {
            backend.setLocalData(QSharedPointer<XlibBackend>(XlibBackend::initialize()));
        }
    }
    return backend.localData().data();
}
