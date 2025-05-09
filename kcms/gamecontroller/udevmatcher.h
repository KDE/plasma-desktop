/*
    SPDX-FileCopyrightText: 2025 Jeremy Whiting <jpwhiting@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <memory>
#include <string>

class UDevMatcher
{
public:
    // Given a device path, get it's udev device id for matching
    static std::string deviceToUdevId(const std::string &device);
};
