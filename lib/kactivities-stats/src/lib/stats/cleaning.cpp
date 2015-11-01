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
#include <QDebug>

#include "cleaning.h"
#include "common/dbus/common.h"

namespace KActivities {
namespace Experimental {
namespace Stats {


void forgetResource(Terms::Activity activities, Terms::Agent agents,
                    const QString &resource)
{
    KAMD_DECL_DBUS_INTERFACE(scoring, Resources/Scoring, ResourcesScoring);
    for (const auto& activity: activities.values) {
        for (const auto& agent: agents.values) {
            scoring.call(QStringLiteral("DeleteStatsForResource"), activity, agent, resource);
        }
    }
}

void forgetResources(const Query &query)
{
    KAMD_DECL_DBUS_INTERFACE(scoring, Resources/Scoring, ResourcesScoring);
    for (const auto& activity: query.activities()) {
        for (const auto& agent: query.agents()) {
            for (const auto& urlFilter: query.urlFilters()) {
                scoring.call(QStringLiteral("DeleteStatsForResource"), activity, agent, urlFilter);
            }
        }
    }
}

void forgetRecentStats(Terms::Activity activities, int count, TimeUnit what)
{
    KAMD_DECL_DBUS_INTERFACE(scoring, Resources/Scoring, ResourcesScoring);
    for (const auto& activity: activities.values) {
        scoring.call(QStringLiteral("DeleteRecentStats"), activity, count,
                what == Hours  ? "h" :
                what == Days   ? "d" :
                                 "m"
            );
    }
}

void forgetEarlierStats(Terms::Activity activities, int months)
{
    KAMD_DECL_DBUS_INTERFACE(scoring, Resources/Scoring, ResourcesScoring);
    for (const auto& activity: activities.values) {
        scoring.call(QStringLiteral("DeleteEarlierStats"), activity, months);
    }
}

} // namespace Stats
} // namespace Experimental
} // namespace KActivities

