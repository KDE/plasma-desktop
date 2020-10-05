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

#include "usermodel.h"

#include <QDBusPendingReply>
#include <QDBusInterface>
#include <algorithm>
#include <KLocalizedString>

#include "accounts_interface.h"
#include "kcmusers_debug.h"

UserModel::UserModel(QObject* parent)
    : QAbstractListModel(parent)
    , m_dbusInterface(new OrgFreedesktopAccountsInterface(QStringLiteral("org.freedesktop.Accounts"), QStringLiteral("/org/freedesktop/Accounts"), QDBusConnection::systemBus(), this))
{
    connect(m_dbusInterface, &OrgFreedesktopAccountsInterface::UserAdded, this, [this](const QDBusObjectPath &path) {
        User *user = new User(this);
        user->setPath(path);
        beginInsertRows(QModelIndex(), m_userList.size(), m_userList.size());
        m_userList.append(user);
        endInsertRows();
    });

    connect(m_dbusInterface, &OrgFreedesktopAccountsInterface::UserDeleted, this, [this](const QDBusObjectPath &path) {
        QList<User*> toRemove;
        for (int i = 0; i < m_userList.length(); i++) {
            if (m_userList[i]->path().path() == path.path()) {
                toRemove << m_userList[i];
            }
        }
        for (auto user : toRemove) {
            auto index = m_userList.indexOf(user);
            beginRemoveRows(QModelIndex(), index, index);
            m_userList.removeOne(user);
            endRemoveRows();
        }
    });

    auto reply = m_dbusInterface->ListCachedUsers();
    reply.waitForFinished();

    if (reply.isError()) {
        qCWarning(KCMUSERS) << reply.error().message();
        return;
    }

    const QList<QDBusObjectPath> users = reply.value();
    for (const QDBusObjectPath& path: users) {
        User *user = new User(this);
        user->setPath(path);

        const std::list<QPair<void(User::*const)(),int>> set = {
            {&User::uidChanged, UidRole},
            {&User::nameChanged, NameRole},
            {&User::faceValidChanged, FaceValidRole},
            {&User::realNameChanged, RealNameRole},
            {&User::emailChanged, EmailRole},
            {&User::administratorChanged, AdministratorRole},
        };

        for (const auto &item: set) {
            connect(user, item.first, [this, user, item]{
                auto idx = index(m_userList.lastIndexOf(user));
                Q_EMIT dataChanged(idx, idx, {item.second});
            });
        }

        m_userList.append(user);
    }
    std::sort(m_userList.begin(), m_userList.end(), [](User *lhs, User *) {
        return lhs->loggedIn();
    });
}

QHash<int, QByteArray> UserModel::roleNames() const
{
    QHash<int, QByteArray> names = QAbstractItemModel::roleNames();
    names.insert(UidRole, "uid");
    names.insert(NameRole, "name");
    names.insert(EmailRole, "email");
    names.insert(AdministratorRole, "administrator");
    names.insert(UserRole, "User");
    names.insert(FaceValidRole, "faceValid");
    names.insert(LoggedInRole, "loggedIn");
    names.insert(SectionHeaderRole, "sectionHeader");
    return names;
}

UserModel::~UserModel()
{
}

User* UserModel::getLoggedInUser() const
{
    for (const auto user : qAsConst(m_userList)) {
        if (user->loggedIn()) {
            return user;
        }
    }
    return nullptr;
}

QVariant UserModel::data(const QModelIndex& index, int role) const
{
    if (!checkIndex(index))
    {
        return QVariant();
    }

    User *user = m_userList.at(index.row());

    switch (role) {
        case NameRole:
            return user->name();
        case FaceRole:
            return user->face().toString();
        case RealNameRole:
            return user->realName();
        case EmailRole:
            return user->email();
        case AdministratorRole:
            return user->administrator();
        case FaceValidRole:
            return QFile::exists(user->face().toString());
        case UserRole:
            return QVariant::fromValue(user);
        case LoggedInRole:
            return user->loggedIn();
        case SectionHeaderRole:
            return user->loggedIn() ? i18n("Your Account") : i18n("Other Accounts");
    }

    return QVariant();
}

int UserModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid()) {
        //Return size 0 if we are a child because this is not a tree
        return 0;
    }

    return m_userList.count();
}
