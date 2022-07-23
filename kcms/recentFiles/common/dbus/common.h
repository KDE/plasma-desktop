/*
    SPDX-FileCopyrightText: 2010-2016 Ivan Cukic <ivan.cukic@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

// clang-format off

#ifndef DBUS_COMMON_H
#define DBUS_COMMON_H

#define KAMD_DBUS_SERVICE "org.kde.ActivityManager"

#define KAMD_DBUS_ACTIVITYMANAGER_PATH "/ActivityManager"

#define KAMD_DBUS_FEATURES_PATH KAMD_DBUS_ACTIVITYMANAGER_PATH"/Features"

#define KAMD_DBUS_RESOURCES_PATH KAMD_DBUS_ACTIVITYMANAGER_PATH"/Resources"
#define KAMD_DBUS_RESOURCES_SCORING_PATH KAMD_DBUS_RESOURCES_PATH"/Scoring"

#endif // DBUS_COMMON_H
