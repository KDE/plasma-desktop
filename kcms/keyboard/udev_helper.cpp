/*
 *  Copyright (C) 2015 David Rosca (nowrep@gmail.com)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "udev_helper.h"
#include "debug.h"

#include <QSocketNotifier>

#include <config-keyboard.h>

#ifdef HAVE_UDEV
#include <libudev.h>
#endif

// Based on qtbase: src/platformsupport/devicediscovery/qdevicediscovery_udev.cpp

UdevDeviceNotifier::UdevDeviceNotifier(QObject *parent)
    : QObject(parent)
    , m_udev(nullptr)
    , m_monitor(nullptr)
{
    init();
}

#ifdef HAVE_UDEV
UdevDeviceNotifier::~UdevDeviceNotifier()
{
    if (m_monitor) {
        udev_monitor_unref(m_monitor);
    }
    if (m_udev) {
        udev_unref(m_udev);
    }
}

void UdevDeviceNotifier::init()
{
    m_udev = udev_new();
    if (!m_udev) {
        return;
    }

    m_monitor = udev_monitor_new_from_netlink(m_udev, "udev");
    if (!m_monitor) {
        return;
    }

    udev_monitor_filter_add_match_subsystem_devtype(m_monitor, "input", 0);
    udev_monitor_enable_receiving(m_monitor);

    int monitor_fd = udev_monitor_get_fd(m_monitor);
    QSocketNotifier *notifier = new QSocketNotifier(monitor_fd, QSocketNotifier::Read, this);
    connect(notifier, &QSocketNotifier::activated, this, &UdevDeviceNotifier::socketActivated);
}

void UdevDeviceNotifier::socketActivated()
{
    static const char *keyboardDevice[] = {
        "ID_INPUT_KEYBOARD",
        "ID_INPUT_KEY"
    };

    static const char *pointerDevice[] = {
        "ID_INPUT_MOUSE",
        "ID_INPUT_TOUCHPAD",
        "ID_INPUT_TABLET"
    };

    struct udev_device *dev = udev_monitor_receive_device(m_monitor);
    if (!dev) {
        return;
    }

    const char *action = udev_device_get_action(dev);
    if (!action || qstrcmp(action, "add") != 0) {
        udev_device_unref(dev);
        return;
    }

    // Skip devices with empty name
    if (!udev_device_get_property_value(dev, "NAME")) {
        udev_device_unref(dev);
        return;
    }

    for (unsigned i = 0; i < sizeof(keyboardDevice) / sizeof(keyboardDevice[0]); ++i) {
        if (qstrcmp(udev_device_get_property_value(dev, keyboardDevice[i]), "1") == 0) {
            qCDebug(KCM_KEYBOARD) << "New Udev keyboard device";
            Q_EMIT newKeyboardDevice();
            break;
        }
    }

    for (unsigned i = 0; i < sizeof(pointerDevice) / sizeof(pointerDevice[0]); ++i) {
        if (qstrcmp(udev_device_get_property_value(dev, pointerDevice[i]), "1") == 0) {
            qCDebug(KCM_KEYBOARD) << "New Udev pointer device";
            Q_EMIT newPointerDevice();
            break;
        }
    }

    udev_device_unref(dev);
}

#else
UdevDeviceNotifier::~UdevDeviceNotifier()
{
}

void UdevDeviceNotifier::init()
{
}

void UdevDeviceNotifier::socketActivated()
{
}
#endif

