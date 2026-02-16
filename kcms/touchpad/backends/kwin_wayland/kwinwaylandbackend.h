/*
    SPDX-FileCopyrightText: 2017 Roman Gilg <subdiff@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

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
    QList<LibinputCommon *> inputDevices() const override
    {
        return m_devices;
    }

private:
    void findTouchpads();

    void onDevicesInserted(const QModelIndex &parent, int first, int last);
    void onDevicesAboutToBeRemoved(const QModelIndex &parent, int first, int last);

    KWinDevices::DevicesModel *m_devicesModel;
    QList<LibinputCommon *> m_devices;

    QString m_errorString = QString();
};
