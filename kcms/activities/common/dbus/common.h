/*
 *   Copyright (C) 2010 - 2016 by Ivan Cukic <ivan.cukic@kde.org>
 *
 *   This program is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU General Public License as
 *   published by the Free Software Foundation; either version 2 of
 *   the License or (at your option) version 3 or any later version
 *   accepted by the membership of KDE e.V. (or its successor approved
 *   by the membership of KDE e.V.), which shall act as a proxy
 *   defined in Section 14 of version 3 of the license.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef DBUS_COMMON_H
#define DBUS_COMMON_H

#include <QDBusConnection>
#include <QDBusInterface>

#define KAMD_DBUS_SERVICE                                                      \
    QStringLiteral("org.kde.ActivityManager")

#define KAMD_DBUS_OBJECT_PATH(A)                                               \
    (sizeof(#A) > 2 ? QLatin1String("/ActivityManager/" #A)                    \
                    : QLatin1String("/ActivityManager"))

#define KAMD_DBUS_OBJECT(A)                                                    \
    QLatin1String("org.kde.ActivityManager." #A)

#define KAMD_DBUS_INTERFACE(OBJECT_PATH, OBJECT, PARENT)                       \
    QDBusInterface(KAMD_DBUS_SERVICE,                                          \
                   KAMD_DBUS_OBJECT_PATH(OBJECT_PATH),                         \
                   KAMD_DBUS_OBJECT(OBJECT),                                   \
                   QDBusConnection::sessionBus(),                              \
                   PARENT)

#define KAMD_DBUS_DECL_INTERFACE(VAR, OBJECT_PATH, OBJECT)                     \
    QDBusInterface VAR(KAMD_DBUS_SERVICE,                                      \
                   KAMD_DBUS_OBJECT_PATH(OBJECT_PATH),                         \
                   KAMD_DBUS_OBJECT(OBJECT),                                   \
                   QDBusConnection::sessionBus(),                              \
                   nullptr)

#define KAMD_DBUS_CLASS_INTERFACE(OBJECT_PATH, OBJECT, PARENT)                 \
    org::kde::ActivityManager::OBJECT(                                         \
                KAMD_DBUS_SERVICE,                                             \
                KAMD_DBUS_OBJECT_PATH(OBJECT_PATH),                            \
                QDBusConnection::sessionBus(),                                 \
                PARENT)

#endif // DBUS_COMMON_H

