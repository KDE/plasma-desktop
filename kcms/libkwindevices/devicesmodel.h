/*
    SPDX-FileCopyrightText: 2021 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QAbstractListModel>

#include <memory>

#include <vector>

namespace KWinDevices
{
class InputDevice;

class DevicesModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Roles {
        DeviceRole = Qt::UserRole,
        SysNameRole,
    };
    Q_ENUM(Roles)

    enum class Kind {
        Pointers = 1,
        Keyboards,
        Touch,
    };
    Q_ENUM(Kind)

    explicit DevicesModel(Kind kind, QObject *parent = nullptr);
    explicit DevicesModel(Kind kind, const QVariantMap &filters, QObject *parent = nullptr);

    QHash<int, QByteArray> roleNames() const override;
    QVariant data(const QModelIndex &index, int role) const override;
    int rowCount(const QModelIndex &parent) const override;
    Q_SCRIPTABLE InputDevice *deviceAt(int row) const;

    void load();
    void save();
    void defaults();
    bool isSaveNeeded() const;
    bool isDefaults() const;
    QString errorString() const;

private Q_SLOTS:
    void onDeviceAdded(const QString &sysName);
    void onDeviceRemoved(const QString &sysName);

Q_SIGNALS:
    void needsSaveChanged();
    void deviceRemoved(const QString &sysName);

private:
    void addDevice(const QString &sysname);

    const Kind m_kind;
    QVariantMap m_filters;
    std::vector<std::unique_ptr<KWinDevices::InputDevice>> m_devices;
    // So it doesn't have to look them up on the device via DBus.
    QStringList m_deviceSysNames;

    QString m_errorString;
};

} // namespace KWinDevices
