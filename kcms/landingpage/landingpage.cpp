/*
    SPDX-FileCopyrightText: 2021 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2018 <furkantokac34@gmail.com>
    SPDX-FileCopyrightText: 2019 Cyril Rossi <cyril.rossi@enioka.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "landingpage.h"

#include <KColorScheme>
#include <KLocalizedString>
#include <KPackage/PackageLoader>
#include <KPluginFactory>
#include <KService>

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusPendingCall>
#include <QGuiApplication>
#include <QProcess>
#include <QQuickItem>
#include <QQuickRenderControl>
#include <QQuickWindow>
#include <QScreen>
#include <QStandardItemModel>

#include "landingpage_kdeglobalssettings.h"
#include "landingpagedata.h"

#include <PlasmaActivities/Stats/ResultModel>
#include <PlasmaActivities/Stats/ResultSet>
#include <PlasmaActivities/Stats/Terms>

namespace KAStats = KActivities::Stats;

using namespace KAStats;
using namespace KAStats::Terms;

K_PLUGIN_FACTORY_WITH_JSON(KCMLandingPageFactory, "kcm_landingpage.json", registerPlugin<KCMLandingPage>(); registerPlugin<LandingPageData>();)

// Program to icon hash
static QHash<QString, QString> s_programs = {{"plasmashell", "plasmashell"}, {"plasma-discover", "plasmadiscover"}};

MostUsedModel::MostUsedModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    sort(0, Qt::DescendingOrder);
    setSortRole(ResultModel::ScoreRole);
    setDynamicSortFilter(true);
    // prepare default items
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
        if (m_resultModel->rowCount() >= 6) {
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
    if (desktopName.endsWith(QLatin1String(".desktop"))) {
        const bool isAlreadyIgnored = ignoredKCMs.contains(desktopName);
        if (!isAlreadyIgnored) {
            ignoredKCMs.append(desktopName);
        }
        return false;
    }
    KService::Ptr service = KService::serviceByStorageId(desktopName);
    return service && (source_row - ignoredKCMs.size() < 6);
}

QVariant MostUsedModel::data(const QModelIndex &index, int role) const
{
    // MenuItem *mi;
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
    }
    case ResultModel::ScoreRole:
        return QSortFilterProxyModel::data(index, ResultModel::ScoreRole);
    default:
        return QVariant();
    }
}

LookAndFeelGroup::LookAndFeelGroup(QObject *parent)
    : QObject(parent)
    , m_package(KPackage::PackageLoader::self()->loadPackage(QStringLiteral("Plasma/LookAndFeel")))
{
}

QString LookAndFeelGroup::id() const
{
    return m_package.metadata().pluginId();
}

QString LookAndFeelGroup::name() const
{
    return m_package.metadata().name();
}

QUrl LookAndFeelGroup::thumbnail() const
{
    return m_package.fileUrl("preview");
}

KCMLandingPage::KCMLandingPage(QObject *parent, const KPluginMetaData &metaData)
    : KQuickManagedConfigModule(parent, metaData)
    , m_data(new LandingPageData(this))
{
    qmlRegisterAnonymousType<LandingPageGlobalsSettings>("org.kde.plasma.landingpage.kcm", 0);
    qmlRegisterAnonymousType<MostUsedModel>("org.kde.plasma.landingpage.kcm", 0);
    qmlRegisterAnonymousType<LookAndFeelGroup>("org.kde.plasma.landingpage.kcm", 0);

    setButtons(Apply);

    m_mostUsedModel = new MostUsedModel(this);
    m_mostUsedModel->setResultModel(new ResultModel(AllResources | Agent(QStringLiteral("org.kde.systemsettings")) | HighScoredFirst | Limit(12), this));

    m_defaultLightLookAndFeel = new LookAndFeelGroup(this);
    m_defaultDarkLookAndFeel = new LookAndFeelGroup(this);

    m_defaultLightLookAndFeel->m_package.setPath(globalsSettings()->defaultLightLookAndFeel());
    m_defaultDarkLookAndFeel->m_package.setPath(globalsSettings()->defaultDarkLookAndFeel());

    connect(globalsSettings(), &LandingPageGlobalsSettings::lookAndFeelPackageChanged, this, [this]() {
        m_lnfDirty = true;
    });
}

inline void swap(QJsonValueRef v1, QJsonValueRef v2)
{
    QJsonValue temp(v1);
    v1 = QJsonValue(v2);
    v2 = temp;
}

MostUsedModel *KCMLandingPage::mostUsedModel() const
{
    return m_mostUsedModel;
}

LandingPageGlobalsSettings *KCMLandingPage::globalsSettings() const
{
    return m_data->settings();
}

void KCMLandingPage::save()
{
    KQuickManagedConfigModule::save();

    QDBusMessage message = QDBusMessage::createSignal("/KGlobalSettings", "org.kde.KGlobalSettings", "notifyChange");
    QList<QVariant> args;
    args.append(3 /*KGlobalSettings::SettingsChanged*/);
    args.append(0 /*KGlobalSettings::SETTINGS_MOUSE*/);
    message.setArguments(args);
    QDBusConnection::sessionBus().send(message);

    if (m_lnfDirty) {
        QProcess::startDetached(QStringLiteral("plasma-apply-lookandfeel"), QStringList({QStringLiteral("-a"), globalsSettings()->lookAndFeelPackage()}));
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

Q_INVOKABLE void KCMLandingPage::openKCM(const QString &kcm)
{
    QProcess::startDetached(QStringLiteral("systemsettings"), QStringList({kcm}));
}

QAction *KCMLandingPage::kcmAction(const QString &storageId)
{
    if (KService::Ptr kcm = KService::serviceByStorageId(storageId)) {
        // Returning parent-less QObject from Q_INVOKABLE, QmlEngine will adopt it.
        auto *action = new QAction(QIcon::fromTheme(kcm->icon()), kcm->name());
        connect(action, &QAction::triggered, this, [this, storageId] {
            openKCM(storageId);
        });
        return action;
    }
    return nullptr;
}

#include "landingpage.moc"
#include "moc_landingpage.cpp"
