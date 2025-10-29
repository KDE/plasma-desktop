/*
    SPDX-FileCopyrightText: 2021 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2018 <furkantokac34@gmail.com>
    SPDX-FileCopyrightText: 2019 Cyril Rossi <cyril.rossi@enioka.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "landingpage.h"
#include "lookandfeelmetadata.h"
#include "lookandfeelmodel.h"
#include "splititem.h"

#include <KAuthorized>
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
        auto oldModel = sourceModel();
        if (m_resultModel->rowCount() >= 6) {
            setSourceModel(m_resultModel);
        } else {
            setSourceModel(m_defaultModel);
        }
        if (oldModel != sourceModel()) {
            ignoredKCMs.clear();
        }
        invalidateFilter();
    };

    m_resultModel = model;

    connect(m_resultModel, &QAbstractItemModel::rowsInserted, this, updateModel);
    connect(m_resultModel, &QAbstractItemModel::rowsRemoved, this, updateModel);

    updateModel();
}

QHash<int, QByteArray> MostUsedModel::roleNames() const
{
    QHash<int, QByteArray> roleNames;
    roleNames.insert(Qt::DisplayRole, QByteArrayLiteral("display"));
    roleNames.insert(Qt::DecorationRole, QByteArrayLiteral("decoration"));
    roleNames.insert(ResultModel::ScoreRole, QByteArrayLiteral("score"));
    roleNames.insert(KcmPluginRole, QByteArrayLiteral("kcmPlugin"));
    return roleNames;
}

bool MostUsedModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    const auto ignoreKCM = [this](const QString &desktopName) {
        const bool isAlreadyIgnored = ignoredKCMs.contains(desktopName);
        if (!isAlreadyIgnored) {
            ignoredKCMs.append(desktopName);
        }
    };

    const QString desktopName = sourceModel()->index(source_row, 0, source_parent).data(ResultModel::ResourceRole).toUrl().path();

    if (desktopName.endsWith(QLatin1String(".desktop"))) {
        ignoreKCM(desktopName);
        return false;
    }

    if (!KAuthorized::authorizeControlModule(desktopName)) {
        ignoreKCM(desktopName);
        return false;
    }

    KService::Ptr service = KService::serviceByStorageId(desktopName);
    if (!service || !service->showOnCurrentPlatform() || !service->exec().startsWith(u"systemsettings")) {
        ignoreKCM(desktopName);
        return false;
    }

    return source_row - ignoredKCMs.size() < 6;
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

KCMLandingPage::KCMLandingPage(QObject *parent, const KPluginMetaData &metaData)
    : KQuickManagedConfigModule(parent, metaData)
    , m_data(new LandingPageData(this))
{
    qmlRegisterAnonymousType<LandingPageGlobalsSettings>("org.kde.plasma.landingpage.kcm", 0);
    qmlRegisterAnonymousType<MostUsedModel>("org.kde.plasma.landingpage.kcm", 0);
    qmlRegisterType<SplitItem>("org.kde.plasma.landingpage.kcm", 1, 0, "SplitView");
    qmlRegisterUncreatableMetaObject(KLookAndFeel::staticMetaObject, "org.kde.plasma.landingpage.kcm", 1, 0, "LookAndFeel", "for Variant enum");
    qmlRegisterType<LookAndFeelModel>("org.kde.plasma.landingpage.kcm", 1, 0, "LookAndFeelModel");
    qmlRegisterType<LookAndFeelMetaData>("org.kde.plasma.landingpage.kcm", 1, 0, "LookAndFeelMetaData");

    setButtons(Apply);

    m_mostUsedModel = new MostUsedModel(this);
    m_mostUsedModel->setResultModel(new ResultModel(AllResources | Agent(QStringLiteral("org.kde.systemsettings")) | HighScoredFirst | Limit(12), this));
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
    const auto lookAndFeelPackageItem = globalsSettings()->findItem(QStringLiteral("lookAndFeelPackage"));
    const auto automaticLookAndFeelItem = globalsSettings()->findItem(QStringLiteral("automaticLookAndFeel"));
    const bool automaticLookAndFeel = automaticLookAndFeelItem->property().toBool();
    const bool applyNewLnf = !automaticLookAndFeel && (lookAndFeelPackageItem->isSaveNeeded() || automaticLookAndFeelItem->isSaveNeeded());

    KQuickManagedConfigModule::save();

    QDBusMessage message = QDBusMessage::createSignal("/KGlobalSettings", "org.kde.KGlobalSettings", "notifyChange");
    QList<QVariant> args;
    args.append(3 /*KGlobalSettings::SettingsChanged*/);
    args.append(0 /*KGlobalSettings::SETTINGS_MOUSE*/);
    message.setArguments(args);
    QDBusConnection::sessionBus().send(message);

    if (applyNewLnf) {
        QProcess::startDetached(QStringLiteral("plasma-apply-lookandfeel"), QStringList({QStringLiteral("-a"), globalsSettings()->lookAndFeelPackage()}));
    }
}

QString KCMLandingPage::defaultLookAndFeelPackage() const
{
    return m_data->settings()->defaultLookAndFeelPackageValue();
}

Q_INVOKABLE void KCMLandingPage::openKCM(const QString &kcm)
{
    QProcess::startDetached(QStringLiteral("systemsettings"), QStringList({kcm}));
}

QAction *KCMLandingPage::kcmAction(const QString &storageId)
{
    if (!KAuthorized::authorizeControlModule(storageId)) {
        return nullptr;
    }

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
