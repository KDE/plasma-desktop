/*
    This file is part of KDE.

    SPDX-FileCopyrightText: 2009 Eckhart Wörner <ewoerner@kde.org>
    SPDX-FileCopyrightText: 2010 Frederik Gladhorn <gladhorn@kde.org>
    SPDX-FileCopyrightText: 2019 Dan Leinir Turthra Jensen <admin@leinir.dk>
    SPDX-FileCopyrightText: 2024 Harald Sitter <sitter@kde.or>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

*/

#include "kdeplatformdependent.h"

#include "attica_plugin_debug.h"

#include <KCMultiDialog>
#include <KConfigGroup>
#include <KLocalizedString>
#include <QApplication>
#include <QNetworkAccessManager>
#include <QNetworkDiskCache>
#include <QStorageInfo>

#include <Accounts/AccountService>
#include <Accounts/Manager>
#include <KAccounts/Core>
#include <KAccounts/GetCredentialsJob>

using namespace Attica;

KdePlatformDependent::KdePlatformDependent()
    : m_config(KSharedConfig::openConfig(QStringLiteral("atticarc")))
    , m_accessManager(nullptr)
{
    // FIXME: Investigate how to not leak this instance without crashing.
    m_accessManager = new QNetworkAccessManager(nullptr);

    const QString cacheDir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + QStringLiteral("/attica");
    QNetworkDiskCache *cache = new QNetworkDiskCache(m_accessManager);
    QStorageInfo storageInfo(QStandardPaths::writableLocation(QStandardPaths::CacheLocation));
    cache->setCacheDirectory(cacheDir);
    cache->setMaximumCacheSize(storageInfo.bytesTotal() / 1000);
    m_accessManager->setCache(cache);

    QMetaObject::invokeMethod(this, &KdePlatformDependent::loadAccessToken, Qt::QueuedConnection);
}

KdePlatformDependent::~KdePlatformDependent()
{
}

QUrl baseUrlFromRequest(const QNetworkRequest &request)
{
    const QUrl url{request.url()};
    QString baseUrl = QLatin1String("%1://%2").arg(url.scheme(), url.host());
    int port = url.port();
    if (port != -1) {
        baseUrl.append(QString::number(port));
    }
    return url;
}

QNetworkRequest KdePlatformDependent::addOAuthToRequest(const QNetworkRequest &request)
{
    QNetworkRequest notConstReq = const_cast<QNetworkRequest &>(request);
    const QString token = m_accessToken;
    if (!token.isEmpty()) {
        const QString bearer_format = QStringLiteral("Bearer %1");
        const QString bearer = bearer_format.arg(token);
        notConstReq.setRawHeader("Authorization", bearer.toUtf8());
    }
    notConstReq.setAttribute(QNetworkRequest::Http2AllowedAttribute, true);

    // Add cache preference in a granular fashion (we will almost certainly want more of these, but...)
    static const QStringList preferCacheEndpoints{QLatin1String{"/content/categories"}};
    for (const QString &endpoint : preferCacheEndpoints) {
        if (notConstReq.url().toString().endsWith(endpoint)) {
            QNetworkCacheMetaData cacheMeta{m_accessManager->cache()->metaData(notConstReq.url())};
            if (cacheMeta.isValid()) {
                // If the expiration date is valid, but longer than 24 hours, don't trust that things
                // haven't changed and check first, otherwise just use the cached version to relieve
                // server strain and reduce network traffic.
                const QDateTime tomorrow{QDateTime::currentDateTime().addDays(1)};
                if (cacheMeta.expirationDate().isValid() && cacheMeta.expirationDate() < tomorrow) {
                    notConstReq.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferCache);
                }
            }
            break;
        }
    }

    return notConstReq;
}

QNetworkReply *KdePlatformDependent::post(const QNetworkRequest &request, const QByteArray &data)
{
    return m_accessManager->post(addOAuthToRequest(removeAuthFromRequest(request)), data);
}

QNetworkReply *KdePlatformDependent::post(const QNetworkRequest &request, QIODevice *data)
{
    return m_accessManager->post(addOAuthToRequest(removeAuthFromRequest(request)), data);
}

QNetworkReply *KdePlatformDependent::get(const QNetworkRequest &request)
{
    return m_accessManager->get(addOAuthToRequest(removeAuthFromRequest(request)));
}

QNetworkRequest KdePlatformDependent::removeAuthFromRequest(const QNetworkRequest &request)
{
    const QStringList noauth = {QStringLiteral("no-auth-prompt"), QStringLiteral("true")};
    QNetworkRequest notConstReq = const_cast<QNetworkRequest &>(request);
    notConstReq.setAttribute(QNetworkRequest::User, noauth);
    return notConstReq;
}

bool KdePlatformDependent::saveCredentials(const QUrl & /*baseUrl*/, const QString & /*user*/, const QString & /*password*/)
{
    qCDebug(ATTICA_PLUGIN_LOG) << "Launch the KAccounts control module";
    // TODO KF6 This will want replacing with a call named something that suggests calling it shows accounts (and perhaps
    // directly requests the accounts kcm to start adding a new account if it's not there, maybe even pre-fills the fields...)

    KCMultiDialog *dialog = new KCMultiDialog;
    dialog->addModule(KPluginMetaData(QStringLiteral("kcm_kaccounts")));
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->show();

    return true;
}

bool KdePlatformDependent::hasCredentials([[maybe_unused]] const QUrl &baseUrl) const
{
    qCDebug(ATTICA_PLUGIN_LOG) << Q_FUNC_INFO;
    return !m_accessToken.isEmpty();
}

