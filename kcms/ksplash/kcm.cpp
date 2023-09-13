/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2014 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2014 Vishesh Handa <me@vhanda.in>
    SPDX-FileCopyrightText: 2019 Cyril Rossi <cyril.rossi@enioka.com>
    SPDX-FileCopyrightText: 2023 ivan tkachenko <me@ratijas.tk>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "kcm.h"

#include <KLocalizedString>
#include <KPluginFactory>

#include <QDir>
#include <QProcess>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QStandardPaths>

#include <KPackage/PackageJob>
#include <KPackage/PackageLoader>
#include <KPackage/PackageStructure>

#include "splashscreendata.h"
#include "splashscreensettings.h"

K_PLUGIN_FACTORY_WITH_JSON(KCMSplashScreenFactory, "kcm_splashscreen.json", registerPlugin<KCMSplashScreen>(); registerPlugin<SplashScreenData>();)

const QLatin1String s_nonePluginName("None");

class SplashScreenSortModel : public QSortFilterProxyModel
{
    using QSortFilterProxyModel::QSortFilterProxyModel;
    bool lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const override
    {
        // The "None" entry should be the last item of the model, BUG: 451422
        if (source_left.data(KCMSplashScreen::PluginNameRole).toString() == s_nonePluginName) {
            return false;
        }
        if (source_right.data(KCMSplashScreen::PluginNameRole).toString() == s_nonePluginName) {
            return true;
        }
        return QSortFilterProxyModel::lessThan(source_left, source_right);
    }
};

KCMSplashScreen::KCMSplashScreen(QObject *parent, const KPluginMetaData &metaData)
    : KQuickManagedConfigModule(parent, metaData)
    , m_data(new SplashScreenData(this))
    , m_model(new QStandardItemModel(this))
{
    qmlRegisterAnonymousType<SplashScreenSettings>("org.kde.plasma.splash.kcm", 0);
    qmlRegisterAnonymousType<QStandardItemModel>("org.kde.plasma.splash.kcm", 0);

    setButtons(Help | Apply | Default);

    QHash<int, QByteArray> roles = m_model->roleNames();
    roles[PluginNameRole] = "pluginName";
    roles[ScreenshotRole] = "screenshot";
    roles[DescriptionRole] = "description";
    roles[UninstallableRole] = "uninstallable";
    roles[PendingDeletionRole] = "pendingDeletion";
    m_model->setItemRoleNames(roles);

    m_sortModel = new SplashScreenSortModel(this);
    m_sortModel->setSourceModel(m_model);
    m_sortModel->setSortLocaleAware(true);
    m_sortModel->setSortRole(Qt::DisplayRole);
    m_sortModel->setSortCaseSensitivity(Qt::CaseInsensitive);
    m_sortModel->setDynamicSortFilter(true);

    connect(m_model, &QAbstractItemModel::dataChanged, this, [this] {
        bool hasPendingDeletions = !pendingDeletions().isEmpty();
        setNeedsSave(m_data->settings()->isSaveNeeded() || hasPendingDeletions);
        setRepresentsDefaults(m_data->settings()->isDefaults() && !hasPendingDeletions);
    });
}

QList<KPackage::Package> KCMSplashScreen::availablePackages(const QString &component)
{
    QList<KPackage::Package> packages;
    QStringList paths;
    const QStringList dataPaths = QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation);

    for (const QString &path : dataPaths) {
        QDir dir(path + QStringLiteral("/plasma/look-and-feel"));
        paths << dir.entryList(QDir::AllDirs | QDir::NoDotAndDotDot);
    }

    for (const QString &path : std::as_const(paths)) {
        KPackage::Package pkg = KPackage::PackageLoader::self()->loadPackage(QStringLiteral("Plasma/LookAndFeel"));
        pkg.setPath(path);
        pkg.setFallbackPackage(KPackage::Package());
        if (component.isEmpty() || !pkg.filePath(component.toUtf8()).isEmpty()) {
            packages << pkg;
        }
    }

    return packages;
}

SplashScreenSettings *KCMSplashScreen::splashScreenSettings() const
{
    return m_data->settings();
}

QAbstractProxyModel *KCMSplashScreen::splashSortedModel() const
{
    return m_sortModel;
}

void KCMSplashScreen::ghnsEntryChanged(const KNSCore::Entry &entry)
{
    auto removeItemFromModel = [this](const QStringList &files) {
        if (!files.isEmpty()) {
            const QString guessedPluginId = QFileInfo(files.constFirst()).fileName();
            const int index = pluginIndex(guessedPluginId);
            if (index != -1) {
                m_model->removeRows(index, 1);
            }
        }
    };

    if (entry.status() == KNSCore::Entry::Deleted) {
        removeItemFromModel(entry.uninstalledFiles());
    } else if (entry.status() == KNSCore::Entry::Installed) {
        removeItemFromModel(entry.installedFiles());
        KPackage::Package pkg = KPackage::PackageLoader::self()->loadPackage(QStringLiteral("Plasma/LookAndFeel"));
        pkg.setPath(entry.installedFiles().constFirst());
        addKPackageToModel(pkg);
        m_sortModel->sort(Qt::DisplayRole);
    }
}

