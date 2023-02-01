/*
    SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "joydevice.h"

#include <QDebug>
#include <QFile>
#include <QSocketNotifier>
#include <Solid/Block>
#include <Solid/GenericInterface>
#include <csignal>
#include <fcntl.h>

JoyDevice::JoyDevice(const Solid::Device &device, QObject *parent)
    : QObject(parent)
{
    auto inputDevice = device.as<Solid::Block>();

    m_name = device.vendor() + " " + device.product();

    auto fd = open(QFile::encodeName(inputDevice->device()), O_RDONLY | O_NONBLOCK);
    if (fd < 0) {
        qDebug() << "Failed to open fd!";
        return;
    }

    int rc = libevdev_new_from_fd(fd, &m_device);
    if (rc < 0) {
        qDebug() << "Failed to open evdev stream!";
        return;
    }

    for (int code = BTN_A; code < BTN_A + 18; code++) {
        if (libevdev_has_event_code(m_device, EV_KEY, code))
            m_numButtons++;
    }

    if (libevdev_has_event_code(m_device, EV_ABS, ABS_X)) {
        m_numAxes++;
    }

    if (libevdev_has_event_code(m_device, EV_ABS, ABS_RX)) {
        m_numAxes++;
    }

    m_hasRumble = libevdev_has_event_code(m_device, EV_FF, FF_RUMBLE);

    m_buttonState.resize(m_numButtons);
    m_axisState.resize(m_numAxes);

    auto notifier = new QSocketNotifier(fd, QSocketNotifier::Read, this);
    connect(notifier, &QSocketNotifier::activated, this, &JoyDevice::poll);
}

void JoyDevice::poll()
{
    const int fd = libevdev_get_fd(m_device);
    struct input_event ev;
    int ret = read(fd, &ev, sizeof(ev));
    if (ret == 0) {
        qDebug() << "nothing to read";
    } else if (ret < 0) {
        qWarning() << "Error while reading" << strerror(errno);
    } else {
        processEvent(ev);
    }

    uint bytes;
    ret = ::ioctl(fd, FIONREAD, &bytes);
    if (ret == 0 && bytes >= sizeof(ev))
        poll();
}

float JoyDevice::normalize(int code, __s32 value)
{
    // TODO: cache this
    auto info = libevdev_get_abs_info(m_device, code);

    __s32 adjustedValue = value - info->value;

    return adjustedValue < 0 ? -static_cast<float>(adjustedValue) / info->minimum : static_cast<float>(adjustedValue) / info->maximum;
}

void JoyDevice::processEvent(struct input_event &ev)
{
    if (ev.type == EV_KEY) {
        const int index = ev.code - BTN_SOUTH;
        if (index >= 0 && index < m_buttonState.size()) {
            m_buttonState[ev.code - BTN_SOUTH] = ev.value;

            emit buttonStateChanged();
        }
    } else if (ev.type == EV_ABS) {
        int axisIndex = ev.code != ABS_X && ev.code != ABS_Y;

        if (ev.code == ABS_Y || ev.code == ABS_RY) {
            m_axisState[axisIndex].setY(normalize(ev.code, ev.value));
        } else {
            m_axisState[axisIndex].setX(normalize(ev.code, ev.value));
        }

        emit axisStateChanged();
    }
}
