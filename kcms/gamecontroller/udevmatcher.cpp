/*
    SPDX-FileCopyrightText: 2025 Jeremy Whiting <jpwhiting@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "udevmatcher.h"

#include <cstring>
#include <libudev.h>

#include "logging.h"

static std::string kDevicePathKey = "DEVPATH";

UDevMatcher *UDevMatcher::m_instance;

UDevMatcher *UDevMatcher::instance()
{
    if (!m_instance) {
        m_instance = new UDevMatcher();
    }

    return m_instance;
}

UDevMatcher::UDevMatcher()
    : m_udev(udev_new())
{
}

UDevMatcher::~UDevMatcher()
{
    udev_unref(m_udev);
    m_udev = nullptr;
}

// This function has one purpose, to translate udev devpaths into an id
// to get from something like /devices/virtual/misc/uhid/0005:18D1:9400.0005/hidraw/hidraw2
// to an id such as 0005:18D1:9400.005 for matching against the ids from hidapi.
std::string UDevMatcher::deviceToUdevId(const std::string &device_path)
{
    std::string devicePath;
    char sys_path[256];

    size_t lastSlash = device_path.find_last_of('/');

    std::string lastPart = device_path.substr(lastSlash + 1);

    if (lastPart.starts_with("hidraw")) {
        // hidrdaw devpaths end with hidrawX
        snprintf(sys_path, sizeof(sys_path), "/sys/class/hidraw/%s", lastPart.c_str());
    } else if (lastPart.starts_with("event")) {
        // input devpaths end with eventXY
        snprintf(sys_path, sizeof(sys_path), "/sys/class/input/%s", lastPart.c_str());
    }
    qCDebug(KCM_GAMECONTROLLER) << "Using sys path: " << sys_path;
    struct udev_device *udevice = udev_device_new_from_syspath(m_udev, sys_path);

    if (udevice) {
        devicePath = udev_device_get_property_value(udevice, kDevicePathKey.c_str());
    }

    // Walk the devpath looking for the ID
    size_t slash = devicePath.find_last_of('/');

    qCDebug(KCM_GAMECONTROLLER) << "Looking for ID of " << devicePath;

    // If the bit after the last / is a digit, stop, we found it
    while (!std::isdigit(devicePath.at(slash + 1))) {
        // Remove the last bit
        devicePath = devicePath.substr(0, slash);

        slash = devicePath.find_last_of('/');

        qCDebug(KCM_GAMECONTROLLER) << "Last part isn't an id, moving up: " << devicePath;
    }

    // Now remove the prefix
    slash = devicePath.find_last_of('/');
    devicePath = devicePath.substr(slash + 1);

    return devicePath;
}
