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

#include "org.kde.ActivityManager.Activities.h"

#include <QMetaType>
#include <QDBusMetaType>

namespace details {

class ActivityInfoStaticInit {
public:
    ActivityInfoStaticInit()
    {
        qDBusRegisterMetaType<ActivityInfo>();
        qDBusRegisterMetaType<ActivityInfoList>();
    }

    static ActivityInfoStaticInit _instance;
};

ActivityInfoStaticInit ActivityInfoStaticInit::_instance;

} // namespace details

QDBusArgument &operator<<(QDBusArgument &arg, const ActivityInfo r)
{
    arg.beginStructure();

    arg << r.id;
    arg << r.name;
    arg << r.description;
    arg << r.icon;
    arg << r.state;

    arg.endStructure();

    return arg;
}

const QDBusArgument &operator>>(const QDBusArgument &arg, ActivityInfo &r)
{
    arg.beginStructure();

    arg >> r.id;
    arg >> r.name;
    arg >> r.description;
    arg >> r.icon;
    arg >> r.state;

    arg.endStructure();

    return arg;
}

QDebug operator<<(QDebug dbg, const ActivityInfo &r)
{
    dbg << "ActivityInfo(" << r.id << r.name << ")";
    return dbg.space();
}
