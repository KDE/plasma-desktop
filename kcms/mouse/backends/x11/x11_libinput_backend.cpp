/*
    SPDX-FileCopyrightText: 2018 Roman Gilg <subdiff@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "x11_libinput_backend.h"

X11LibinputBackend::X11LibinputBackend(QObject *parent)
    : InputBackend(parent)
    , m_device(new X11LibinputDummyDevice(this, QX11Info::display()))
{
    connect(m_device.get(), &X11LibinputDummyDevice::needsSaveChanged, this, &InputBackend::needsSaveChanged);
}

bool X11LibinputBackend::save()
{
    return m_device->save();
}

bool X11LibinputBackend::load()
{
    return m_device->load();
}

bool X11LibinputBackend::defaults()
{
    return m_device->defaults();
}

bool X11LibinputBackend::isSaveNeeded() const
{
    return m_device->isSaveNeeded();
}

void X11LibinputBackend::kcmInit()
{
    m_device->defaultsFromX();

    load();
    save();
}

QString X11LibinputBackend::errorString() const
{
    return m_errorString;
}

int X11LibinputBackend::deviceCount() const
{
    return 1;
}

bool X11LibinputBackend::isAnonymousInputDevice() const
{
    return true;
}

QList<InputDevice *> X11LibinputBackend::inputDevices() const
{
    return {m_device.get()};
}

#include "moc_x11_libinput_backend.cpp"
