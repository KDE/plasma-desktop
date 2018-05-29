/*
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

#include "x11_libinput_backend.h"
#include "x11_libinput_dummydevice.h"

X11LibinputBackend::X11LibinputBackend(QObject *parent) :
    X11Backend(parent)
{
    m_mode = InputBackendMode::XLibinput;
    m_device = new X11LibinputDummyDevice(this, m_dpy);
}

bool X11LibinputBackend::applyConfig()
{
    return static_cast<X11LibinputDummyDevice*>(m_device)->applyConfig();
}

bool X11LibinputBackend::getConfig()
{
    return static_cast<X11LibinputDummyDevice*>(m_device)->getConfig();
}

bool X11LibinputBackend::getDefaultConfig()
{
    return static_cast<X11LibinputDummyDevice*>(m_device)->getDefaultConfig();
}

bool X11LibinputBackend::isChangedConfig() const
{
    return static_cast<X11LibinputDummyDevice*>(m_device)->isChangedConfig();
}

void X11LibinputBackend::kcmInit()
{
    getConfig();
    applyConfig();
    X11Backend::kcmInit();
}
