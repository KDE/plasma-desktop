/*
    SPDX-FileCopyrightText: 2019 Nicolas Fella <nicolas.fella@gmx.de>
    SPDX-FileCopyrightText: 2020 Carson Black <uhhadd@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#pragma once

#include <KJob>
#include <QDBusObjectPath>
#include <QObject>
#include <QPointer>
#include <QUrl>

class OrgFreedesktopAccountsUserInterface;
class QDBusError;

class UserApplyJob : public KJob
{
    Q_OBJECT

public:
    UserApplyJob(QPointer<OrgFreedesktopAccountsUserInterface> dbusIface, QString name, QString email, QString realname, QString icon, int type);
    void start() override;

    enum class Error {
        NoError = 0,
        PermissionDenied,
        Failed,
        Unknown,
        UserFacing,
    };

private:
    void setError(const QDBusError &error);

    QString m_name;
    QString m_email;
    QString m_realname;
    QString m_icon;
    int m_type;
    int m_jobs;
    QPointer<OrgFreedesktopAccountsUserInterface> m_dbusIface;
};

class User : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int uid READ uid WRITE setUid NOTIFY uidChanged)
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)

    Q_PROPERTY(QString realName READ realName WRITE setRealName NOTIFY realNameChanged)

    Q_PROPERTY(QString email READ email WRITE setEmail NOTIFY emailChanged)

    Q_PROPERTY(QUrl face READ face WRITE setFace NOTIFY faceChanged)

    Q_PROPERTY(bool faceValid READ faceValid NOTIFY faceValidChanged)

    Q_PROPERTY(QString password READ _ WRITE setPassword)

    Q_PROPERTY(bool loggedIn READ loggedIn CONSTANT)

    Q_PROPERTY(bool administrator READ administrator WRITE setAdministrator NOTIFY administratorChanged)

public:
    explicit User(QObject *parent = nullptr);

    int uid() const;
    void setUid(int value);

    QString name() const;
    QString realName() const;
    QString email() const;
    QUrl face() const;
    bool faceValid() const;
    bool loggedIn() const;
    bool administrator() const;
    QDBusObjectPath path() const;

    void setName(const QString &value);
    void setRealName(const QString &value);
    void setEmail(const QString &value);
    void setFace(const QUrl &value);
    void setPassword(const QString &value);
    void setAdministrator(bool value);
    void setPath(const QDBusObjectPath &path);

    void loadData();

    QString _()
    {
        return QString();
    };

public Q_SLOTS:
    Q_SCRIPTABLE void apply();
    Q_SCRIPTABLE bool usesDefaultWallet();
    Q_SCRIPTABLE void changeWalletPassword();

Q_SIGNALS:

    void dataChanged();
    void uidChanged();
    void nameChanged();
    void realNameChanged();
    void emailChanged();
    void faceChanged();
    void faceValidChanged();
    void administratorChanged();
    void applyError(const QString &errorMessage);
    void passwordSuccessfullyChanged();

private:
    int mUid = 0;
    QString mName = QString();
    QString mRealName = QString();
    QString mEmail = QString();
    QUrl mFace = QUrl();
    bool mAdministrator = false;
    bool mFaceValid = false;
    bool mLoggedIn = false;
    QDBusObjectPath mPath;
    QPointer<OrgFreedesktopAccountsUserInterface> m_dbusIface;
};
