/*
 *  Copyright (C) 2021 Marco Martin <mart@kde.org>
 *  Copyright (C) 2018 <furkantokac34@gmail.com>
 *  Copyright (c) 2019 Cyril Rossi <cyril.rossi@enioka.com>
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

#include "landingpage.h"

#include <KPluginFactory>
#include <KAboutData>
#include <KColorScheme>
#include <KLocalizedString>
#include <KGlobalSettings>
#include <KPackage/PackageLoader>
#include <KService>
#include <KCModuleInfo>

#include <QDBusMessage>
#include <QDBusConnection>
#include <QDBusPendingCall>
#include <QQuickItem>
#include <QQuickWindow>
#include <QQuickRenderControl>
#include <QScreen>
#include <QStandardItemModel>
#include <QGuiApplication>

#include "landingpagedata.h"
#include "landingpage_kdeglobalssettings.h"
#include "landingpage_baloosettings.h"

#include <KActivities/Stats/ResultModel>
#include <KActivities/Stats/ResultSet>
#include <KActivities/Stats/Terms>

namespace KAStats = KActivities::Stats;

using namespace KAStats;
using namespace KAStats::Terms;



K_PLUGIN_FACTORY_WITH_JSON(KCMLandingPageFactory, "kcm_landingpage.json", registerPlugin<KCMLandingPage>(); registerPlugin<LandingPageData>();)



MostUsedModel::MostUsedModel(QObject *parent)
    : QSortFilterProxyModel (parent)
{
    sort(0, Qt::DescendingOrder);
    setSortRole(ResultModel::ScoreRole);
    setDynamicSortFilter(true);
    //prepare default items
    m_defaultModel = new QStandardItemModel(this);

    KService::Ptr service = KService::serviceByDesktopName(qGuiApp->desktopFileName());
    if (service) {
        const auto actions = service->actions();
        for (const KServiceAction &action : actions) {
            QStandardItem *item = new QStandardItem();
            item->setData(QUrl(QStringLiteral("kcm:%1.desktop").arg(action.name())), ResultModel::ResourceRole);
            m_defaultModel->appendRow(item);
        }
    } else {
        qCritical() << "Failed to find desktop file for" << qGuiApp->desktopFileName();
    }
}

void MostUsedModel::setResultModel(ResultModel *model)
{
    if (m_resultModel == model) {
        return;
    }

    auto updateModel = [this]() {
        if (m_resultModel->rowCount() >= 5) {
            setSourceModel(m_resultModel);
        } else {
            setSourceModel(m_defaultModel);
        }
    };

    m_resultModel = model;

    connect(m_resultModel, &QAbstractItemModel::rowsInserted, this, updateModel);
    connect(m_resultModel, &QAbstractItemModel::rowsRemoved, this, updateModel);

    updateModel();
}

QHash<int, QByteArray> MostUsedModel::roleNames() const
{
    QHash<int, QByteArray> roleNames;
    roleNames.insert(Qt::DisplayRole, "display");
    roleNames.insert(Qt::DecorationRole, "decoration");
    roleNames.insert(ResultModel::ScoreRole, "score");
    roleNames.insert(KcmPluginRole, "kcmPlugin");
    return roleNames;
}

bool MostUsedModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    const QString desktopName = sourceModel()->index(source_row, 0, source_parent).data(ResultModel::ResourceRole).toUrl().path();
    KService::Ptr service = KService::serviceByStorageId(desktopName);
    return service;
}

QVariant MostUsedModel::data(const QModelIndex &index, int role) const
{
    //MenuItem *mi;
    const QString desktopName = QSortFilterProxyModel::data(index, ResultModel::ResourceRole).toUrl().path();

    KService::Ptr service = KService::serviceByStorageId(desktopName);

    if (!service) {
        return QVariant();
    }

    switch (role) {

        case Qt::DisplayRole:
            return service->name();
        case Qt::DecorationRole:
            return service->icon();
        case KcmPluginRole: {
            return service->desktopEntryName();
            KCModuleInfo info(service);
            return info.handle();
        }
        case ResultModel::ScoreRole:
            return QSortFilterProxyModel::data(index, ResultModel::ScoreRole);
        default:
            return QVariant();
    }
}



LookAndFeelGroup::LookAndFeelGroup(QObject *parent)
    : QObject(parent)
{
    m_package = KPackage::PackageLoader::self()->loadPackage(QStringLiteral("Plasma/LookAndFeel"));
}

QString LookAndFeelGroup::id() const
{
    return m_package.metadata().pluginId();
}

QString LookAndFeelGroup::name() const
{
    return m_package.metadata().name();
}

QString LookAndFeelGroup::thumbnail() const
{
    return m_package.filePath("preview");;
}



KCMLandingPage::KCMLandingPage(QObject *parent, const QVariantList &args)
    : KQuickAddons::ManagedConfigModule(parent, args)
    , m_data(new LandingPageData(this))
{
    qmlRegisterType<LandingPageGlobalsSettings>();
    qmlRegisterType<BalooSettings>();
    qmlRegisterType<MostUsedModel>();
    qmlRegisterType<LookAndFeelGroup>();

    KAboutData *about = new KAboutData(QStringLiteral("kcm_landingpage"),
                                       i18n("Quick Settings"),
                                       QStringLiteral("1.1"),
                                       i18n("Landing page with some basic settings."),
                                       KAboutLicense::GPL);

    about->addAuthor(i18n("Marco Martin"), QString(), QStringLiteral("mart@kde.org"));
    setAboutData(about);

    setButtons(Apply | Help);

    m_mostUsedModel = new MostUsedModel(this);
    m_mostUsedModel->setResultModel(new ResultModel( AllResources | Agent(QStringLiteral("org.kde.systemsettings")) | HighScoredFirst | Limit(5), this));

    m_defaultLightLookAndFeel = new LookAndFeelGroup(this);
    m_defaultDarkLookAndFeel = new LookAndFeelGroup(this);

    m_defaultLightLookAndFeel->m_package.setPath(m_data->landingPageGlobalsSettings()->defaultLightLookAndFeel());
    m_defaultDarkLookAndFeel->m_package.setPath(m_data->landingPageGlobalsSettings()->defaultDarkLookAndFeel());

    connect(globalsSettings(), &LandingPageGlobalsSettings::lookAndFeelPackageChanged,
            this, [this]() {m_lnfDirty = true;});
}

MostUsedModel *KCMLandingPage::mostUsedModel() const
{
    return m_mostUsedModel;
}

LandingPageGlobalsSettings *KCMLandingPage::globalsSettings() const
{
    return m_data->landingPageGlobalsSettings();
}

BalooSettings *KCMLandingPage::balooSettings() const
{
    return m_data->balooSettings();
}

void KCMLandingPage::save()
{
    ManagedConfigModule::save();

    QDBusMessage message = QDBusMessage::createSignal("/KGlobalSettings", "org.kde.KGlobalSettings", "notifyChange");
    QList<QVariant> args;
    args.append(KGlobalSettings::SettingsChanged);
    args.append(KGlobalSettings::SETTINGS_MOUSE);
    message.setArguments(args);
    QDBusConnection::sessionBus().send(message);

    // Update Baloo config or start/stop Baloo
    if (balooSettings()->indexingEnabled()) {
        // Trying to start baloo when it is already running is fine
        const QString exe = QStandardPaths::findExecutable(QStringLiteral("baloo_file"));
        QProcess::startDetached(exe, QStringList());
    } else {
        QDBusMessage message =
            QDBusMessage::createMethodCall(QStringLiteral("org.kde.baloo"), QStringLiteral("/"), QStringLiteral("org.kde.baloo.main"), QStringLiteral("quit"));

        QDBusConnection::sessionBus().asyncCall(message);
    }


    if (m_lnfDirty) {
        QProcess::startDetached(QStringLiteral("lookandfeeltool"), QStringList({QStringLiteral("-a"), m_data->landingPageGlobalsSettings()->lookAndFeelPackage()}));
    }
}

static void copyEntry(KConfigGroup &from, KConfigGroup &to, const QString &entry)
{
    if (from.hasKey(entry)) {
        to.writeEntry(entry, from.readEntry(entry));
    }
}

LookAndFeelGroup *KCMLandingPage::defaultLightLookAndFeel() const
{
    return m_defaultLightLookAndFeel;
}

LookAndFeelGroup *KCMLandingPage::defaultDarkLookAndFeel() const
{
    return m_defaultDarkLookAndFeel;
}

void KCMLandingPage::openWallpaperDialog()
{
    QString connector;

    QQuickItem *item = mainUi();
    if (!item) {
        return;
    }

    QQuickWindow *quickWindow = item->window();
    if (!quickWindow) {
        return;
    }

    QWindow *window = QQuickRenderControl::renderWindowFor(quickWindow);
    if (!window) {
        return;
    }

    QScreen *screen = window->screen();
    if (screen) {
        connector = screen->name();
    }

    QDBusMessage message = QDBusMessage::createMethodCall(QStringLiteral("org.kde.plasmashell"), QStringLiteral("/PlasmaShell"),
                                                   QStringLiteral("org.kde.PlasmaShell"), QStringLiteral("evaluateScript"));

    QList<QVariant> args;
    args << QStringLiteral(R"(
        let id = screenForConnector("%1");

        if (id >= 0) {
            let desktop = desktopForScreen(id);
            desktop.showConfigurationInterface();
        })").arg(connector);

    message.setArguments(args);

    QDBusConnection::sessionBus().call(message, QDBus::NoBlock);
}

Q_INVOKABLE void KCMLandingPage::openKCM(const QString &kcm)
{
    QProcess::startDetached(QStringLiteral("systemsettings5"), QStringList({kcm}));
}

#include "landingpage.moc"
