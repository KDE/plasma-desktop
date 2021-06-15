// vim: noexpandtab shiftwidth=4 tabstop=4
/*
   SPDX-FileCopyrightText: 2002 Daniel Molkentin <molkentin@kde.org>
   SPDX-FileCopyrightText: 2020 Kai Uwe Broulik <kde@broulik.de>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kcmkded.h"

#include "debug.h"

#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusPendingCall>
#include <QDBusPendingCallWatcher>

#include <KAboutData>
#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>
#include <KPluginFactory>

#include <algorithm>

#include "filterproxymodel.h"
#include "modulesmodel.h"

#include "kded_interface.h"
#include "kdedconfigdata.h"

K_PLUGIN_FACTORY_WITH_JSON(KCMStyleFactory, "kcmkded.json", registerPlugin<KDEDConfig>(); registerPlugin<KDEDConfigData>();)

static const QString s_kdedServiceName = QStringLiteral("org.kde.kded5");

KDEDConfig::KDEDConfig(QObject *parent, const QVariantList &args)
    : KQuickAddons::ConfigModule(parent, args)
    , m_model(new ModulesModel(this))
    , m_filteredModel(new FilterProxyModel(this))
    , m_kdedInterface(new org::kde::kded5(s_kdedServiceName, QStringLiteral("/kded"), QDBusConnection::sessionBus()))
    , m_kdedWatcher(new QDBusServiceWatcher(s_kdedServiceName, QDBusConnection::sessionBus(), QDBusServiceWatcher::WatchForOwnerChange, this))
{
    qmlRegisterUncreatableType<KDEDConfig>("org.kde.private.kcms.style", 1, 0, "KCM", QStringLiteral("Cannot create instances of KCM"));
    // FIXME Qt 5.14 qmlRegisterAnonymousType
    qmlRegisterType<ModulesModel>();
    qmlRegisterType<FilterProxyModel>();

    KAboutData *about = new KAboutData(QStringLiteral("kcm5_kded"),
                                       i18n("Background Services"),
                                       QStringLiteral("2.0"),
                                       QString(),
                                       KAboutLicense::GPL,
                                       i18n("(c) 2002 Daniel Molkentin, (c) 2020 Kai Uwe Broulik"));
    about->addAuthor(i18n("Daniel Molkentin"), QString(), QStringLiteral("molkentin@kde.org"));
    about->addAuthor(i18n("Kai Uwe Broulik"), QString(), QStringLiteral("kde@broulik.de"));
    setAboutData(about);
    setButtons(Apply | Default | Help);

    m_filteredModel->setSourceModel(m_model);

    connect(m_model, &ModulesModel::autoloadedModulesChanged, this, [this] {
        setNeedsSave(m_model->needsSave());
        setRepresentsDefaults(m_model->representsDefault());
    });

    connect(m_kdedWatcher, &QDBusServiceWatcher::serviceOwnerChanged, this, [this](const QString &service, const QString &oldOwner, const QString &newOwner) {
        Q_UNUSED(service)
        Q_UNUSED(oldOwner)
        setKdedRunning(!newOwner.isEmpty());
    });
    setKdedRunning(QDBusConnection::sessionBus().interface()->isServiceRegistered(s_kdedServiceName));
}

ModulesModel *KDEDConfig::model() const
{
    return m_model;
}

FilterProxyModel *KDEDConfig::filteredModel() const
{
    return m_filteredModel;
}

bool KDEDConfig::kdedRunning() const
{
    return m_kdedRunning;
}

void KDEDConfig::setKdedRunning(bool kdedRunning)
{
    if (m_kdedRunning == kdedRunning) {
        return;
    }

    m_kdedRunning = kdedRunning;
    Q_EMIT kdedRunningChanged();

    if (kdedRunning) {
        getModuleStatus();
    } else {
        m_model->setRunningModulesKnown(false);
    }
}

void KDEDConfig::startModule(const QString &moduleName)
{
    startOrStopModule(moduleName, Running);
}

void KDEDConfig::stopModule(const QString &moduleName)
{
    startOrStopModule(moduleName, NotRunning);
}

void KDEDConfig::startOrStopModule(const QString &moduleName, ModuleStatus status)
{
    auto call = (status == NotRunning ? m_kdedInterface->unloadModule(moduleName) : m_kdedInterface->loadModule(moduleName));

    QDBusPendingCallWatcher *callWatcher = new QDBusPendingCallWatcher(call, this);
    connect(callWatcher, &QDBusPendingCallWatcher::finished, this, [this, moduleName, status](QDBusPendingCallWatcher *watcher) {
        QDBusPendingReply<bool> reply = *watcher;
        watcher->deleteLater();

        if (reply.isError()) {
            if (status == NotRunning) {
                Q_EMIT errorMessage(i18n("Failed to stop service: %1", reply.error().message()));
            } else {
                Q_EMIT errorMessage(i18n("Failed to start service: %1", reply.error().message()));
            }
            return;
        }

        if (!reply.value()) {
            if (status == NotRunning) {
                Q_EMIT errorMessage(i18n("Failed to stop service."));
            } else {
                Q_EMIT errorMessage(i18n("Failed to start service."));
            }
            return;
        }

        qCDebug(KCM_KDED) << "Successfully" << (status == Running ? "started" : "stopped") << moduleName;
        if (status == Running) {
            m_lastStartedModule = moduleName;
        } else {
            m_lastStartedModule.clear();
        }
        getModuleStatus();
    });
}

void KDEDConfig::getModuleStatus()
{
    auto call = m_kdedInterface->loadedModules();

    QDBusPendingCallWatcher *callWatcher = new QDBusPendingCallWatcher(call, this);
    connect(callWatcher, &QDBusPendingCallWatcher::finished, this, [this](QDBusPendingCallWatcher *watcher) {
        QDBusPendingReply<QStringList> reply = *watcher;
        watcher->deleteLater();

        if (reply.isError()) {
            qCWarning(KCM_KDED) << "Failed to get loaded modules" << reply.error().name() << reply.error().message();
            return;
        }

        QStringList runningModules = reply.value();
        m_model->setRunningModules(runningModules);
        m_model->setRunningModulesKnown(true);

        // Check if the user just tried starting a module that then disabled itself again.
        // Some kded modules disable themselves on start when they deem themselves unnecessary
        // based on some configuration independ of kded or the current environment.
        // At least provide some feedback and not leave the user wondering why the service doesn't start.
        if (!m_lastStartedModule.isEmpty() && !runningModules.contains(m_lastStartedModule)) {
            Q_EMIT showSelfDisablingModulesHint();
        }
        m_lastStartedModule.clear();

        // Check if any modules got started/stopped as a result of reloading kded
        if (!m_runningModulesBeforeReconfigure.isEmpty()) {
            std::sort(m_runningModulesBeforeReconfigure.begin(), m_runningModulesBeforeReconfigure.end());
            std::sort(runningModules.begin(), runningModules.end());

            if (m_runningModulesBeforeReconfigure != runningModules) {
                Q_EMIT showRunningModulesChangedAfterSaveHint();
            }
        }
        m_runningModulesBeforeReconfigure.clear();
    });
}

void KDEDConfig::load()
{
    m_model->load();

    setNeedsSave(false);
    setRepresentsDefaults(m_model->representsDefault());
}

void KDEDConfig::save()
{
    KConfig kdedrc(QStringLiteral("kded5rc"), KConfig::NoGlobals);

    for (int i = 0; i < m_model->rowCount(); ++i) {
        const QModelIndex idx = m_model->index(i, 0);

        const auto type = static_cast<ModuleType>(idx.data(ModulesModel::TypeRole).toInt());
        if (type != AutostartType) {
            continue;
        }

        const QString moduleName = idx.data(ModulesModel::ModuleNameRole).toString();

        const bool autoloadEnabled = idx.data(ModulesModel::AutoloadEnabledRole).toBool();
        KConfigGroup cg(&kdedrc, QStringLiteral("Module-%1").arg(moduleName));
        cg.writeEntry("autoload", autoloadEnabled);
    }

    kdedrc.sync();
    m_model->refreshAutoloadEnabledSavedState();
    setNeedsSave(false);

    m_runningModulesBeforeReconfigure = m_model->runningModules();

    // Is all of this really necessary? I would also think it to be fire and forget...
    // Only if changing autoload for a module may load/unload it, otherwise there's no point.
    // Autoload doesn't affect a running session and reloading the running modules is also useless then.
    auto call = m_kdedInterface->reconfigure();
    QDBusPendingCallWatcher *callWatcher = new QDBusPendingCallWatcher(call, this);
    connect(callWatcher, &QDBusPendingCallWatcher::finished, this, [this](QDBusPendingCallWatcher *watcher) {
        QDBusPendingReply<void> reply = *watcher;
        watcher->deleteLater();

        if (reply.isError()) {
            Q_EMIT errorMessage(i18n("Failed to notify KDE Service Manager (kded5) of saved changed: %1", reply.error().message()));
            return;
        }

        qCDebug(KCM_KDED) << "Successfully reconfigured kded";
        getModuleStatus();
    });
}

void KDEDConfig::defaults()
{
    for (int i = 0; i < m_model->rowCount(); ++i) {
        const QModelIndex idx = m_model->index(i, 0);
        m_model->setData(idx, true, ModulesModel::AutoloadEnabledRole);
    }
}

#include "kcmkded.moc"
