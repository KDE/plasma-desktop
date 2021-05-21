/*
    SPDX-FileCopyrightText: 2019 Nicolas Fella <nicolas.fella@gmx.de>
    SPDX-FileCopyrightText: 2020 Carson Black <uhhadd@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#pragma once

#include <QAbstractListModel>

#include "user.h"

class OrgFreedesktopAccountsInterface;

class UserModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum ModelRoles {
        RealNameRole = Qt::DisplayRole,
        FaceRole = Qt::DecorationRole,
        UidRole = Qt::UserRole,
        NameRole,
        EmailRole,
        FaceValidRole,
        AdministratorRole,
        UserRole,
        LoggedInRole,
        SectionHeaderRole,
    };

    explicit UserModel(QObject *parent = nullptr);
    ~UserModel() override;

    QVariant data(const QModelIndex &index, int role) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    Q_INVOKABLE User *getLoggedInUser() const;

    QHash<int, QByteArray> roleNames() const override;

private:
    OrgFreedesktopAccountsInterface *m_dbusInterface;
    QVector<User *> m_userList;
};
