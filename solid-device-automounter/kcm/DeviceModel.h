/*
    SPDX-FileCopyrightText: 2009-2010 Trever Fischer <tdfischer@fedoraproject.org>
    SPDX-FileCopyrightText: 2015 Kai Uwe Broulik <kde@privat.broulik.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QAbstractItemModel>
#include <QHash>
#include <QList>
#include <QModelIndex>
#include <QVariant>

class AutomounterSettings;

class DeviceModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit DeviceModel(AutomounterSettings *m_settings, QObject *parent = nullptr);
    ~DeviceModel() override = default;

    enum DeviceType {
        Attached,
        Detached,
    };

    enum {
        KeyRole = Qt::UserRole,
        TypeRole,
    };

    enum {
        RowAll,
        RowAttached,
        RowDetached,
    };

    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;

    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    void setAutomaticMountOnLogin(bool automaticLogin);
    void setAutomaticMountOnPlugin(bool automaticAttached);
    void setAutomaticUnknown(bool automaticUnknown);

    void updateCheckedColumns(int column = -1);

public Q_SLOTS:
    void forgetDevice(const QString &key);
    void reload();

private Q_SLOTS:
    void deviceAttached(const QString &udi);
    void deviceRemoved(const QString &udi);

private:
    void addNewDevice(const QString &key);

    QList<QString> m_attached;
    QList<QString> m_disconnected;
    // Maps the key of an attached device to its UDI
    QHash<QString, QString> m_attachedUdis;
    AutomounterSettings *const m_settings;
};
