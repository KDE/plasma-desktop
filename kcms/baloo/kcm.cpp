/* This file is part of the KDE Project
   Copyright (c) 2007-2010 Sebastian Trueg <trueg@kde.org>
   Copyright (c) 2012-2014 Vishesh Handa <me@vhanda.in>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "kcm.h"
#include "fileexcludefilters.h"

#include <KPluginFactory>
#include <KPluginLoader>
#include <KAboutData>
#include <QDebug>
#include <QStandardPaths>
#include <KLocalizedString>

#include <QPushButton>
#include <QDir>
#include <QProcess>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusPendingCall>
#include <QStorageInfo>

#include <Baloo/IndexerConfig>

K_PLUGIN_FACTORY_WITH_JSON(KCMColorsFactory, "kcm_baloofile.json", registerPlugin<Baloo::ServerConfigModule>();)

namespace
{
    QStringList defaultFolders()
    {
        return { QDir::homePath() };
    }
} // namespace

using namespace Baloo;

ServerConfigModule::ServerConfigModule(QObject* parent, const QVariantList& args)
    : KQuickAddons::ConfigModule(parent, args)
    , m_filteredFolderModel(new FilteredFolderModel(this))
    , m_indexing(false)
    , m_fileContents(false)
    {
    qmlRegisterType<FilteredFolderModel>();

    KAboutData* about = new KAboutData(
        QStringLiteral("kcm_baloofile"), i18n("File Search"),
        QStringLiteral("0.1"), QString(), KAboutLicense::GPL,
        i18n("Copyright 2007-2010 Sebastian Trüg"));

    about->addAuthor(i18n("Sebastian Trüg"), QString(), QStringLiteral("trueg@kde.org"));
    about->addAuthor(i18n("Vishesh Handa"), QString(), QStringLiteral("vhanda@kde.org"));
    about->addAuthor(i18n("Tomaz Canabrava"), QString(), QStringLiteral("tcnaabrava@kde.org"));

    setAboutData(about);
    setButtons(Help | Apply | Default);

    connect(m_filteredFolderModel, &FilteredFolderModel::folderAdded, this, [this]{ setNeedsSave(true); });
    connect(m_filteredFolderModel, &FilteredFolderModel::folderRemoved, this, [this]{ setNeedsSave(true); });
}

ServerConfigModule::~ServerConfigModule()
{
}

void ServerConfigModule::load()
{
    Baloo::IndexerConfig config;
    m_indexing = config.fileIndexingEnabled();
    m_previouslyEnabled = m_indexing;
    m_fileContents = !config.onlyBasicIndexing();

    m_filteredFolderModel->setDirectoryList(config.includeFolders(), config.excludeFolders());

    emit indexingChanged(m_indexing);
    emit fileContentsChanged(m_fileContents);
}

void ServerConfigModule::save()
{
    bool enabled = m_indexing && !allMountPointsExcluded();

    Baloo::IndexerConfig config;
    config.setFileIndexingEnabled(enabled);
    config.setIncludeFolders(m_filteredFolderModel->includeFolders());
    config.setExcludeFolders(m_filteredFolderModel->excludeFolders());
    config.setOnlyBasicIndexing(!m_fileContents);
    config.setFirstRun(false);

    if (m_previouslyEnabled != enabled) {
        config.setFirstRun(true);
    }

    // Start Baloo
    if (enabled) {
        const QString exe = QStandardPaths::findExecutable(QStringLiteral("baloo_file"));
        QProcess::startDetached(exe, QStringList());
    }
    else {
        QDBusMessage message = QDBusMessage::createMethodCall(
            QStringLiteral("org.kde.baloo"),
            QStringLiteral("/"),
            QStringLiteral("org.kde.baloo.main"),
            QStringLiteral("quit")
        );

        QDBusConnection::sessionBus().asyncCall(message);
    }

    // Update the baloo_file's config cache
    config.refresh();
}

void ServerConfigModule::defaults()
{
    m_filteredFolderModel->setDirectoryList(defaultFolders(), QStringList());
}

bool ServerConfigModule::indexing() const
{
    return m_indexing;
}

bool ServerConfigModule::fileContents() const
{
    return m_fileContents;
}

FilteredFolderModel *ServerConfigModule::filteredModel() const {
    return m_filteredFolderModel;
}

void ServerConfigModule::setIndexing(bool indexing)
{
    if (m_indexing != indexing) {
        m_indexing = indexing;
        Q_EMIT indexingChanged(indexing);
        setNeedsSave(true);
    }
}

void ServerConfigModule::setFileContents(bool fileContents)
{
    if (m_fileContents != fileContents) {
        m_fileContents = fileContents;
        Q_EMIT fileContentsChanged(fileContents);
        setNeedsSave(true);
    }
}

bool ServerConfigModule::allMountPointsExcluded()
{
    QStringList mountPoints;
    for (const QStorageInfo &si : QStorageInfo::mountedVolumes()) {
        mountPoints.append(si.rootPath());
    }

    return m_filteredFolderModel->excludeFolders().toSet() == mountPoints.toSet();
}

#include "kcm.moc"
