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
    UserApplyJob(QPointer<OrgFreedesktopAccountsUserInterface> dbusIface, std::optional<QString> name, std::optional<QString> email, std::optional<QString> realname, std::optional<QString> icon, int type);
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

    std::optional<QString> m_name;
    std::optional<QString> m_email;
    std::optional<QString> m_realname;
    std::optional<QString> m_icon;
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
    QString mCurrentName;
    QString mCurrentRealName;
    QString mCurrentEmail;
    QUrl mCurrentFace = QUrl();
    bool mCurrentFaceValid = false;
    bool mCurrentAdministrator = false;

    std::optional<QString> mName;
    std::optional<QString> mRealName;
    std::optional<QString> mEmail;
    std::optional<QUrl> mFace;
    std::optional<bool> mFaceValid = false;
    std::optional<bool> mAdministrator = false;

    bool mLoggedIn = false;
    QDBusObjectPath mPath;
    QPointer<OrgFreedesktopAccountsUserInterface> m_dbusIface;
};
