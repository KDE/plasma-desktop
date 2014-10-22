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
        QLatin1String("kcm_baloofile"), i18n("Configure File Search"),
        QLatin1String("0.1"), QString(), KAboutLicense::GPL,
        i18n("Copyright 2007-2010 Sebastian Trüg"));
    about->addAuthor(i18n("Sebastian Trüg"), QString(), QLatin1String("trueg@kde.org"));
    about->addAuthor(i18n("Vishesh Handa"), QString(), QLatin1String("vhanda@kde.org"));
    setAboutData(about);
    setButtons(Help | Apply | Default);

    setupUi(this);

    int pixelSize = style()->pixelMetric(QStyle::PM_LargeIconSize);
    const QPixmap pixmap = QIcon::fromTheme(QLatin1String("baloo")).pixmap(QSize(pixelSize, pixelSize));
    m_pixmapLabel->setPixmap(pixmap);

    connect(m_folderSelectionWidget, SIGNAL(changed()),
            this, SLOT(changed()));
    connect(m_folderSelectionWidget, SIGNAL(changed()),
            this, SLOT(folderSelectionChanged()));
    connect(m_enableCheckbox, SIGNAL(stateChanged(int)),
            this, SLOT(changed()));
}


ServerConfigModule::~ServerConfigModule()
{
}


void ServerConfigModule::load()
{
    Baloo::IndexerConfig config;

    m_previouslyEnabled = config.fileIndexingEnabled();
    m_enableCheckbox->setChecked(m_previouslyEnabled);

    QStringList includeFolders = config.includeFolders();
    QStringList excludeFolders = config.excludeFolders();
    m_folderSelectionWidget->setFolders(includeFolders, excludeFolders);

    // All values loaded -> no changes
    Q_EMIT changed(false);
}


void ServerConfigModule::save()
{
    QStringList includeFolders = m_folderSelectionWidget->includeFolders();
    QStringList excludeFolders = m_folderSelectionWidget->excludeFolders();

    bool mountPointsEx = m_folderSelectionWidget->allMountPointsExcluded();

    bool enabled = m_enableCheckbox->isChecked();
    if (mountPointsEx)
        enabled = false;

    Baloo::IndexerConfig config;
    config.setFileIndexingEnabled(enabled);
    config.setIncludeFolders(includeFolders);
    config.setExcludeFolders(excludeFolders);

    if (m_previouslyEnabled != enabled) {
        config.setFirstRun(true);
    }

    // Start Baloo
    if (enabled) {
        const QString exe = QStandardPaths::findExecutable(QLatin1String("baloo_file"));
        QProcess::startDetached(exe);
    }
    else {
        QDBusMessage message = QDBusMessage::createMethodCall(QLatin1String("org.kde.baloo"),
                                                              QLatin1String("/indexer"),
                                                              QLatin1String("org.kde.baloo"),
                                                              QLatin1String("quit"));

        QDBusConnection::sessionBus().asyncCall(message);
    }

    // Start cleaner
    const QString exe = QStandardPaths::findExecutable(QLatin1String("baloo_file_cleaner"));
    QProcess::startDetached(exe);

    // all values saved -> no changes
    Q_EMIT changed(false);
}


void ServerConfigModule::defaults()
{
    m_folderSelectionWidget->setFolders(defaultFolders(), QStringList());
}

void ServerConfigModule::folderSelectionChanged()
{
    const bool disabled = m_folderSelectionWidget->allMountPointsExcluded();
    m_enableCheckbox->setChecked(!disabled);
}

#include "kcm.moc"
