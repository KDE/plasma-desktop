/*
    SPDX-FileCopyrightText: 2016-2018 Jan Grulich <jgrulich@redhat.com>
    SPDX-FileCopyrightText: 2019 Nicolas Fella <nicolas.fella@gmx.de>
    SPDX-FileCopyrightText: 2020 Carson Black <uhhadd@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "kcm.h"

// KDE
#include <KAboutData>
#include <KLocalizedString>
#include <KPluginFactory>
// Qt
#include <QApplication>
#include <QFileDialog>
#include <QFontMetrics>
#include <QQmlProperty>
#include <QTemporaryFile>
#include <QTimer>
#include <QtQuick/QQuickItem>

#include "accounts_interface.h"
#include "user.h"

Q_LOGGING_CATEGORY(kcm_users, "kcm_users")

K_PLUGIN_CLASS_WITH_JSON(KCMUser, "kcm_users.json")

KCMUser::KCMUser(QObject *parent, const QVariantList &args)
    : KQuickAddons::ConfigModule(parent, args)
    , m_dbusInterface(new OrgFreedesktopAccountsInterface(QStringLiteral("org.freedesktop.Accounts"),
                                                          QStringLiteral("/org/freedesktop/Accounts"),
                                                          QDBusConnection::systemBus(),
                                                          this))
    , m_model(new UserModel(this))
{
    qmlRegisterUncreatableType<User>("org.kde.plasma.kcm.users", 1, 0, "User", QString());
    KAboutData *about = new KAboutData(QStringLiteral("kcm_users"), i18n("Manage user accounts"), QStringLiteral("0.1"), QString(), KAboutLicense::GPL);
    about->addAuthor(i18n("Nicolas Fella"), QString(), QStringLiteral("nicolas.fella@gmx.de"));
    about->addAuthor(i18n("Carson Black"), QString(), QStringLiteral("uhhadd@gmail.com"));
    setAboutData(about);
    setButtons(Apply);
    auto font = QApplication::font("QLabel");
    auto fm = QFontMetrics(font);
    setColumnWidth(fm.capHeight() * 30);

    const auto dirs = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, QStringLiteral("plasma/avatars"), QStandardPaths::LocateDirectory);
    for (const auto &dir : dirs) {
        QDirIterator it(dir, QStringList{QStringLiteral("*.jpg"), QStringLiteral("*.png")}, QDir::Files, QDirIterator::Subdirectories);

        while (it.hasNext()) {
            m_avatarFiles << it.next();
        }
    }
}

bool KCMUser::createUser(const QString &name, const QString &realName, const QString &password, bool isAdmin)
{
    QDBusPendingReply<QDBusObjectPath> reply = m_dbusInterface->CreateUser(name, realName, isAdmin);
    reply.waitForFinished();
    if (reply.isValid()) {
        User *createdUser = new User(this);
        createdUser->setPath(reply.value());
        createdUser->setPassword(password);
        delete createdUser;
        return true;
    }
    return false;
}

bool KCMUser::deleteUser(int id, bool deleteHome)
{
    QDBusPendingReply<> reply = m_dbusInterface->DeleteUser(id, deleteHome);
    reply.waitForFinished();
    if (reply.isError()) {
        return false;
    } else {
        return true;
    }
}

KCMUser::~KCMUser()
{
}

void KCMUser::save()
{
    KQuickAddons::ConfigModule::save();
    Q_EMIT apply();
}

// Grab the initials of a string
QString KCMUser::initializeString(const QString &stringToGrabInitialsOf)
{
    if (stringToGrabInitialsOf.isEmpty())
        return "";

    auto normalized = stringToGrabInitialsOf.normalized(QString::NormalizationForm_D);
    if (normalized.contains(" ")) {
        QStringList split = normalized.split(" ");
        auto first = split.first();
        auto last = split.last();
        if (first.isEmpty()) {
            return QString(last.front());
        }
        if (last.isEmpty()) {
            return QString(first.front());
        }
        return QString(first.front()) + last.front();
    } else {
        return QString(normalized.front());
    }
}

QString KCMUser::plonkImageInTempfile(const QImage &image)
{
    auto file = new QTemporaryFile(qApp);
    if (file->open()) {
        image.save(file, "PNG");
    }
    return file->fileName();
}

void KCMUser::load()
{
    Q_EMIT reset();
}

#include "kcm.moc"
