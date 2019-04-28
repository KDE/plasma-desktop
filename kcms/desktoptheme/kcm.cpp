/* This file is part of the KDE Project
   Copyright (c) 2014 Marco Martin <mart@kde.org>
   Copyright (c) 2014 Vishesh Handa <me@vhanda.in>
   Copyright (c) 2016 David Rosca <nowrep@gmail.com>
   Copyright (c) 2018 Kai Uwe Broulik <kde@privat.broulik.de>

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
#include <KSharedConfig>
#include <KLocalizedString>
#include <KDesktopFile>

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

#include <KNewStuff3/KNS3/DownloadDialog>

Q_LOGGING_CATEGORY(KCM_DESKTOP_THEME, "kcm_desktoptheme")

K_PLUGIN_FACTORY_WITH_JSON(KCMDesktopThemeFactory, "kcm_desktoptheme.json", registerPlugin<KCMDesktopTheme>();)

KCMDesktopTheme::KCMDesktopTheme(QObject *parent, const QVariantList &args)
    : KQuickAddons::ConfigModule(parent, args)
    , m_defaultTheme(new Plasma::Theme(this))
    , m_haveThemeExplorerInstalled(false)
{
    //This flag seems to be needed in order for QQuickWidget to work
    //see https://bugreports.qt-project.org/browse/QTBUG-40765
    //also, it seems to work only if set in the kcm, not in the systemsettings' main
    qApp->setAttribute(Qt::AA_DontCreateNativeWidgetSiblings);
    qmlRegisterType<QStandardItemModel>();

    KAboutData* about = new KAboutData(QStringLiteral("kcm_desktoptheme"), i18n("Plasma Theme"),
                                       QStringLiteral("0.1"), QString(), KAboutLicense::LGPL);
    about->addAuthor(i18n("David Rosca"), QString(), QStringLiteral("nowrep@gmail.com"));
    setAboutData(about);
    setButtons(Apply | Default | Help);

    m_model = new QStandardItemModel(this);
    QHash<int, QByteArray> roles = m_model->roleNames();
    roles[PluginNameRole] = QByteArrayLiteral("pluginName");
    roles[ThemeNameRole] = QByteArrayLiteral("themeName");
    roles[DescriptionRole] = QByteArrayLiteral("description");
    roles[IsLocalRole] = QByteArrayLiteral("isLocal");
    roles[PendingDeletionRole] = QByteArrayLiteral("pendingDeletion");
    m_model->setItemRoleNames(roles);

    m_haveThemeExplorerInstalled = !QStandardPaths::findExecutable(QStringLiteral("plasmathemeexplorer")).isEmpty();
}

KCMDesktopTheme::~KCMDesktopTheme()
{
    delete m_defaultTheme;
}

QStandardItemModel *KCMDesktopTheme::desktopThemeModel() const
{
    return m_model;
}

QString KCMDesktopTheme::selectedPlugin() const
{
    return m_selectedPlugin;
}

void KCMDesktopTheme::setSelectedPlugin(const QString &plugin)
{
    if (m_selectedPlugin == plugin) {
        return;
    }
    m_selectedPlugin = plugin;
    emit selectedPluginChanged(m_selectedPlugin);
    emit selectedPluginIndexChanged();
    updateNeedsSave();
}

int KCMDesktopTheme::selectedPluginIndex() const
{
    const auto results = m_model->match(m_model->index(0, 0), PluginNameRole, m_selectedPlugin);
    if (results.count() == 1) {
        return results.first().row();
    }

    return -1;
}

bool KCMDesktopTheme::downloadingFile() const
{
    return m_tempCopyJob;
}

void KCMDesktopTheme::setPendingDeletion(int index, bool pending)
{
    QModelIndex idx = m_model->index(index, 0);

    m_model->setData(idx, pending, PendingDeletionRole);

    if (pending && selectedPluginIndex() == index) {
        // move to the next non-pending theme
        const auto nonPending = m_model->match(idx, PendingDeletionRole, false);
        setSelectedPlugin(nonPending.first().data(PluginNameRole).toString());
    }

    updateNeedsSave();
}

void KCMDesktopTheme::getNewStuff(QQuickItem *ctx)
{
    if (!m_newStuffDialog) {
        m_newStuffDialog = new KNS3::DownloadDialog(QStringLiteral("plasma-themes.knsrc"));
        m_newStuffDialog.data()->setWindowTitle(i18n("Download New Plasma Themes"));
        m_newStuffDialog->setWindowModality(Qt::WindowModal);
        m_newStuffDialog->winId(); // so it creates the windowHandle();
        connect(m_newStuffDialog.data(), &KNS3::DownloadDialog::accepted, this, &KCMDesktopTheme::load);
    }

    if (ctx && ctx->window()) {
        m_newStuffDialog->windowHandle()->setTransientParent(ctx->window());
    }

    m_newStuffDialog.data()->show();
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

    qCDebug(KCM_DESKTOP_THEME) << program << arguments.join(QStringLiteral(" "));
    QProcess *myProcess = new QProcess(this);
    connect(myProcess, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
            this, [this, myProcess](int exitCode, QProcess::ExitStatus exitStatus) {
                Q_UNUSED(exitStatus);
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
    m_pendingRemoval.clear();

    // Get all desktop themes
    QStringList themes;
    const QStringList &packs = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, QStringLiteral("plasma/desktoptheme"), QStandardPaths::LocateDirectory);
    Q_FOREACH (const QString &ppath, packs) {
        const QDir cd(ppath);
        const QStringList &entries = cd.entryList(QDir::Dirs | QDir::Hidden | QDir::NoDotAndDotDot);
        Q_FOREACH (const QString &pack, entries) {
            const QString _metadata = ppath + QLatin1Char('/') + pack + QStringLiteral("/metadata.desktop");
            if (QFile::exists(_metadata)) {
                themes << _metadata;
            }
        }
    }

    m_model->clear();

    Q_FOREACH (const QString &theme, themes) {
        int themeSepIndex = theme.lastIndexOf(QLatin1Char('/'), -1);
        const QString themeRoot = theme.left(themeSepIndex);
        int themeNameSepIndex = themeRoot.lastIndexOf(QLatin1Char('/'), -1);
        const QString packageName = themeRoot.right(themeRoot.length() - themeNameSepIndex - 1);

        KDesktopFile df(theme);

        if (df.noDisplay()) {
            continue;
        }

        QString name = df.readName();
        if (name.isEmpty()) {
            name = packageName;
        }
        const bool isLocal = QFileInfo(theme).isWritable();

        if (m_model->findItems(packageName).isEmpty()) {
            QStandardItem *item = new QStandardItem;
            item->setText(packageName);
            item->setData(packageName, PluginNameRole);
            item->setData(name, ThemeNameRole);
            item->setData(df.readComment(), DescriptionRole);
            item->setData(isLocal, IsLocalRole);
            item->setData(false, PendingDeletionRole);
            m_model->appendRow(item);
        }
    }

    m_model->setSortRole(ThemeNameRole); // FIXME the model should really be just using Qt::DisplayRole
    m_model->sort(0 /*column*/);

    KConfigGroup cg(KSharedConfig::openConfig(QStringLiteral("plasmarc")), "Theme");
    setSelectedPlugin(cg.readEntry("name", m_defaultTheme->themeName()));

    emit selectedPluginIndexChanged();

    updateNeedsSave();
}

