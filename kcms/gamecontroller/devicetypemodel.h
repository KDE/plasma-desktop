/*
    SPDX-FileCopyrightText: 2023 Jeremy Whiting <jpwhiting@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QList>
#include <QStandardItemModel>

class DeviceTypeModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum CustomRoles {
        NameRole = Qt::UserRole + 1, // Name shown in the gui for selection
        TypeRole, // Which gamepadtype qml file to load
    };

    enum DeviceType { Unknown, Xbox, DualSense };
    Q_ENUM(DeviceType)

    DeviceTypeModel();

    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE int rowFromType(int type);
    Q_INVOKABLE DeviceTypeModel::DeviceType typeFromRow(int row);
    // Get the svg path for a given device type row
    Q_INVOKABLE QString pathFromRow(int row);

private:
    void addType(int type, const QString &guiName, const DeviceType &deviceType, const QString &svgPath);

    void addDefaultType(int type, int closestMatch);

    // Map of SDL type enum, to gamepad name our DeviceType, and svgPath
    QMap<int, std::tuple<QString, DeviceType, QString>> m_types;

    // List of SDL controller type enumeration in order of insertion
    QList<int> m_typeOrder;

    // Map of SDL type enum to current closest match. Will go away after all sdl types have
    // proper qml representations, etc.
    QMap<int, int> m_defaultTypes;
};
