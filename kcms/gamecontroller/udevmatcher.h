/*
    SPDX-FileCopyrightText: 2025 Jeremy Whiting <jpwhiting@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <string>

struct udev;

class UDevMatcher
{
public:
    static UDevMatcher *instance();
    ~UDevMatcher();

    // Given a device path, get it's udev device id for matching
    std::string deviceToUdevId(const std::string &device);

private:
    UDevMatcher();

    udev *m_udev;
    static UDevMatcher *m_instance;
};
