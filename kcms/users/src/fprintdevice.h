/*
    Copyright 2020  Devin Lin <espidev@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) version 3, or any
    later version accepted by the membership of KDE e.V. (or its
    successor approved by the membership of KDE e.V.), which shall
    act as a proxy defined in Section 6 of version 3 of the license.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include "fprint_device_interface.h"
#include "fprint_manager_interface.h"

class FprintDevice : public QObject
{
    Q_OBJECT

public:
    explicit FprintDevice(QDBusObjectPath path, QObject* parent = nullptr);

    QDBusPendingReply<QStringList> listEnrolledFingers(const QString &username);
    
    QDBusError claim(const QString &username);
    QDBusError release();
    
    QDBusError deleteEnrolledFingers();
    QDBusError deleteEnrolledFinger(QString &finger);
    QDBusError startEnrolling(const QString &finger);
    QDBusError stopEnrolling();

    int numOfEnrollStages();
    QString scanType();
    bool fingerPresent();
    bool fingerNeeded();

public Q_SLOTS:
    void enrollStatus(QString result, bool done);

Q_SIGNALS:
    void enrollCompleted();
    void enrollStagePassed();
    void enrollRetryStage(QString feedback);
    void enrollFailed(QString error);
    
private:
    QString m_devicePath;
    QString m_username;
    NetReactivatedFprintDeviceInterface *m_fprintInterface;
    QDBusInterface *m_freedesktopInterface;
};

