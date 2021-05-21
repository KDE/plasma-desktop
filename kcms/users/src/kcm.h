/*
    SPDX-FileCopyrightText: 2016-2018 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
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
