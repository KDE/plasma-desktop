/*
    Copyright 2016-2018 Jan Grulich <jgrulich@redhat.com>

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

#include "usermodel.h"
#include <KQuickAddons/ConfigModule>

class OrgFreedesktopAccountsInterface;

class QQuickView;

class KCMUser : public KQuickAddons::ConfigModule
{
    Q_OBJECT
    Q_PROPERTY(UserModel *userModel MEMBER m_model CONSTANT)
    Q_PROPERTY(QStringList avatarFiles MEMBER m_avatarFiles CONSTANT)

private:
    OrgFreedesktopAccountsInterface *m_dbusInterface;
    UserModel *m_model;
    QStringList m_avatarFiles;

public:
    KCMUser(QObject *parent = nullptr, const QVariantList &args = QVariantList());
    ~KCMUser() override;

    Q_SCRIPTABLE bool createUser(const QString &name, const QString &realName, const QString &password, bool admin);
    Q_SCRIPTABLE bool deleteUser(int index, bool deleteHome);
    // Grab the initials of a string
    Q_SCRIPTABLE QString initializeString(const QString &stringToGrabInitialsOf);
    Q_SCRIPTABLE QString plonkImageInTempfile(const QImage &image);

Q_SIGNALS:
    Q_SCRIPTABLE void apply();
    Q_SCRIPTABLE void reset();

public Q_SLOTS:
    void save() override;
    void load() override;
};
