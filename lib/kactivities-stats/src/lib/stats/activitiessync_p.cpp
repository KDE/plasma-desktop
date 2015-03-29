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

#include "activitiessync_p.h"

#include <QCoreApplication>

namespace ActivitiesSync {
    typedef std::shared_ptr<KActivities::Consumer> ConsumerPtr;

    ConsumerPtr instance()
    {
        static std::mutex s_instanceMutex;
        static std::weak_ptr<KActivities::Consumer> s_instance;

        std::unique_lock<std::mutex> locker;

        auto ptr = s_instance.lock();

        if (!ptr) {
            ptr = std::make_shared<KActivities::Consumer>();
            s_instance = ptr;
        }

        return ptr;

    }

    QString currentActivity(ConsumerPtr &activities)
    {
        // We need to get the current activity synchonously,
        // this means waiting for the service to be available.
        // It should not introduce blockages since there usually
        // is a global activity cache in applications that care
        // about activities.

        if (!activities) {
            activities = instance();
        }

        while (activities->serviceStatus() == KActivities::Consumer::Unknown) {
            QCoreApplication::instance()->processEvents();
        }

        return activities->currentActivity();
    }

} // namespace ActivitiesSync

