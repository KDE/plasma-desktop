/*
    Copyright 2019  Nicolas Fella <nicolas.fella@gmx.de>
    Copyright 2020  Carson Black <uhhadd@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) version 3, or any
    later version accepted by the membership of KDE e.V. (or its
    successor approved by the membership of KDE e.V.), which shall
    act as a proxy defined in Section 6 of version 3 of the license.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include <QAbstractListModel>

#include "user.h"

class OrgFreedesktopAccountsInterface;

class UserModel
    : public QAbstractListModel
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

    explicit UserModel(QObject* parent = nullptr);
    ~UserModel() override;

    QVariant data(const QModelIndex& index, int role) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;

    Q_INVOKABLE User* getLoggedInUser() const;

    QHash<int, QByteArray> roleNames() const override;

private:
    OrgFreedesktopAccountsInterface* m_dbusInterface;
    QVector<User *> m_userList;
};
