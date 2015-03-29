/*
 *   Copyright (C) 2010, 2011, 2012, 2013, 2014 Ivan Cukic <ivan.cukic(at)kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Lesser General Public License version 2,
 *   or (at your option) any later version, as published by the Free
 *   Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Lesser General Public License for more details
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef DBUS_COMMON_H
#define DBUS_COMMON_H

#include <QDBusConnection>
#include <QDBusInterface>

#define KAMD_DBUS_SERVICE                                                      \
    QStringLiteral("org.kde.ActivityManager")

#define KAMD_DBUS_OBJECT_PATH(A)                                               \
    QLatin1String("/ActivityManager/" #A)

#define KAMD_DBUS_OBJECT(A)                                                    \
    QLatin1String("org.kde.ActivityManager." #A)

#define KAMD_DBUS_INTERFACE(OBJECT_PATH, OBJECT, PARENT)                       \
    QDBusInterface(KAMD_DBUS_SERVICE,                                          \
                   KAMD_DBUS_OBJECT_PATH(OBJECT_PATH),                         \
                   KAMD_DBUS_OBJECT(OBJECT),                                   \
                   QDBusConnection::sessionBus(),                              \
                   PARENT)

#define KAMD_DECL_DBUS_INTERFACE(VAR, OBJECT_PATH, OBJECT)                     \
    QDBusInterface VAR(KAMD_DBUS_SERVICE,                                      \
                   KAMD_DBUS_OBJECT_PATH(OBJECT_PATH),                         \
                   KAMD_DBUS_OBJECT(OBJECT),                                   \
                   QDBusConnection::sessionBus(),                              \
                   Q_NULLPTR)

#define KAMD_DBUS_CLASS_INTERFACE(OBJECT_PATH, OBJECT, PARENT)                 \
    org::kde::ActivityManager::OBJECT(                                         \
                KAMD_DBUS_SERVICE,                                             \
                KAMD_DBUS_OBJECT_PATH(OBJECT_PATH),                            \
                QDBusConnection::sessionBus(),                                 \
                PARENT)

#endif // DBUS_COMMON_H

