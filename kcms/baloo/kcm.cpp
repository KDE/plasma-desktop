/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2007-2010 Sebastian Trueg <trueg@kde.org>
    SPDX-FileCopyrightText: 2012-2014 Vishesh Handa <me@vhanda.in>
    SPDX-FileCopyrightText: 2020 Benjamin Port <benjamin.port@enioka.com>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "kcm.h"
#include "fileexcludefilters.h"

#include <KAboutData>
#include <KFormat>
#include <KLocalizedString>
#include <KPluginFactory>
#include <QStandardPaths>

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusPendingCall>
#include <QDir>
#include <QProcess>
#include <QPushButton>

#include <Baloo/IndexerConfig>
#include <baloo/baloosettings.h>
#include <baloodata.h>

K_PLUGIN_FACTORY_WITH_JSON(KCMColorsFactory, "kcm_baloofile.json", registerPlugin<Baloo::ServerConfigModule>(); registerPlugin<BalooData>();)

static QString balooDatabaseLocation()
{
    // First consult the environment variable, in case the index has been
    // relocated manually
    QString location = QString::fromLocal8Bit(qgetenv("BALOO_DB_PATH"));
    if (location.isEmpty()) {
        location = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1String("/baloo/index");
    } else {
        location.append(QLatin1String("/index"));
    }
    return location;
}

using namespace Baloo;

ServerConfigModule::ServerConfigModule(QObject *parent, const QVariantList &args)
    : KQuickAddons::ManagedConfigModule(parent, args)
    , m_data(new BalooData(this))
    , m_filteredFolderModel(new FilteredFolderModel(m_data->settings(), this))
{
    qmlRegisterType<FilteredFolderModel>();
    qmlRegisterType<BalooSettings>();

    KAboutData *about = new KAboutData(QStringLiteral("kcm_baloofile"),
                                       i18n("File Search"),
                                       QStringLiteral("0.1"),
                                       QString(),
                                       KAboutLicense::GPL,
                                       i18n("Copyright 2007-2010 Sebastian Trüg"));

    about->addAuthor(i18n("Sebastian Trüg"), QString(), QStringLiteral("trueg@kde.org"));
    about->addAuthor(i18n("Vishesh Handa"), QString(), QStringLiteral("vhanda@kde.org"));
    about->addAuthor(i18n("Tomaz Canabrava"), QString(), QStringLiteral("tcnaabrava@kde.org"));

    setAboutData(about);
    setButtons(Help | Apply | Default);

    connect(balooSettings(), &BalooSettings::excludedFoldersChanged, m_filteredFolderModel, &FilteredFolderModel::updateDirectoryList);
    connect(balooSettings(), &BalooSettings::foldersChanged, m_filteredFolderModel, &FilteredFolderModel::updateDirectoryList);
    m_filteredFolderModel->updateDirectoryList();
}

ServerConfigModule::~ServerConfigModule()
{
}

void ServerConfigModule::load()
{
    ManagedConfigModule::load();
}

void ServerConfigModule::save()
{
    ManagedConfigModule::save();

    // Update Baloo config or start Baloo
    if (balooSettings()->indexingEnabled()) {
        // Update the baloo_file's config cache - if it not yet running,
        // the DBus call goes nowhere
        Baloo::IndexerConfig config;
        config.refresh();

        // Trying to start baloo when it is already running is fine
        const QString exe = QStandardPaths::findExecutable(QStringLiteral("baloo_file"));
        QProcess::startDetached(exe, QStringList());
    } else {
        QDBusMessage message =
            QDBusMessage::createMethodCall(QStringLiteral("org.kde.baloo"), QStringLiteral("/"), QStringLiteral("org.kde.baloo.main"), QStringLiteral("quit"));

        QDBusConnection::sessionBus().asyncCall(message);
    }
}

FilteredFolderModel *ServerConfigModule::filteredModel() const
{
    return m_filteredFolderModel;
}

BalooSettings *ServerConfigModule::balooSettings() const
{
    return m_data->settings();
}

void ServerConfigModule::deleteIndex()
{
    QFile(balooDatabaseLocation()).remove();
}

int ServerConfigModule::rawIndexFileSize()
{
    return QFile(balooDatabaseLocation()).size();
}

QString ServerConfigModule::prettyIndexFileSize()
{
    return KFormat().formatByteSize(rawIndexFileSize());
}

#include "kcm.moc"
