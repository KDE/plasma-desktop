/*
    Copyright 2019  Nicolas Fella <nicolas.fella@gmx.de>
    Copyright 2020  Carson Black <uhhadd@gmail.com>
    Copyright 2020  David Redondo <kde@david-redondo.de>

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

#include "user.h"
#include "user_interface.h"
#include "kcmusers_debug.h"
#include <unistd.h>
#include <sys/types.h>
#include <QtConcurrent>
#include <KLocalizedString>

User::User(QObject* parent) : QObject(parent) {}

int User::uid() const {
    return mUid;
}

void User::setUid(int value) {
    
    if (mUid == value) {
        return;
    }
    mUid = value;
    Q_EMIT uidChanged();
}

QString User::name() const {
    return mName;
}

void User::setName(const QString &value) {
    
    if (mName == value) {
        return;
    }
    mName = value;
    Q_EMIT nameChanged();
}

QString User::realName() const {
    return mRealName;
}

void User::setRealName(const QString &value) {

    if (mRealName == value) {
        return;
    }
    mRealName = value;
    Q_EMIT realNameChanged();
}

QString User::email() const {
    return mEmail;
}

void User::setEmail(const QString &value) {
    
    if (mEmail == value) {
        return;
    }
    mEmail = value;
    Q_EMIT emailChanged();
}

QUrl User::face() const {
    return mFace;
}

bool User::faceValid() const {
    return mFaceValid;
}

void User::setFace(const QUrl &value) {

    if (mFace == value) {
        return;
    }
    mFace = value;
    mFaceValid = QFile::exists(value.path());
    Q_EMIT faceValidChanged();
    Q_EMIT faceChanged();
}

bool User::administrator() const {
    return mAdministrator;
}
void User::setAdministrator(bool value) {
    
    if (mAdministrator == value) {
        return;
    }
    mAdministrator = value;
    Q_EMIT administratorChanged();
}

void User::setPath(const QDBusObjectPath &path) {
    if (!m_dbusIface.isNull()) delete m_dbusIface;
    m_dbusIface = new OrgFreedesktopAccountsUserInterface(QStringLiteral("org.freedesktop.Accounts"), path.path(), QDBusConnection::systemBus(), this);

    if (m_dbusIface->systemAccount()) {
        return;
    }

    mPath = path;

    auto update = [=]() {
        bool userDataChanged = false;
        if (mUid != m_dbusIface->uid()) {
            mUid = m_dbusIface->uid();
            userDataChanged = true;
            Q_EMIT uidChanged();
        }
        if (mName != m_dbusIface->userName()) {
            mName = m_dbusIface->userName();
            userDataChanged = true;
            Q_EMIT nameChanged();
        }
        if (mFace != QUrl(m_dbusIface->iconFile())) {
            mFace = QUrl(m_dbusIface->iconFile());
            mFaceValid = QFileInfo::exists(mFace.toString());
            userDataChanged = true;
            Q_EMIT faceChanged();
            Q_EMIT faceValidChanged();
        }
        if (mRealName != m_dbusIface->realName()) {
            mRealName = m_dbusIface->realName();
            userDataChanged = true;
            Q_EMIT realNameChanged();
        }
        if (mEmail != m_dbusIface->email()) {
            mEmail = m_dbusIface->email();
            userDataChanged = true;
            Q_EMIT emailChanged();
        }
        const auto administrator = (m_dbusIface->accountType() == 1);
        if (mAdministrator != administrator) {
            mAdministrator = administrator;
            userDataChanged = true;
            Q_EMIT administratorChanged();
        }
        const auto loggedIn = (mUid == getuid());
        if (mLoggedIn != loggedIn) {
            mLoggedIn = loggedIn;
            userDataChanged = true;
        }
        if (userDataChanged) {
            Q_EMIT dataChanged();
        }
    };

    connect(m_dbusIface, &OrgFreedesktopAccountsUserInterface::Changed, [=]() {
        update();
    });

    update();
}

static char
saltCharacter() {
    static constexpr const quint32 letterCount = 64;
    static const char saltCharacters[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                         "abcdefghijklmnopqrstuvwxyz"
                                         "./0123456789"; // and trailing NUL
    static_assert(sizeof(saltCharacters) == (letterCount+1), // 64 letters and trailing NUL
                  "Salt-chars array is not exactly 64 letters long");
    
    const quint32 index = QRandomGenerator::system()->bounded(0u, letterCount);

    return saltCharacters[index];
}

static QString
saltPassword(const QString &plain)
{
    QString salt;

    salt.append("$6$");

    for (auto i = 0; i < 16; i++) {
        salt.append(saltCharacter());
    }

    salt.append("$");

    auto stdStrPlain = plain.toStdString();
    auto cStrPlain = stdStrPlain.c_str();
    auto stdStrSalt = salt.toStdString();
    auto cStrSalt = stdStrSalt.c_str();

    auto salted = crypt(cStrPlain, cStrSalt);

    return QString::fromUtf8(salted);
}

void User::setPassword(const QString &password)
{
    m_dbusIface->SetPassword(saltPassword(password), QString());
}

QDBusObjectPath User::path() const
{
    return mPath;
}

void User::apply()
{
    auto job = new UserApplyJob(m_dbusIface, mName, mEmail, mRealName, mFace.toString().replace("file://", ""), mAdministrator ? 1 : 0);
    connect(job, &UserApplyJob::result, this, [this, job] {
        switch (static_cast<UserApplyJob::Error>(job->error())) {
        case UserApplyJob::Error::PermissionDenied:
            Q_EMIT applyError(i18n("Could not get permission to save user %1", mName));
            break;
        case UserApplyJob::Error::Failed:
        case UserApplyJob::Error::Unknown:
            Q_EMIT applyError(i18n("There was an error while saving changes"));
            break;
        case UserApplyJob::Error::NoError: ; // Do nothing
        }
    });
    job->start();
}

bool User::loggedIn() const
{
    return mLoggedIn;
}

UserApplyJob::UserApplyJob(QPointer<OrgFreedesktopAccountsUserInterface> dbusIface, QString name, QString email, QString realname, QString icon, int type)
    : KJob(),
    m_name(name),
    m_email(email),
    m_realname(realname),
    m_icon(icon),
    m_type(type),
    m_dbusIface(dbusIface)
{
}

void UserApplyJob::start()
{
    const std::multimap<QString,QDBusPendingReply<> (OrgFreedesktopAccountsUserInterface::*)(const QString&)> set = {
        {m_name, &OrgFreedesktopAccountsUserInterface::SetUserName},
        {m_email, &OrgFreedesktopAccountsUserInterface::SetEmail},
        {m_realname, &OrgFreedesktopAccountsUserInterface::SetRealName},
        {m_icon, &OrgFreedesktopAccountsUserInterface::SetIconFile}
    };
    // Do our dbus invocations with blocking calls, since the accounts service
    // will return permission denied if there's a polkit dialog open while a 
    // request is made.
    for (auto const &x: set) {
        auto resp = (m_dbusIface->*(x.second))(x.first);
        resp.waitForFinished();
        if (resp.isError()) {
            setError(resp.error());
            qCWarning(KCMUSERS) << resp.error().name() << resp.error().message();
            emitResult();
            return;
        }
    }
    auto setAccount = m_dbusIface->SetAccountType(m_type);
    setAccount.waitForFinished();
    if (setAccount.isError()) {
        setError(setAccount.error());
        qCWarning(KCMUSERS) << setAccount.error().name() << setAccount.error().message();
    }
    emitResult();
}

void UserApplyJob::setError(const QDBusError& error)
{
    setErrorText(error.message());
    if (error.name() == QLatin1String("org.freedesktop.Accounts.Error.Failed")) {
        KJob::setError(static_cast<int>(Error::Failed));
    } else if (error.name() == QLatin1String("org.freedesktop.Accounts.Error.PermissionDenied")) {
        KJob::setError(static_cast<int>(Error::PermissionDenied));
    } else {
        KJob::setError(static_cast<int>(Error::Unknown));
    }
}
