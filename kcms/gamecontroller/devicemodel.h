/*
    SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
    SPDX-FileCopyrightText: 2023 Jeremy Whiting <jpwhiting@kde.org>
    SPDX-FileCopyrightText: 2023 Niccolò Venerandi <niccolo@venerandi.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QList>
#include <QPointer>
#include <QStandardItemModel>

class QTimer;
class Gamepad;

class DeviceModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY devicesChanged)

public:
    DeviceModel();
    virtual ~DeviceModel();

    Q_INVOKABLE Gamepad *device(int index) const;

    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;

    int count() const;

Q_SIGNALS:
    void devicesChanged();

private Q_SLOTS:
    void poll();

private:
    void addDevice(const int deviceIndex);
    void removeDevice(const int deviceIndex);

    // Map of sdl indexes to Gamepad devices
    QMap<int, Gamepad *> m_devices;
    QPointer<QTimer> m_timer;
};
