/*
    SPDX-FileCopyrightText: 2020-2025 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <KQuickManagedConfigModule>
#include <KService>
#include <QAbstractListModel>

class KDesktopFile;
class VirtualKeyboardData;
class VirtualKeyboardSettings;

class VirtualKeyboardsModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON
public:
    enum Roles {
        DesktopFileNameRole = Qt::UserRole + 1,
    };
    Q_ENUM(Roles)

    VirtualKeyboardsModel(QObject *parent = nullptr);
    QVariant data(const QModelIndex &index, int role) const override;
    int rowCount(const QModelIndex &parent) const override;

    QHash<int, QByteArray> roleNames() const override;

    Q_SCRIPTABLE int inputMethodIndex(const QString &desktopFile) const;

private:
    KService::List m_services;
};
