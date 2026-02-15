/*
    SPDX-FileCopyrightText: 2024 Nicolas Fella <nicolas.fella@gmx.de>
    SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QAbstractListModel>

#include <memory>
#include <vector>

#include "inputdevice.h"

extern "C" {
#include <libwacom/libwacom.h>
}

struct TabletDevice {
    QString deviceGroup;
    KWinDevices::InputDevice *padDevice = nullptr;
    KWinDevices::InputDevice *penDevice = nullptr;
};

class TabletsModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit TabletsModel(WacomDeviceDatabase *db, QObject *parent = nullptr);

    QHash<int, QByteArray> roleNames() const override;
    QVariant data(const QModelIndex &index, int role) const override;
    int rowCount(const QModelIndex &parent) const override;

    void load();
    void save();
    void defaults();
    bool isSaveNeeded() const;
    bool isDefaults() const;

public Q_SLOTS:
    KWinDevices::InputDevice *penAt(int row) const;
    KWinDevices::InputDevice *padAt(int row) const;

private Q_SLOTS:
    void onDeviceAdded(const QString &sysName);
    void onDeviceRemoved(const QString &sysName);
    void loadReply(QDBusMessage reply);

Q_SIGNALS:
    void needsSaveChanged();
    void dbChanged();
    void deviceRemoved(const QString &sysName);
    void deviceChanged(int row);

private:
    void addDevice(const QString &sysname, bool tellModel);
    void resetModel();

    std::vector<TabletDevice> m_devices;
    WacomDeviceDatabase *m_db = nullptr;
};
