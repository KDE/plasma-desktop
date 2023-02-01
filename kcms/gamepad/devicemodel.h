/*
    SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QList>
#include <QStandardItemModel>
#include <Solid/Device>

class JoyDevice;

class DeviceModel : public QAbstractListModel
{
public:
    enum CustomRoles { NameRole = Qt::UserRole + 1, DeviceRole };

    DeviceModel();

    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

private:
    void addDevice(const Solid::Device &device);

    QList<JoyDevice *> m_devices;
};