void KCMDesktopTheme::save()
{
    if (m_defaultTheme->themeName() != m_selectedPlugin) {
        m_defaultTheme->setThemeName(m_selectedPlugin);
    }

    processPendingDeletions();
    updateNeedsSave();
}

void KCMDesktopTheme::defaults()
{
    setSelectedPlugin(QStringLiteral("default"));

    // can this be done more elegantly?
    const auto pendingDeletions = m_model->match(m_model->index(0, 0), PendingDeletionRole, true);
    for (const QModelIndex &idx : pendingDeletions) {
        m_model->setData(idx, false, PendingDeletionRole);
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

void KCMDesktopTheme::updateNeedsSave()
{
    setNeedsSave(!m_model->match(m_model->index(0, 0), PendingDeletionRole, true).isEmpty()
                    || m_selectedPlugin != m_defaultTheme->themeName());
}

void KCMDesktopTheme::processPendingDeletions()
{
    const QString program = QStringLiteral("plasmapkg2");

    const auto pendingDeletions = m_model->match(m_model->index(0, 0), PendingDeletionRole, true, -1 /*all*/);
    QVector<QPersistentModelIndex> persistentPendingDeletions;
    // turn into persistent model index so we can delete as we go
    std::transform(pendingDeletions.begin(), pendingDeletions.end(),
                   std::back_inserter(persistentPendingDeletions), [](const QModelIndex &idx) {
        return QPersistentModelIndex(idx);
    });

    for (const QPersistentModelIndex &idx : persistentPendingDeletions) {
        const QString pluginName = idx.data(PluginNameRole).toString();
        const QString displayName = idx.data(Qt::DisplayRole).toString();

        Q_ASSERT(pluginName != m_selectedPlugin);

        const QStringList arguments = {QStringLiteral("-t"), QStringLiteral("theme"), QStringLiteral("-r"), pluginName};

        QProcess *process = new QProcess(this);
        connect(process, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), this,
            [this, process, idx, pluginName, displayName](int exitCode, QProcess::ExitStatus exitStatus) {
                Q_UNUSED(exitStatus);
                if (exitCode == 0) {
                    m_model->removeRow(idx.row());
                } else {
                    emit showErrorMessage(i18n("Removing theme failed: %1",
                                               QString::fromLocal8Bit(process->readAllStandardOutput().trimmed())));
                    m_model->setData(idx, false, PendingDeletionRole);
                }
                process->deleteLater();
            });

        process->start(program, arguments);
        process->waitForFinished(); // needed so it deletes fine when "OK" is clicked and the dialog destroyed
    }
}

#include "kcm.moc"
