/*
    This file is part of KDE.

    SPDX-FileCopyrightText: 2009 Eckhart Wörner <ewoerner@kde.org>
    SPDX-FileCopyrightText: 2010 Frederik Gladhorn <gladhorn@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

*/

#ifndef ATTICA_KDEPLATFORMDEPENDENT_H
#define ATTICA_KDEPLATFORMDEPENDENT_H

#include <attica/platformdependent.h>

#include <QHash>

#include <KIO/AccessManager>
#include <KSharedConfig>

namespace Attica
{
class KdePlatformDependent : public QObject, public Attica::PlatformDependent
{
    Q_OBJECT
    Q_INTERFACES(Attica::PlatformDependent)
    Q_PLUGIN_METADATA(IID "org.kde.attica-kde")

public:
    KdePlatformDependent();
    ~KdePlatformDependent() override;
    QList<QUrl> getDefaultProviderFiles() const override;
    void addDefaultProviderFile(const QUrl &url) override;
    void removeDefaultProviderFile(const QUrl &url) override;
    void enableProvider(const QUrl &baseUrl, bool enabled) const override;
    bool isEnabled(const QUrl &baseUrl) const override;

    QNetworkReply *post(const QNetworkRequest &request, const QByteArray &data) override;
    QNetworkReply *post(const QNetworkRequest &request, QIODevice *data) override;
    QNetworkReply *get(const QNetworkRequest &request) override;
    bool saveCredentials(const QUrl &baseUrl, const QString &user, const QString &password) override;
    bool hasCredentials(const QUrl &baseUrl) const override;
    bool loadCredentials(const QUrl &baseUrl, QString &user, QString &password) override;
    bool askForCredentials(const QUrl &baseUrl, QString &user, QString &password) override;
    QNetworkAccessManager *nam() override;

private:
    QNetworkRequest addOAuthToRequest(const QNetworkRequest &request);
    QNetworkRequest removeAuthFromRequest(const QNetworkRequest &request);
    QString getAccessToken(const QUrl &baseUrl) const;

    KSharedConfigPtr m_config;
    QNetworkAccessManager *m_accessManager{nullptr};
    QHash<QString, QPair<QString, QString>> m_passwords;
};

}

#endif
