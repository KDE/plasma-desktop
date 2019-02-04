/*
 * main.cpp
 *
 * Copyright (c) 1999 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
 * Copyright (c) 2000 Antonio Larrosa <larrosa@kde.org>
 * Copyright (C) 2000 Geert Jansen <jansen@kde.org>
 * KDE Frameworks 5 port Copyright (C) 2013 Jonathan Riddell <jr@jriddell.org>
 * Copyright (C) 2018 Kai Uwe Broulik <kde@privat.broulik.de>
 *
 * Requires the Qt widget libraries, available at no cost at
 * http://www.troll.no/
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "main.h"

#include <QDBusConnection>
#include <QDBusMessage>
#include <QGuiApplication>
#include <QPainter>
#include <QPixmapCache>
#include <QProcess>
#include <QQuickItem>
#include <QQuickWindow>
#include <QStringList>
#include <QSvgRenderer>

#include <KAboutData>
#include <KConfigGroup>
#include <Kdelibs4Migration>
#include <KIconLoader>
#include <KIconTheme>
#include <KJobUiDelegate>
#include <KLocalizedString>
#include <KSharedConfig>
#include <KPluginFactory>
#include <KTar>

#include <KIO/DeleteJob>
#include <KIO/FileCopyJob>

#include <algorithm>
#include <unistd.h> // for unlink

#include "iconsmodel.h"

#include "config.h" // for CMAKE_INSTALL_FULL_LIBEXECDIR

static const QVector<int> s_defaultIconSizes = { 32, 22, 22, 16, 48, 32 };
// we try to use KIconTheme::defaultThemeName() but that could be "hicolor" which isn't a "real" theme
static const QString s_defaultThemeName = QStringLiteral("breeze");

K_PLUGIN_FACTORY_WITH_JSON(IconsFactory, "kcm_icons.json", registerPlugin<IconModule>();)

IconModule::IconModule(QObject *parent, const QVariantList &args)
    : KQuickAddons::ConfigModule(parent, args)
    , m_model(new IconsModel(this))
    , m_iconGroups{
        QStringLiteral("Desktop"),
        QStringLiteral("Toolbar"),
        QStringLiteral("MainToolbar"),
        QStringLiteral("Small"),
        QStringLiteral("Panel"),
        QStringLiteral("Dialog")
    }
{
    qmlRegisterType<IconsModel>();

    // to be able to access its enums
    qmlRegisterUncreatableType<KIconLoader>("org.kde.private.kcms.icons", 1, 0, "KIconLoader", QString());

    KAboutData* about = new KAboutData(QStringLiteral("kcm5_icons"), i18n("Icons"), QStringLiteral("1.0"),
                                       i18n("Icons Control Panel Module"), KAboutLicense::GPL,
                                       i18n("(c) 2000-2003 Geert Jansen"));
    about->addAuthor(i18n("Geert Jansen"), QString(), QStringLiteral("jansen@kde.org"));
    about->addAuthor(i18n("Antonio Larrosa Jimenez"), QString(), QStringLiteral("larrosa@kde.org"));
    about->addCredit(i18n("Torsten Rahn"), QString(), QStringLiteral("torsten@kde.org"));
    about->addAuthor(i18n("Jonathan Riddell"), QString(), QStringLiteral("jr@jriddell.org"));
    about->addAuthor(i18n("Kai Uwe Broulik"), QString(), QStringLiteral("kde@privat.broulik.de>"));
    setAboutData(about);

    setButtons(Apply | Default);

    connect(m_model, &IconsModel::selectedThemeChanged, this, [this] {
        m_selectedThemeDirty = true;
        setNeedsSave(true);
    });
    connect(m_model, &IconsModel::pendingDeletionsChanged, this, [this] {
        setNeedsSave(true);
    });

    // When user has a lot of themes installed, preview pixmaps might get evicted prematurely
    QPixmapCache::setCacheLimit(50 * 1024); // 50 MiB
}

IconModule::~IconModule()
{

}

IconsModel *IconModule::iconsModel() const
{
    return m_model;
}

QStringList IconModule::iconGroups() const
{
    return m_iconGroups;
}

bool IconModule::downloadingFile() const
{
    return m_tempCopyJob;
}

int IconModule::iconSize(int group) const
{
    return m_iconSizes[group];
}

void IconModule::setIconSize(int group, int size)
{
    if (iconSize(group) == size) {
        return;
    }

    m_iconSizes[group] = size;
    setNeedsSave(true);
    m_iconSizesDirty = true;
    emit iconSizesChanged();
}

QList<int> IconModule::availableIconSizes(int group) const
{
    return KIconLoader::global()->theme()->querySizes(static_cast<KIconLoader::Group>(group));
}

void IconModule::load()
{
    m_model->load();
    loadIconSizes();
    m_model->setSelectedTheme(KIconTheme::current());
    setNeedsSave(false);
    m_selectedThemeDirty = false;
    m_iconSizesDirty = false;
}

void IconModule::save()
{
    if (m_selectedThemeDirty) {
        QProcess::startDetached(CMAKE_INSTALL_FULL_LIBEXECDIR "/plasma-changeicons", {m_model->selectedTheme()});
    }

    if (m_iconSizesDirty || m_revertIconEffects) {
        auto cfg = KSharedConfig::openConfig();
        for (int i = 0; i < m_iconGroups.count(); ++i) {
            const QString &group = m_iconGroups.at(i);
            KConfigGroup cg(cfg, group + QLatin1String("Icons"));
            cg.writeEntry("Size", m_iconSizes.at(i), KConfig::Normal | KConfig::Global);

            if (m_revertIconEffects) {
                cg.revertToDefault("Animated");

                const QStringList states = {
                    QStringLiteral("Default"),
                    QStringLiteral("Active"),
                    QStringLiteral("Disabled")
                };

                const QStringList keys = {
                    QStringLiteral("Effect"),
                    QStringLiteral("Value"),
                    QStringLiteral("Color"),
                    QStringLiteral("Color2"),
                    QStringLiteral("SemiTransparent")
                };

                for (const QString &state : states) {
                    for (const QString &key : keys) {
                        cg.revertToDefault(state + key);
                    }
                }
            }
        }
        cfg->sync();
    }

    if (m_selectedThemeDirty || m_iconSizesDirty || m_revertIconEffects) {
        exportToKDE4();
    }

    processPendingDeletions();

    KIconLoader::global()->newIconLoader();

    setNeedsSave(false);
    m_selectedThemeDirty = false;
    m_iconSizesDirty = false;
    m_revertIconEffects = false;
}

void IconModule::processPendingDeletions()
{
    const QStringList pendingDeletions = m_model->pendingDeletions();

    for (const QString &themeName : pendingDeletions) {
        Q_ASSERT(themeName != m_model->selectedTheme());

        KIconTheme theme(themeName);
        auto *job = KIO::del(QUrl::fromLocalFile(theme.dir()), KIO::HideProgressInfo);
        // needs to block for it to work on "OK" where the dialog (kcmshell) closes
        job->exec();
    }

    m_model->removeItemsPendingDeletion();
}

void IconModule::defaults()
{
    if (m_iconSizes != s_defaultIconSizes) {
        m_iconSizes = s_defaultIconSizes;
        emit iconSizesChanged();
    }

    auto setThemeIfAvailable = [this](const QString &themeName) {
        const auto results = m_model->match(m_model->index(0, 0), ThemeNameRole, themeName);
        if (results.isEmpty()) {
            return false;
        }

        m_model->setSelectedTheme(themeName);
        return true;
    };

    if (!setThemeIfAvailable(KIconTheme::defaultThemeName())) {
        setThemeIfAvailable(QStringLiteral("breeze"));
    }

    m_revertIconEffects = true;
    setNeedsSave(true);
}

void IconModule::loadIconSizes()
{
    auto cfg = KSharedConfig::openConfig();

    QVector<int> iconSizes(6, 0); // why doesn't KIconLoader::LastGroup - 1 work here?!

    int i = KIconLoader::FirstGroup;
    for (const QString &group : qAsConst(m_iconGroups)) {
        int size = KIconLoader::global()->theme()->defaultSize(static_cast<KIconLoader::Group>(i));

        KConfigGroup iconGroup(cfg, group + QLatin1String("Icons"));
        size = iconGroup.readEntry("Size", size);

        iconSizes[i] = size;

        ++i;
    }

    if (m_iconSizes != iconSizes) {
        m_iconSizes = iconSizes;
        emit iconSizesChanged();
    }
}

void IconModule::getNewStuff(QQuickItem *ctx)
{
    if (!m_newStuffDialog) {
        m_newStuffDialog = new KNS3::DownloadDialog(QStringLiteral("icons.knsrc"));
        m_newStuffDialog->setWindowTitle(i18n("Download New Icon Themes"));
        m_newStuffDialog->setWindowModality(Qt::WindowModal);
        m_newStuffDialog->winId(); // so it creates the windowHandle();
        // TODO would be lovely to scroll to and select the newly installed scheme, if any
        connect(m_newStuffDialog.data(), &KNS3::DownloadDialog::accepted, this, [this] {
            if (m_newStuffDialog->changedEntries().isEmpty()) {
                return;
            }

            // reload the display icontheme items
            KIconLoader::global()->newIconLoader();
            m_model->load();
            QPixmapCache::clear();
        });
    }

    if (ctx && ctx->window()) {
        m_newStuffDialog->windowHandle()->setTransientParent(ctx->window());
    }

    m_newStuffDialog->show();
}

void IconModule::installThemeFromFile(const QUrl &url)
{
    if (url.isLocalFile()) {
        installThemeFile(url.toLocalFile());
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

    m_tempCopyJob = KIO::file_copy(url,QUrl::fromLocalFile(m_tempInstallFile->fileName()),
                                           -1, KIO::Overwrite);
    m_tempCopyJob->uiDelegate()->setAutoErrorHandlingEnabled(true);
    emit downloadingFileChanged();

    connect(m_tempCopyJob, &KIO::FileCopyJob::result, this, [this, url](KJob *job) {
        if (job->error() != KJob::NoError) {
            emit showErrorMessage(i18n("Unable to download the icon theme archive: %1", job->errorText()));
            return;
        }

        installThemeFile(m_tempInstallFile->fileName());
        m_tempInstallFile.reset();
    });
    connect(m_tempCopyJob, &QObject::destroyed, this, &IconModule::downloadingFileChanged);
}

void IconModule::installThemeFile(const QString &path)
{
    const QStringList themesNames = findThemeDirs(path);
    if (themesNames.isEmpty()) {
        emit showErrorMessage(i18n("The file is not a valid icon theme archive."));
        return;
    }

    if (!installThemes(themesNames, path)) {
        emit showErrorMessage(i18n("A problem occurred during the installation process; however, most of the themes in the archive have been installed"));
        return;
    }

    emit showSuccessMessage(i18n("Theme installed successfully."));

    KIconLoader::global()->newIconLoader();
    m_model->load();
}

void IconModule::exportToKDE4()
{
    //TODO: killing the kde4 icon cache: possible? (kde4migration doesn't let access the cache folder)
    Kdelibs4Migration migration;
    QString configFilePath = migration.saveLocation("config");
    if (configFilePath.isEmpty()) {
        return;
    }

    configFilePath += QLatin1String("kdeglobals");

    KSharedConfigPtr kglobalcfg = KSharedConfig::openConfig(QStringLiteral("kdeglobals"));
    KConfig kde4config(configFilePath, KConfig::SimpleConfig);

    KConfigGroup kde4IconGroup(&kde4config, "Icons");
    kde4IconGroup.writeEntry("Theme", m_model->selectedTheme());

    //Synchronize icon effects
    for (const QString &group : qAsConst(m_iconGroups)) {
        const QString groupName = group + QLatin1String("Icons");
        KConfigGroup cg(kglobalcfg, groupName);
        KConfigGroup kde4Cg(&kde4config, groupName);

        // HACK copyTo only copies keys, it doesn't replace the entire group
        // which means if we removed the effects in our config it won't remove
        // them from the kde4 config, hence revert all of them prior to copying
        const QStringList keys =  cg.keyList() + kde4Cg.keyList();
        for (const QString &key : keys) {
            kde4Cg.revertToDefault(key);
        }
        // now copy over the new values
        cg.copyTo(&kde4Cg);
    }

    kde4config.sync();

    QProcess *cachePathProcess = new QProcess(this);
    connect(cachePathProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this,
            [cachePathProcess](int exitCode, QProcess::ExitStatus status) {

        if (status == QProcess::NormalExit && exitCode == 0) {
            QString path = cachePathProcess->readAllStandardOutput().trimmed();
            path.append(QLatin1String("icon-cache.kcache"));
            QFile::remove(path);
        }

        //message kde4 apps that icon theme is changed
        for (int i = 0; i < KIconLoader::LastGroup; i++) {
            KIconLoader::emitChange(KIconLoader::Group(i));

            QDBusMessage message = QDBusMessage::createSignal(QStringLiteral("/KGlobalSettings"),
                                                              QStringLiteral("org.kde.KGlobalSettings"),
                                                              QStringLiteral("notifyChange"));
            message.setArguments({
                4, // KGlobalSettings::IconChanged
                KIconLoader::Group(i)
            });
            QDBusConnection::sessionBus().send(message);
        }

        cachePathProcess->deleteLater();
    });
    cachePathProcess->start(QStringLiteral("kde4-config --path cache"));
}

QStringList IconModule::findThemeDirs(const QString &archiveName)
{
    QStringList foundThemes;

    KTar archive(archiveName);
    archive.open(QIODevice::ReadOnly);
    const KArchiveDirectory *themeDir = archive.directory();

    KArchiveEntry *possibleDir = nullptr;
    KArchiveDirectory *subDir = nullptr;

    // iterate all the dirs looking for an index.theme or index.desktop file
    const QStringList entries = themeDir->entries();
    for (const QString &entry : entries) {
        possibleDir = const_cast<KArchiveEntry*>(themeDir->entry(entry));
        if (!possibleDir->isDirectory()) {
            continue;
        }

        subDir = dynamic_cast<KArchiveDirectory *>(possibleDir);
        if (!subDir) {
            continue;
        }

        if (subDir->entry(QStringLiteral("index.theme"))
                || subDir->entry(QStringLiteral("index.desktop"))) {
          foundThemes.append(subDir->name());
        }
    }

    archive.close();
    return foundThemes;
}

bool IconModule::installThemes(const QStringList &themes, const QString &archiveName)
{
    bool everythingOk = true;
    const QString localThemesDir(QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1String("/icons/./"));

    emit showProgress(i18n("Installing icon themes..."));

    KTar archive(archiveName);
    archive.open(QIODevice::ReadOnly);
    qApp->processEvents(QEventLoop::ExcludeUserInputEvents);

    const KArchiveDirectory *rootDir = archive.directory();

    KArchiveDirectory *currentTheme = nullptr;
    for (const QString &theme : themes) {
        emit showProgress(i18n("Installing %1 theme...", theme));

        qApp->processEvents(QEventLoop::ExcludeUserInputEvents);

        currentTheme = dynamic_cast<KArchiveDirectory*>(const_cast<KArchiveEntry*>(rootDir->entry(theme)));
        if (!currentTheme) {
            // we tell back that something went wrong, but try to install as much
            // as possible
            everythingOk = false;
            continue;
        }

        currentTheme->copyTo(localThemesDir + theme);
    }

    archive.close();

    emit hideProgress();
    return everythingOk;
}

QVariantList IconModule::previewIcons(const QString &themeName, int size, qreal dpr, int limit)
{
    static QVector<QStringList> s_previewIcons{
        {QStringLiteral("system-run"), QStringLiteral("exec")},
        {QStringLiteral("folder")},
        {QStringLiteral("document"), QStringLiteral("text-x-generic")},
        {QStringLiteral("user-trash"), QStringLiteral("user-trash-empty")},
        {QStringLiteral("system-help"), QStringLiteral("help-about"), QStringLiteral("help-contents")},
        {QStringLiteral("preferences-system"), QStringLiteral("systemsettings"), QStringLiteral("configure")},

        {QStringLiteral("text-html")},
        {QStringLiteral("image-x-generic"), QStringLiteral("image-png"), QStringLiteral("image-jpeg")},
        {QStringLiteral("video-x-generic"), QStringLiteral("video-x-theora+ogg"), QStringLiteral("video-mp4")},
        {QStringLiteral("x-office-document")},
        {QStringLiteral("x-office-spreadsheet")},
        {QStringLiteral("x-office-presentation"), QStringLiteral("application-presentation")},

        {QStringLiteral("user-home")},
        {QStringLiteral("user-desktop"), QStringLiteral("desktop")},
        {QStringLiteral("folder-image"), QStringLiteral("folder-images"), QStringLiteral("folder-pictures"), QStringLiteral("folder-picture")},
        {QStringLiteral("folder-documents")},
        {QStringLiteral("folder-download"), QStringLiteral("folder-downloads")},
        {QStringLiteral("folder-video"), QStringLiteral("folder-videos")}
    };

    // created on-demand as it is quite expensive to do and we don't want to do it every loop iteration either
    QScopedPointer<KIconTheme> theme;

    QVariantList pixmaps;

    for (const QStringList &iconNames : s_previewIcons) {
        const QString cacheKey = themeName + QLatin1Char('@') + QString::number(size) + QLatin1Char('@')
            + QString::number(dpr,'f',1) + QLatin1Char('@') + iconNames.join(QLatin1Char(','));

        QPixmap pix;
        if (!QPixmapCache::find(cacheKey, pix)) {
            if (!theme) {
                theme.reset(new KIconTheme(themeName));
            }

            pix = getBestIcon(*theme.data(), iconNames, size, dpr);

            // Inserting a pixmap even if null so we know whether we searched for it already
            QPixmapCache::insert(cacheKey, pix);
        }

        if (pix.isNull()) {
            continue;
        }

        pixmaps.append(pix);

        if (limit > -1 && pixmaps.count() >= limit) {
            break;
        }
    }

    return pixmaps;
}

QPixmap IconModule::getBestIcon(KIconTheme &theme, const QStringList &iconNames, int size, qreal dpr)
{
    QSvgRenderer renderer;

    const int iconSize = size * dpr;

    // not using initializer list as we want to unwrap inherits()
    const QStringList themes = QStringList() << theme.internalName() << theme.inherits();
    for (const QString &themeName : themes) {
        KIconTheme theme(themeName);

        for (const QString &iconName : iconNames) {
            QString path = theme.iconPath(QStringLiteral("%1.png").arg(iconName), iconSize, KIconLoader::MatchBest);
            if (!path.isEmpty()) {
                QPixmap pixmap(path);
                pixmap.setDevicePixelRatio(dpr);
                return pixmap;
            }

            //could not find the .png, try loading the .svg or .svgz
            path = theme.iconPath(QStringLiteral("%1.svg").arg(iconName), iconSize, KIconLoader::MatchBest);
            if (path.isEmpty()) {
                path = theme.iconPath(QStringLiteral("%1.svgz").arg(iconName), iconSize, KIconLoader::MatchBest);
            }

            if (path.isEmpty()) {
                continue;
            }

            if (!renderer.load(path)) {
                continue;
            }

            QPixmap pixmap(iconSize, iconSize);
            pixmap.setDevicePixelRatio(dpr);
            pixmap.fill(QColor(Qt::transparent));
            QPainter p(&pixmap);
            p.setViewport(0, 0, size, size);
            renderer.render(&p);
            return pixmap;
        }
    }

    return QPixmap();
}

#include "main.moc"
