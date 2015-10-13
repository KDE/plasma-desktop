/*
 *   Copyright (C) 2010, 2011, 2012 Ivan Cukic <ivan.cukic(at)kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License version 2,
 *   or (at your option) any later version, as published by the Free
 *   Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef KAMD_ACTIVITIES_DBUS_H
#define KAMD_ACTIVITIES_DBUS_H

#include <QString>
#include <QList>
#include <QDBusArgument>
#include <QDebug>

struct ActivityInfo {
    QString id;
    QString name;
    QString description;
    QString icon;
    int state;

    ActivityInfo(const QString &id = QString(),
                 const QString &name = QString(),
                 const QString &description = QString(),
                 const QString &icon = QString(),
                 int state = 0)
        : id(id)
        , name(name)
        , description(description)
        , icon(icon)
        , state(state)
    {
    }

    bool operator<(const ActivityInfo &other) const
    {
        return id < other.id;
    }

    bool operator==(const ActivityInfo &other) const
    {
        return id == other.id;
    }
};

typedef QList<ActivityInfo> ActivityInfoList;

Q_DECLARE_METATYPE(ActivityInfo)
Q_DECLARE_METATYPE(ActivityInfoList)

QDBusArgument &operator<<(QDBusArgument &arg, const ActivityInfo);
const QDBusArgument &operator>>(const QDBusArgument &arg, ActivityInfo &rec);

QDebug operator<<(QDebug dbg, const ActivityInfo &r);

#endif // KAMD_ACTIVITIES_DBUS_H