void KCMSplashScreen::addKPackageToModel(const KPackage::Package &pkg)
{
    const static QString writableLocation = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
    QStandardItem *row = new QStandardItem(pkg.metadata().name());
    row->setData(pkg.metadata().pluginId(), PluginNameRole);
    row->setData(pkg.fileUrl("previews", QStringLiteral("splash.png")), ScreenshotRole);
    row->setData(pkg.metadata().description(), DescriptionRole);
    row->setData(pkg.path().startsWith(writableLocation), UninstallableRole);
    row->setData(false, PendingDeletionRole);
    m_packageRoot = writableLocation + QLatin1Char('/') + pkg.defaultPackageRoot();
    m_model->appendRow(row);
}

void KCMSplashScreen::load()
{
    m_data->settings()->load();
    m_model->clear();

    const QList<KPackage::Package> pkgs = availablePackages(QStringLiteral("splashmainscript"));
    for (const KPackage::Package &pkg : pkgs) {
        addKPackageToModel(pkg);
    }
    m_sortModel->sort(Qt::DisplayRole);

    QStandardItem *row = new QStandardItem(i18n("None"));
    row->setData(s_nonePluginName, PluginNameRole);
    row->setData(QUrl(), ScreenshotRole);
    row->setData(i18n("No splash screen will be shown"), DescriptionRole);
    row->setData(false, UninstallableRole);
    row->setData(false, PendingDeletionRole);
    m_model->insertRow(0, row);

    if (-1 == pluginIndex(m_data->settings()->theme())) {
        defaults();
    }

    Q_EMIT m_data->settings()->themeChanged();
}

void KCMSplashScreen::save()
{
    using namespace KPackage;
    const QStringList pendingDeletionPlugins = pendingDeletions();
    for (const QString &plugin : pendingDeletionPlugins) {
        if (plugin == m_data->settings()->theme()) {
            Q_EMIT error(i18n("You cannot delete the currently selected splash screen"));
            m_model->setData(m_model->index(pluginIndex(plugin), 0), false, Roles::PendingDeletionRole);
            continue;
        }

        KJob *uninstallJob = KPackage::PackageJob::uninstall(QStringLiteral("Plasma/LookAndFeel"), plugin, m_packageRoot);
        connect(uninstallJob, &KJob::result, this, [this, uninstallJob, plugin]() {
            if (uninstallJob->error()) {
                Q_EMIT error(uninstallJob->errorString());
            } else {
                m_model->removeRows(pluginIndex(plugin), 1);
            }
        });
    }
    m_data->settings()->setEngine(m_data->settings()->theme() == QStringLiteral("None") ? QStringLiteral("none") : QStringLiteral("KSplashQML"));
    KQuickManagedConfigModule::save();
}

int KCMSplashScreen::sortModelPluginIndex(const QString &pluginName) const
{
    return m_sortModel->mapFromSource(m_model->index(pluginIndex(pluginName), 0)).row();
}

int KCMSplashScreen::pluginIndex(const QString &pluginName) const
{
    const auto results = m_model->match(m_model->index(0, 0), PluginNameRole, pluginName, 1, Qt::MatchExactly);
    if (results.count() == 1) {
        return results.first().row();
    }

    return -1;
}

bool KCMSplashScreen::testing() const
{
    return m_testProcess;
}

void KCMSplashScreen::test(const QString &plugin)
{
    if (plugin.isEmpty() || plugin == QLatin1String("None") || m_testProcess) {
        return;
    }

    m_testProcess = new QProcess(this);
    connect(m_testProcess, &QProcess::errorOccurred, this, [this](QProcess::ProcessError error) {
        Q_UNUSED(error)
        Q_EMIT testingFailed(QString::fromLocal8Bit(m_testProcess->readAllStandardError()));
    });
    connect(m_testProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, [this](int exitCode, QProcess::ExitStatus exitStatus) {
        Q_UNUSED(exitCode)
        Q_UNUSED(exitStatus)

        m_testProcess->deleteLater();
        m_testProcess = nullptr;
        Q_EMIT testingChanged();
    });

    Q_EMIT testingChanged();
    m_testProcess->start(QStringLiteral("ksplashqml"), {plugin, QStringLiteral("--test")});
}

QStringList KCMSplashScreen::pendingDeletions()
{
    QStringList pendingDeletions;
    for (int i = 0, count = m_model->rowCount(); i < count; ++i) {
        if (m_model->item(i)->data(Roles::PendingDeletionRole).toBool()) {
            pendingDeletions << m_model->item(i)->data(Roles::PluginNameRole).toString();
        }
    }
    return pendingDeletions;
}

void KCMSplashScreen::defaults()
{
    KQuickManagedConfigModule::defaults();
    // Make sure we clear all pending deletions
    for (int i = 0, count = m_model->rowCount(); i < count; ++i) {
        m_model->item(i)->setData(false, Roles::PendingDeletionRole);
    }
}

#include "kcm.moc"
#include "moc_kcm.cpp"
