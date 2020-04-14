/*
 * Copyright (C) 2013 Alexander Mezin <mezin.alexander@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#include "touchpadservice.h"

#include <Plasma/ServiceJob>

#include "touchpadinterface.h"

TouchpadService::TouchpadService(OrgKdeTouchpadInterface *daemon,
                                 const QString &destination,
                                 QObject *parent)
    : Plasma::Service(parent), m_daemon(daemon), m_destination(destination)
{
    setName("touchpad");
}

TouchpadService::~TouchpadService()
{
}

class TouchpadJob : public Plasma::ServiceJob
{
public:
    TouchpadJob(OrgKdeTouchpadInterface *daemon,
                const QString &destination, const QString &operation,
                const QMap<QString, QVariant> &parameters, QObject *parent = nullptr)
        : Plasma::ServiceJob(destination, operation, parameters, parent),
          m_daemon(daemon)
    {
    }

    void start() override
    {
        if (m_daemon) {
            QMetaObject::invokeMethod(m_daemon, operationName().toLatin1());
        }
        emitResult();
    }

private:
    OrgKdeTouchpadInterface *m_daemon;
};

Plasma::ServiceJob *TouchpadService::createJob(const QString &operation,
                                               QMap<QString, QVariant> &params)
{
    return new TouchpadJob(m_daemon, m_destination, operation, params, this);
}
