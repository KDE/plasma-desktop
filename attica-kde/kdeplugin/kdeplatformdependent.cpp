/*
    This file is part of KDE.

    Copyright (c) 2009 Eckhart Wörner <ewoerner@kde.org>
    Copyright (c) 2010 Frederik Gladhorn <gladhorn@kde.org>

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

#include "kdeplatformdependent.h"

#include <QDebug>

#include <KConfigGroup>
#include <kcomponentdata.h>
#include <KDebug>
#include <KWallet/Wallet>
#include <kcmultidialog.h>
#include <KLocalizedString>
#include <KStringHandler>
#include <KMessageBox>

using namespace Attica;

KdePlatformDependent::KdePlatformDependent()
    : m_config(KSharedConfig::openConfig("atticarc")), m_accessManager(0), m_wallet(0)
{
    // FIXME: Investigate how to not leak this instance witohut crashing.
    m_accessManager = new KIO::Integration::AccessManager(0);
}

KdePlatformDependent::~KdePlatformDependent()
{
    delete m_wallet;
}

bool KdePlatformDependent::openWallet(bool force)
{
    if (m_wallet) {
        return true;
    }

    QString networkWallet = KWallet::Wallet::NetworkWallet();
    // if not forced, or the folder doesn't exist, don't try to open the wallet
    if (force || (!KWallet::Wallet::folderDoesNotExist(networkWallet, "Attica"))) {
        m_wallet = KWallet::Wallet::openWallet(networkWallet, 0);
    }

    if (m_wallet) {
        m_wallet->createFolder("Attica");
        m_wallet->setFolder("Attica");
        return true;
    }
    return false;
}

QNetworkReply* KdePlatformDependent::post(const QNetworkRequest& request, const QByteArray& data)
{
    return m_accessManager->post(removeAuthFromRequest(request), data);
}

QNetworkReply* KdePlatformDependent::post(const QNetworkRequest& request, QIODevice* data)
{
    return m_accessManager->post(removeAuthFromRequest(request), data);
}

QNetworkReply* KdePlatformDependent::get(const QNetworkRequest& request)
{
    return m_accessManager->get(removeAuthFromRequest(request));
}

QNetworkRequest KdePlatformDependent::removeAuthFromRequest(const QNetworkRequest& request)
{
    QStringList noauth;
    noauth << "no-auth-prompt" << "true";
    QNetworkRequest notConstReq = const_cast<QNetworkRequest&>(request);
    notConstReq.setAttribute(QNetworkRequest::User, noauth);
    return notConstReq;
}

bool KdePlatformDependent::saveCredentials(const QUrl& baseUrl, const QString& user, const QString& password)
{
    m_passwords[baseUrl.toString()] = QPair<QString, QString> (user, password);

    if (!m_wallet && !openWallet(true)) {

        if (KMessageBox::warningContinueCancel(0, i18n("Should the password be stored in the configuration file? This is unsafe.")
                , i18n("Social Desktop Configuration"))
                == KMessageBox::Cancel) {
            return false;
        }

        // use kconfig
        KConfigGroup group(m_config, baseUrl.toString());
        group.writeEntry("user", user);
        group.writeEntry("password", KStringHandler::obscure(password));
        kDebug() << "Saved credentials in KConfig";
        return true;
    }

    // Remove the entry when user name is empty
    if (user.isEmpty()) {
        m_wallet->removeEntry(baseUrl.toString());
        return true;
    }

    QMap<QString, QString> entries;
    entries.insert("user", user);
    entries.insert("password", password);
    kDebug() << "Saved credentials in KWallet";

    return !m_wallet->writeMap(baseUrl.toString(), entries);
}

bool KdePlatformDependent::hasCredentials(const QUrl& baseUrl) const
{
    if (m_passwords.contains(baseUrl.toString())) {
        return true;
    }

    QString networkWallet = KWallet::Wallet::NetworkWallet();
    if (!KWallet::Wallet::folderDoesNotExist(networkWallet, "Attica") &&
        !KWallet::Wallet::keyDoesNotExist(networkWallet, "Attica", baseUrl.toString())) {
        kDebug() << "Found credentials in KWallet";
        return true;
    }

    KConfigGroup group(m_config, baseUrl.toString());
    QString user;
    user = group.readEntry("user", QString());
    if (!user.isEmpty()) {
        kDebug() << "Found credentials in KConfig";
        return true;
    }

    kDebug() << "No credentials found";
    return false;
}


bool KdePlatformDependent::loadCredentials(const QUrl& baseUrl, QString& user, QString& password)
{
    QString networkWallet = KWallet::Wallet::NetworkWallet();
    if (KWallet::Wallet::folderDoesNotExist(networkWallet, "Attica") &&
        KWallet::Wallet::keyDoesNotExist(networkWallet, "Attica", baseUrl.toString())) {
        // use KConfig
        KConfigGroup group(m_config, baseUrl.toString());
        user = group.readEntry("user", QString());
        password = KStringHandler::obscure(group.readEntry("password", QString()));
        if (!user.isEmpty()) {
            kDebug() << "Successfully loaded credentials from kconfig";
            m_passwords[baseUrl.toString()] = QPair<QString, QString> (user, password);
            return true;
        }
        return false;
    }

    if (!m_wallet && !openWallet(true)) {
        return false;
    }

    QMap<QString, QString> entries;
    if (m_wallet->readMap(baseUrl.toString(), entries) != 0) {
        return false;
    }
    user = entries.value("user");
    password = entries.value("password");
    kDebug() << "Successfully loaded credentials.";

    m_passwords[baseUrl.toString()] = QPair<QString, QString> (user, password);

    return true;
}


bool Attica::KdePlatformDependent::askForCredentials(const QUrl& baseUrl, QString& user, QString& password)
{
    Q_UNUSED(baseUrl);
    Q_UNUSED(user);
    Q_UNUSED(password);

    return false;
}

QList<QUrl> KdePlatformDependent::getDefaultProviderFiles() const
{
    KConfigGroup group(m_config, "General");
    QStringList pathStrings = group.readPathEntry("providerFiles", QStringList("http://download.kde.org/ocs/providers.xml"));
    QList<QUrl> paths;
    foreach (const QString& pathString, pathStrings) {
        paths.append(QUrl(pathString));
    }
    qDebug() << "Loaded paths from config:" << paths;
    return paths;
}

void KdePlatformDependent::addDefaultProviderFile(const QUrl& url)
{
    KConfigGroup group(m_config, "General");
    QStringList pathStrings = group.readPathEntry("providerFiles", QStringList("http://download.kde.org/ocs/providers.xml"));
    QString urlString = url.toString();
    if(!pathStrings.contains(urlString)) {
        pathStrings.append(urlString);
        group.writeEntry("providerFiles", pathStrings);
        group.sync();
        kDebug() << "wrote providers: " << pathStrings;
    }
}

void KdePlatformDependent::removeDefaultProviderFile(const QUrl& url)
{
    KConfigGroup group(m_config, "General");
    QStringList pathStrings = group.readPathEntry("providerFiles", QStringList("http://download.kde.org/ocs/providers.xml"));
    pathStrings.removeAll(url.toString());
    group.writeEntry("providerFiles", pathStrings);
}

void KdePlatformDependent::enableProvider(const QUrl& baseUrl, bool enabled) const
{
    KConfigGroup group(m_config, "General");
    QStringList pathStrings = group.readPathEntry("disabledProviders", QStringList());
    if (enabled) {
        pathStrings.removeAll(baseUrl.toString());
    } else {
        if (!pathStrings.contains(baseUrl.toString())) {
            pathStrings.append(baseUrl.toString());
        }
    }
    group.writeEntry("disabledProviders", pathStrings);
    group.sync();
}

bool KdePlatformDependent::isEnabled(const QUrl& baseUrl) const
{
    KConfigGroup group(m_config, "General");
    return !group.readPathEntry("disabledProviders", QStringList()).contains(baseUrl.toString());
}

QNetworkAccessManager* Attica::KdePlatformDependent::nam()
{
    return m_accessManager;
}

// TODO: re-enable, see http://community.kde.org/Frameworks/Porting_Notes
// Q_EXPORT_PLUGIN2(attica_kde, Attica::KdePlatformDependent)

