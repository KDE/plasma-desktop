/*
    SPDX-FileCopyrightText: 2018 Roman Gilg <subdiff@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "x11_libinput_backend.h"
#include "x11_libinput_dummydevice.h"

X11LibinputBackend::X11LibinputBackend(QObject *parent)
    : X11Backend(parent)
{
    m_mode = InputBackendMode::XLibinput;
    m_device = new X11LibinputDummyDevice(this, m_dpy);
}

bool X11LibinputBackend::applyConfig()
{
    return static_cast<X11LibinputDummyDevice *>(m_device)->applyConfig();
}

bool X11LibinputBackend::getConfig()
{
    return static_cast<X11LibinputDummyDevice *>(m_device)->getConfig();
}

bool X11LibinputBackend::getDefaultConfig()
{
    return static_cast<X11LibinputDummyDevice *>(m_device)->getDefaultConfig();
}

bool X11LibinputBackend::isChangedConfig() const
{
    return static_cast<X11LibinputDummyDevice *>(m_device)->isChangedConfig();
}

void X11LibinputBackend::kcmInit()
{
    getConfig();
    applyConfig();
    X11Backend::kcmInit();
}
