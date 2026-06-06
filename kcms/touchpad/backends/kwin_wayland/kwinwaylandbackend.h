/*
    SPDX-FileCopyrightText: 2017 Roman Gilg <subdiff@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "kwinwaylandtouchpad.h"
#include "touchpadbackend.h"

#include <QList>

namespace KWinDevices
{
class DevicesModel;
}

class KWinWaylandBackend : public TouchpadBackend
{
    Q_OBJECT

public:
    explicit KWinWaylandBackend(QObject *parent = nullptr);
    ~KWinWaylandBackend();

    bool save() override;
    bool load() override;
    bool defaults() override;
    bool isSaveNeeded() const override;

    QString errorString() const override
    {
        return m_errorString;
    }

    int deviceCount() const override
    {
        return m_devices.count();
    }
    QList<KWinWaylandTouchpad *> inputDevices() const override
    {
        return m_devices;
    }

private:
    void findTouchpads();

    void onDeviceRowsInserted(const QModelIndex &parent, int first, int last);
    void onDeviceRowsAboutToBeRemoved(const QModelIndex &parent, int first, int last);

    KWinDevices::DevicesModel *m_devicesModel;
    QList<KWinWaylandTouchpad *> m_devices;

    QString m_errorString = QString();
};