bool KdePlatformDependent::loadCredentials([[maybe_unused]] const QUrl &baseUrl, QString &user, QString & /*password*/)
{
    qCDebug(ATTICA_PLUGIN_LOG) << Q_FUNC_INFO;
    if (!m_accessToken.isEmpty()) {
        user = m_accessToken;
    }
    return !m_accessToken.isEmpty();
}

bool Attica::KdePlatformDependent::askForCredentials(const QUrl &baseUrl, QString &user, QString &password)
{
    Q_UNUSED(baseUrl);
    Q_UNUSED(user);
    Q_UNUSED(password);

    return false;
}

QList<QUrl> KdePlatformDependent::getDefaultProviderFiles() const
{
    KConfigGroup group(m_config, QStringLiteral("General"));
    const QStringList pathStrings = group.readPathEntry("providerFiles", QStringList(QStringLiteral("https://autoconfig.kde.org/ocs/providers.xml")));
    QList<QUrl> paths;
    for (const QString &pathString : pathStrings) {
        paths.append(QUrl(pathString));
    }
    qCDebug(ATTICA_PLUGIN_LOG) << "Loaded paths from config:" << paths;
    return paths;
}

void KdePlatformDependent::addDefaultProviderFile(const QUrl &url)
{
    KConfigGroup group(m_config, QStringLiteral("General"));
    QStringList pathStrings = group.readPathEntry("providerFiles", QStringList(QStringLiteral("https://autoconfig.kde.org/ocs/providers.xml")));
    QString urlString = url.toString();
    if (!pathStrings.contains(urlString)) {
        pathStrings.append(urlString);
        group.writeEntry("providerFiles", pathStrings);
        group.sync();
        qCDebug(ATTICA_PLUGIN_LOG) << "wrote providers: " << pathStrings;
    }
}

void KdePlatformDependent::removeDefaultProviderFile(const QUrl &url)
{
    KConfigGroup group(m_config, QStringLiteral("General"));
    QStringList pathStrings = group.readPathEntry("providerFiles", QStringList(QStringLiteral("https://autoconfig.kde.org/ocs/providers.xml")));
    pathStrings.removeAll(url.toString());
    group.writeEntry("providerFiles", pathStrings);
}

void KdePlatformDependent::enableProvider(const QUrl &baseUrl, bool enabled) const
{
    KConfigGroup group(m_config, QStringLiteral("General"));
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

bool KdePlatformDependent::isEnabled(const QUrl &baseUrl) const
{
    KConfigGroup group(m_config, QStringLiteral("General"));
    return !group.readPathEntry("disabledProviders", QStringList()).contains(baseUrl.toString());
}

QNetworkAccessManager *Attica::KdePlatformDependent::nam()
{
    return m_accessManager;
}

QNetworkReply *Attica::KdePlatformDependent::deleteResource(const QNetworkRequest &request)
{
    return m_accessManager->deleteResource(addOAuthToRequest(removeAuthFromRequest(request)));
}

QNetworkReply *Attica::KdePlatformDependent::put(const QNetworkRequest &request, QIODevice *data)
{
    return m_accessManager->put(addOAuthToRequest(removeAuthFromRequest(request)), data);
}

QNetworkReply *Attica::KdePlatformDependent::put(const QNetworkRequest &request, const QByteArray &data)
{
    return m_accessManager->put(addOAuthToRequest(removeAuthFromRequest(request)), data);
}

bool Attica::KdePlatformDependent::isReady()
{
    return !m_accessToken.isEmpty();
}

// TODO Don't just cache it forever, so reset to nullptr every so often, so we pick up potential new stuff the user's done
// NOTE The above todo is nonesense. Whatever changes the credentials should broadcast that something changed and then we
//   should reload. Also reloading shouldn't clear the m_accessToken but simply overwrite with a new one - sitter, 2024
void Attica::KdePlatformDependent::loadAccessToken()
{
    auto accountsManager = KAccounts::accountsManager();
    if (!accountsManager) {
        qCDebug(ATTICA_PLUGIN_LOG) << "No accounts manager could be fetched, so could not ask it for account details";
        return;
    }

    // TODO Present the user with a choice in case there's more than one, but for now just pick the latest successful one
    const auto accountIds = accountsManager->accountList(QStringLiteral("opendesktop-rating"));
    for (const auto &accountId : accountIds) {
        qCDebug(ATTICA_PLUGIN_LOG) << "Fetching data for" << accountId;

        auto account = accountsManager->account(accountId);
        if (!account) {
            qCWarning(ATTICA_PLUGIN_LOG) << "Failed to retrieve account" << accountId;
            continue;
        }

        auto job = new KAccounts::GetCredentialsJob(accountId, accountsManager);
        connect(job, &KJob::finished, [this, job, accountId = account->id()]() {
            const auto credentialsData = job->credentialsData();
            const auto idToken = credentialsData[QStringLiteral("IdToken")].toString();

            if (idToken.isEmpty()) {
                // If we arrived here, we did have an opendesktop account, but without the id token, which means an old version of the signon oauth2 plugin was
                // used
                qCWarning(ATTICA_PLUGIN_LOG) << "We got an OpenDesktop account, but it seems to be lacking the id token. This means an old SignOn OAuth2 "
                                                "plugin was used for logging in. The plugin may have been upgraded in the meantime, but an account created "
                                                "using the old plugin cannot be used, and you must log out and back in again.";
                return;
            }

            qCDebug(ATTICA_PLUGIN_LOG) << "OpenID Access token retrieved for account" << accountId;
            m_accessToken = idToken;
            Q_EMIT readyChanged();
        });
    }
}

#include "moc_kdeplatformdependent.cpp"
