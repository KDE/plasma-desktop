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

#include "attica_plugin_debug.h"

#include <KConfigGroup>
#include <KWallet/KWallet>
#include <kcmultidialog.h>
#include <KLocalizedString>
#include <KStringHandler>
#include <KMessageBox>
#include <QNetworkDiskCache>
#include <QStorageInfo>

using namespace Attica;

KdePlatformDependent::KdePlatformDependent()
    : m_config(KSharedConfig::openConfig(QStringLiteral("atticarc"))), m_accessManager(nullptr), m_wallet(nullptr)
{
    // FIXME: Investigate how to not leak this instance without crashing.
    m_accessManager = new QNetworkAccessManager(nullptr);

    const QString cacheDir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + QStringLiteral("/attica");
    QNetworkDiskCache *cache = new QNetworkDiskCache(m_accessManager);
    QStorageInfo storageInfo(cacheDir);
    cache->setCacheDirectory(cacheDir);
    cache->setMaximumCacheSize(storageInfo.bytesTotal() / 1000);
    m_accessManager->setCache(cache);
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
    if (force || (!KWallet::Wallet::folderDoesNotExist(networkWallet, QStringLiteral("Attica")))) {
        m_wallet = KWallet::Wallet::openWallet(networkWallet, 0);
    }

    if (m_wallet) {
        m_wallet->createFolder(QStringLiteral("Attica"));
        m_wallet->setFolder(QStringLiteral("Attica"));
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
    const QStringList noauth = { QStringLiteral("no-auth-prompt"), QStringLiteral("true") };
    QNetworkRequest notConstReq = const_cast<QNetworkRequest&>(request);
    notConstReq.setAttribute(QNetworkRequest::User, noauth);
    return notConstReq;
}

bool KdePlatformDependent::saveCredentials(const QUrl& baseUrl, const QString& user, const QString& password)
{
    m_passwords[baseUrl.toString()] = qMakePair(user, password);

    if (!m_wallet && !openWallet(true)) {

        if (KMessageBox::warningContinueCancel(nullptr, i18n("Should the password be stored in the configuration file? This is unsafe.")
                , i18n("Social Desktop Configuration"))
                == KMessageBox::Cancel) {
            return false;
        }

        // use kconfig
        KConfigGroup group(m_config, baseUrl.toString());
        group.writeEntry("user", user);
        group.writeEntry("password", KStringHandler::obscure(password));
        qCDebug(ATTICA_PLUGIN_LOG) << "Saved credentials in KConfig";
        return true;
    }

    // Remove the entry when user name is empty
    if (user.isEmpty()) {
        m_wallet->removeEntry(baseUrl.toString());
        return true;
    }

    const QMap<QString, QString> entries = {
        { QStringLiteral("user"), user },
        { QStringLiteral("password"), password }
    };
    qCDebug(ATTICA_PLUGIN_LOG) << "Saved credentials in KWallet";

    return !m_wallet->writeMap(baseUrl.toString(), entries);
}

bool KdePlatformDependent::hasCredentials(const QUrl& baseUrl) const
{
    if (m_passwords.contains(baseUrl.toString())) {
        return true;
    }

    QString networkWallet = KWallet::Wallet::NetworkWallet();
    if (!KWallet::Wallet::folderDoesNotExist(networkWallet, QStringLiteral("Attica")) &&
        !KWallet::Wallet::keyDoesNotExist(networkWallet, QStringLiteral("Attica"), baseUrl.toString())) {
        qCDebug(ATTICA_PLUGIN_LOG) << "Found credentials in KWallet";
        return true;
    }

    KConfigGroup group(m_config, baseUrl.toString());

    const QString user = group.readEntry("user", QString());
    qCDebug(ATTICA_PLUGIN_LOG) << "Credentials found:" << !user.isEmpty();
    return !user.isEmpty();
}


bool KdePlatformDependent::loadCredentials(const QUrl& baseUrl, QString& user, QString& password)
{
    QString networkWallet = KWallet::Wallet::NetworkWallet();
    if (KWallet::Wallet::folderDoesNotExist(networkWallet, QStringLiteral("Attica")) &&
        KWallet::Wallet::keyDoesNotExist(networkWallet, QStringLiteral("Attica"), baseUrl.toString())) {
        // use KConfig
        KConfigGroup group(m_config, baseUrl.toString());
        user = group.readEntry("user", QString());
        password = KStringHandler::obscure(group.readEntry("password", QString()));
        if (!user.isEmpty()) {
            qCDebug(ATTICA_PLUGIN_LOG) << "Successfully loaded credentials from kconfig";
            m_passwords[baseUrl.toString()] = qMakePair(user, password);
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
    user = entries.value(QStringLiteral("user"));
    password = entries.value(QStringLiteral("password"));
    qCDebug(ATTICA_PLUGIN_LOG) << "Successfully loaded credentials.";

    m_passwords[baseUrl.toString()] = qMakePair(user, password);

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
    QStringList pathStrings = group.readPathEntry("providerFiles", QStringList(QStringLiteral("https://autoconfig.kde.org/ocs/providers.xml")));
    QList<QUrl> paths;
    foreach (const QString& pathString, pathStrings) {
        paths.append(QUrl(pathString));
    }
    qCDebug(ATTICA_PLUGIN_LOG) << "Loaded paths from config:" << paths;
    return paths;
}

void KdePlatformDependent::addDefaultProviderFile(const QUrl& url)
{
    KConfigGroup group(m_config, "General");
    QStringList pathStrings = group.readPathEntry("providerFiles", QStringList(QStringLiteral("https://autoconfig.kde.org/ocs/providers.xml")));
    QString urlString = url.toString();
    if(!pathStrings.contains(urlString)) {
        pathStrings.append(urlString);
        group.writeEntry("providerFiles", pathStrings);
        group.sync();
        qCDebug(ATTICA_PLUGIN_LOG) << "wrote providers: " << pathStrings;
    }
}

void KdePlatformDependent::removeDefaultProviderFile(const QUrl& url)
{
    KConfigGroup group(m_config, "General");
    QStringList pathStrings = group.readPathEntry("providerFiles", QStringList(QStringLiteral("https://autoconfig.kde.org/ocs/providers.xml")));
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

// TODO: re-enable, see https://community.kde.org/Frameworks/Porting_Notes
// Q_EXPORT_PLUGIN2(attica_kde, Attica::KdePlatformDependent)

