/*
    SPDX-FileCopyrightText: 2013 Alejandro Fiestas Olivares <afiestas@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QDBusArgument>
#include <QDBusObjectPath>
#include <QObject>

struct UserInfo {
    uint id;
    QString name;
    QDBusObjectPath path;
};
Q_DECLARE_METATYPE(UserInfo)

using UserInfoList = QList<UserInfo>;
Q_DECLARE_METATYPE(UserInfoList)

class QDBusPendingCallWatcher;
class OrgFreedesktopLogin1ManagerInterface;
class UserSession : public QObject
{
    Q_OBJECT
public:
    explicit UserSession(QObject *parent = nullptr);
    ~UserSession() override;

public Q_SLOTS:
    void UserNew(uint id);
    void UserRemoved(uint id);
    void listUsersSlot(QDBusPendingCallWatcher *watcher);

Q_SIGNALS:
    void userLogged(uint id, bool logged);

private:
    OrgFreedesktopLogin1ManagerInterface *m_manager;
};
