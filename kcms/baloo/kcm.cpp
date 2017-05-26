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
#include "folderselectionwidget.h"

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

K_PLUGIN_FACTORY(BalooConfigModuleFactory, registerPlugin<Baloo::ServerConfigModule>();)
K_EXPORT_PLUGIN(BalooConfigModuleFactory("kcm_baloofile"))


namespace
{
QStringList defaultFolders()
{
    return QStringList() << QDir::homePath();
}

}

using namespace Baloo;

ServerConfigModule::ServerConfigModule(QWidget* parent, const QVariantList& args)
    : KCModule(parent, args)
{
    KAboutData* about = new KAboutData(
        QStringLiteral("kcm_baloofile"), i18n("Configure File Search"),
        QStringLiteral("0.1"), QString(), KAboutLicense::GPL,
        i18n("Copyright 2007-2010 Sebastian Trüg"));
    about->addAuthor(i18n("Sebastian Trüg"), QString(), QStringLiteral("trueg@kde.org"));
    about->addAuthor(i18n("Vishesh Handa"), QString(), QStringLiteral("vhanda@kde.org"));
    setAboutData(about);
    setButtons(Help | Apply | Default);

    setupUi(this);

    connect(m_excludeFolders_FSW, SIGNAL(changed()),
            this, SLOT(changed()));
    connect(m_excludeFolders_FSW, &FolderSelectionWidget::changed,
            this, &ServerConfigModule::onDirectoryListChanged);
    connect(m_enableCheckbox, SIGNAL(stateChanged(int)),
            this, SLOT(changed()));
    connect(m_enableContentIndexing, SIGNAL(stateChanged(int)),
            this, SLOT(changed()));
    connect(m_enableCheckbox, SIGNAL(stateChanged(int)),
            this, SLOT(indexingEnabledChanged()));
}


ServerConfigModule::~ServerConfigModule()
{
}


void ServerConfigModule::load()
{
    Baloo::IndexerConfig config;

    m_previouslyEnabled = config.fileIndexingEnabled();
    m_enableCheckbox->setChecked(m_previouslyEnabled);

    m_enableContentIndexing->setChecked(!config.onlyBasicIndexing());
    m_enableContentIndexing->setEnabled(m_enableCheckbox->isChecked());

    QStringList includeFolders = config.includeFolders();
    QStringList excludeFolders = config.excludeFolders();
    m_excludeFolders_FSW->setDirectoryList(includeFolders, excludeFolders);

    // All values loaded -> no changes
    Q_EMIT changed(false);
}


void ServerConfigModule::save()
{
    QStringList includeFolders = m_excludeFolders_FSW->includeFolders();
    QStringList excludeFolders = m_excludeFolders_FSW->excludeFolders();

    bool mountPointsEx = allMountPointsExcluded();

    bool enabled = m_enableCheckbox->isChecked();
    if (mountPointsEx)
        enabled = false;

    Baloo::IndexerConfig config;
    config.setFileIndexingEnabled(enabled);
    config.setIncludeFolders(includeFolders);
    config.setExcludeFolders(excludeFolders);
    config.setOnlyBasicIndexing(!m_enableContentIndexing->isChecked());

    if (m_previouslyEnabled != enabled) {
        config.setFirstRun(true);
    }

    // Start Baloo
    if (enabled) {
        const QString exe = QStandardPaths::findExecutable(QStringLiteral("baloo_file"));
        QProcess::startDetached(exe, QStringList());
    }
    else {
        QDBusMessage message = QDBusMessage::createMethodCall(QStringLiteral("org.kde.baloo"),
                                                              QStringLiteral("/"),
                                                              QStringLiteral("org.kde.baloo.main"),
                                                              QStringLiteral("quit"));

        QDBusConnection::sessionBus().asyncCall(message);
    }

    // Start cleaner
    const QString exe = QStandardPaths::findExecutable(QStringLiteral("baloo_file_cleaner"));
    QProcess::startDetached(exe, QStringList());

    // Update the baloo_file's config cache
    config.refresh();

    // all values saved -> no changes
    Q_EMIT changed(false);
}


void ServerConfigModule::defaults()
{
    m_excludeFolders_FSW->setDirectoryList(defaultFolders(), QStringList());
}

void ServerConfigModule::indexingEnabledChanged()
{
    m_enableContentIndexing->setEnabled(m_enableCheckbox->isChecked());
}

void ServerConfigModule::onDirectoryListChanged()
{
    m_enableCheckbox->setChecked(!allMountPointsExcluded());
}

bool ServerConfigModule::allMountPointsExcluded()
{
    QStringList mountPoints;
    Q_FOREACH (const QStorageInfo &si, QStorageInfo::mountedVolumes()) {
        mountPoints << si.rootPath();
    }

    const QStringList excluded = m_excludeFolders_FSW->excludeFolders();

    if (excluded.toSet() == mountPoints.toSet()) {
        return true;
    }

    return false;
}

#include "kcm.moc"
