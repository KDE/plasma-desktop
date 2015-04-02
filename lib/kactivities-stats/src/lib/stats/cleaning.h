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

#ifndef CLEANING_H
#define CLEANING_H

#include <QString>

namespace KActivities {
namespace Experimental {
namespace Stats {

/**
 * Forget the resource(s) for the specified activity and agent
 */
void forgetResource(const QString &activity, const QString &agent,
                    const QString &resource);

enum TimeUnit {
    Hours,
    Days,
    Months
};

/**
 * Forget recent stats for the specified activity and time
 */
void forgetRecentStats(const QString &activity, int count, TimeUnit what);

/**
 * Forget events that are older than the specified number of months
 */
void forgetEarlierStats(const QString &activity, int months);

} // namespace Stats
} // namespace Experimental
} // namespace KActivities

#endif // CLEANING_H

