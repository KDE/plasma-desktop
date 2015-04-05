/*
 *   Copyright (C) 2015 Ivan Cukic <ivan.cukic(at)kde.org>
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


#include <QString>
#include <QDBusReply>

#include "cleaning.h"
#include "common/dbus/common.h"

namespace KActivities {
namespace Experimental {
namespace Stats {


void forgetResource(const QString &activity, const QString &agent,
                    const QString &resource)
{
    KAMD_DECL_DBUS_INTERFACE(scoring, Resources/Scoring, ResourcesScoring);
    scoring.call("DeleteStatsForResource", activity, agent, resource);
}

void forgetRecentStats(const QString &activity, int count, TimeUnit what)
{
    KAMD_DECL_DBUS_INTERFACE(scoring, Resources/Scoring, ResourcesScoring);
    scoring.call("DeleteRecentStats", activity, count,
            what == Hours  ? "h" :
            what == Days   ? "d" :
                             "m"
        );
}

void forgetEarlierStats(const QString &activity, int months)
{
    KAMD_DECL_DBUS_INTERFACE(scoring, Resources/Scoring, ResourcesScoring);
    scoring.call("DeleteEarlierStats", activity, months);
}

} // namespace Stats
} // namespace Experimental
} // namespace KActivities

