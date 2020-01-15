/* This file is part of the KDE Project
   Copyright (c) 2014 Marco Martin <mart@kde.org>
   Copyright (c) 2014 Vishesh Handa <me@vhanda.in>
   Copyright (c) 2016 David Rosca <nowrep@gmail.com>
   Copyright (c) 2018 Kai Uwe Broulik <kde@privat.broulik.de>
   Copyright (c) 2019 Kevin Ottens <kevin.ottens@enioka.com>

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

#include <KPluginFactory>
#include <KAboutData>
#include <KLocalizedString>

#include <KIO/FileCopyJob>
#include <KIO/JobUiDelegate>

#include <Plasma/Theme>
#include <Plasma/Svg>

#include <QDebug>
#include <QProcess>
#include <QQuickItem>
#include <QQuickWindow>
#include <QStandardPaths>
#include <QStandardItemModel>
#include <QTemporaryFile>

#include "desktopthemesettings.h"
#include "filterproxymodel.h"
#include "themesmodel.h"

Q_LOGGING_CATEGORY(KCM_DESKTOP_THEME, "kcm_desktoptheme")

K_PLUGIN_FACTORY_WITH_JSON(KCMDesktopThemeFactory, "kcm_desktoptheme.json", registerPlugin<KCMDesktopTheme>();)

KCMDesktopTheme::KCMDesktopTheme(QObject *parent, const QVariantList &args)
    : KQuickAddons::ManagedConfigModule(parent, args)
    , m_settings(new DesktopThemeSettings(this))
    , m_model(new ThemesModel(this))
    , m_filteredModel(new FilterProxyModel(this))
    , m_haveThemeExplorerInstalled(false)
{
    qmlRegisterType<DesktopThemeSettings>();
    qmlRegisterUncreatableType<ThemesModel>("org.kde.private.kcms.desktoptheme", 1, 0, "ThemesModel", "Cannot create ThemesModel");
    qmlRegisterUncreatableType<FilterProxyModel>("org.kde.private.kcms.desktoptheme", 1, 0, "FilterProxyModel", "Cannot create FilterProxyModel");

    KAboutData* about = new KAboutData(QStringLiteral("kcm_desktoptheme"), i18n("Plasma Style"),
                                       QStringLiteral("0.1"), QString(), KAboutLicense::LGPL);
    about->addAuthor(i18n("David Rosca"), QString(), QStringLiteral("nowrep@gmail.com"));
    setAboutData(about);
    setButtons(Apply | Default | Help);

    m_haveThemeExplorerInstalled = !QStandardPaths::findExecutable(QStringLiteral("plasmathemeexplorer")).isEmpty();

    connect(m_model, &ThemesModel::pendingDeletionsChanged, this, &KCMDesktopTheme::settingsChanged);

    connect(m_model, &ThemesModel::selectedThemeChanged, this, [this](const QString &pluginName) {
        m_settings->setName(pluginName);
    });

    connect(m_settings, &DesktopThemeSettings::nameChanged, this, [this] {
        m_model->setSelectedTheme(m_settings->name());
    });

    connect(m_model, &ThemesModel::selectedThemeChanged, m_filteredModel, &FilterProxyModel::setSelectedTheme);

    m_filteredModel->setSourceModel(m_model);
}

KCMDesktopTheme::~KCMDesktopTheme()
{
}

DesktopThemeSettings *KCMDesktopTheme::desktopThemeSettings() const
{
    return m_settings;
}

ThemesModel *KCMDesktopTheme::desktopThemeModel() const
{
    return m_model;
}

FilterProxyModel *KCMDesktopTheme::filteredModel() const
{
    return m_filteredModel;
}

bool KCMDesktopTheme::downloadingFile() const
{
    return m_tempCopyJob;
}

void KCMDesktopTheme::installThemeFromFile(const QUrl &url)
{
    if (url.isLocalFile()) {
        installTheme(url.toLocalFile());
        return;
    }

    if (m_tempCopyJob) {
        return;
    }

    m_tempInstallFile.reset(new QTemporaryFile());
    if (!m_tempInstallFile->open()) {
        emit showErrorMessage(i18n("Unable to create a temporary file."));
        m_tempInstallFile.reset();
        return;
    }

    m_tempCopyJob = KIO::file_copy(url, QUrl::fromLocalFile(m_tempInstallFile->fileName()),
                                    -1, KIO::Overwrite);
    m_tempCopyJob->uiDelegate()->setAutoErrorHandlingEnabled(true);
    emit downloadingFileChanged();

    connect(m_tempCopyJob, &KIO::FileCopyJob::result, this, [this, url](KJob *job) {
        if (job->error() != KJob::NoError) {
            emit showErrorMessage(i18n("Unable to download the theme: %1", job->errorText()));
            return;
        }

        installTheme(m_tempInstallFile->fileName());
        m_tempInstallFile.reset();
    });
    connect(m_tempCopyJob, &QObject::destroyed, this, &KCMDesktopTheme::downloadingFileChanged);
}

void KCMDesktopTheme::installTheme(const QString &path)
{
    qCDebug(KCM_DESKTOP_THEME) << "Installing ... " << path;

    const QString program = QStringLiteral("kpackagetool5");
    const QStringList arguments = { QStringLiteral("--type"), QStringLiteral("Plasma/Theme"), QStringLiteral("--install"), path};

    qCDebug(KCM_DESKTOP_THEME) << program << arguments.join(QLatin1Char(' '));
    QProcess *myProcess = new QProcess(this);
    connect(myProcess, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
            this, [this](int exitCode, QProcess::ExitStatus exitStatus) {
                Q_UNUSED(exitStatus)
                if (exitCode == 0) {
                    emit showSuccessMessage(i18n("Theme installed successfully."));
                    load();
                } else {
                    Q_EMIT showErrorMessage(i18n("Theme installation failed."));

                }
            });

    connect(myProcess, static_cast<void (QProcess::*)(QProcess::ProcessError)>(&QProcess::error),
            this, [this](QProcess::ProcessError e) {
                qCWarning(KCM_DESKTOP_THEME) << "Theme installation failed: " << e;
                Q_EMIT showErrorMessage(i18n("Theme installation failed."));
            });

    myProcess->start(program, arguments);
}

void KCMDesktopTheme::applyPlasmaTheme(QQuickItem *item, const QString &themeName)
{
    if (!item) {
        return;
    }

    Plasma::Theme *theme = m_themes[themeName];
    if (!theme) {
        theme = new Plasma::Theme(themeName, this);
        m_themes[themeName] = theme;
    }

    Q_FOREACH (Plasma::Svg *svg, item->findChildren<Plasma::Svg*>()) {
        svg->setTheme(theme);
        svg->setUsingRenderingCache(false);
    }
}

void KCMDesktopTheme::load()
{
    ManagedConfigModule::load();
    m_model->load();
    m_model->setSelectedTheme(m_settings->name());
}

void KCMDesktopTheme::save()
{
    ManagedConfigModule::save();
    Plasma::Theme().setThemeName(m_settings->name());
    processPendingDeletions();
}

void KCMDesktopTheme::defaults()
{
    ManagedConfigModule::defaults();

    // can this be done more elegantly?
    const auto pendingDeletions = m_model->match(m_model->index(0, 0), ThemesModel::PendingDeletionRole, true);
    for (const QModelIndex &idx : pendingDeletions) {
        m_model->setData(idx, false, ThemesModel::PendingDeletionRole);
    }
}

bool KCMDesktopTheme::canEditThemes() const
{
    return m_haveThemeExplorerInstalled;
}

void KCMDesktopTheme::editTheme(const QString &theme)
{
    QProcess::startDetached(QStringLiteral("plasmathemeexplorer -t ") % theme);
}

bool KCMDesktopTheme::isSaveNeeded() const
{
    return !m_model->match(m_model->index(0, 0), ThemesModel::PendingDeletionRole, true).isEmpty();
}

void KCMDesktopTheme::processPendingDeletions()
{
    const QString program = QStringLiteral("plasmapkg2");

    const auto pendingDeletions = m_model->match(m_model->index(0, 0), ThemesModel::PendingDeletionRole, true, -1 /*all*/);
    QVector<QPersistentModelIndex> persistentPendingDeletions;
    // turn into persistent model index so we can delete as we go
    std::transform(pendingDeletions.begin(), pendingDeletions.end(),
                   std::back_inserter(persistentPendingDeletions), [](const QModelIndex &idx) {
        return QPersistentModelIndex(idx);
    });

    for (const QPersistentModelIndex &idx : persistentPendingDeletions) {
        const QString pluginName = idx.data(ThemesModel::PluginNameRole).toString();
        const QString displayName = idx.data(Qt::DisplayRole).toString();

        Q_ASSERT(pluginName != m_settings->name());

        const QStringList arguments = {QStringLiteral("-t"), QStringLiteral("theme"), QStringLiteral("-r"), pluginName};

        QProcess *process = new QProcess(this);
        connect(process, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), this,
            [this, process, idx, pluginName, displayName](int exitCode, QProcess::ExitStatus exitStatus) {
                Q_UNUSED(exitStatus)
                if (exitCode == 0) {
                    m_model->removeRow(idx.row());
                } else {
                    emit showErrorMessage(i18n("Removing theme failed: %1",
                                               QString::fromLocal8Bit(process->readAllStandardOutput().trimmed())));
                    m_model->setData(idx, false, ThemesModel::PendingDeletionRole);
                }
                process->deleteLater();
            });

        process->start(program, arguments);
        process->waitForFinished(); // needed so it deletes fine when "OK" is clicked and the dialog destroyed
    }
}

#include "kcm.moc"
