/*
    SPDX-FileCopyrightText: 2018 Roman Gilg <subdiff@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "x11_libinput_backend.h"
#include "x11_libinput_dummydevice.h"

X11LibinputBackend::X11LibinputBackend(QObject *parent)
    : InputBackend(parent)
{
    m_device = new X11LibinputDummyDevice(this, QX11Info::display());
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
    static_cast<X11LibinputDummyDevice *>(m_device)->getDefaultConfigFromX();

    getConfig();
    applyConfig();
}

QString X11LibinputBackend::errorString() const
{
    return m_errorString;
}

int X11LibinputBackend::deviceCount() const
{
    return 1;
}

bool X11LibinputBackend::isAnonymousDevice() const
{
    return true;
}

QList<QObject *> X11LibinputBackend::getDevices() const
{
    return QList<QObject *>(1, m_device);
}

#include "moc_x11_libinput_backend.cpp"
