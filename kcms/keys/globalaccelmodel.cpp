/*
    SPDX-FileCopyrightText: 2020 David Redondo <david@david-redondo.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "globalaccelmodel.h"

#include <QDBusPendingCallWatcher>
#include <QFile>

#include <KApplicationTrader>
#include <KConfigGroup>
#include <KDesktopFile>
#include <KGlobalAccel>
#include <KGlobalShortcutInfo>
#include <KGlobalShortcutInfoExt>
#include <KLocalizedString>
#include <KService>

#include "basemodel.h"
#include "component.h"
#include "kcmkeys_debug.h"

#include <kglobalaccel_componentprivatesettings_interface.h>
#include <kglobalaccel_privatesettings_interface.h>

struct GlobalAccelModelPrivate {
    QList<Component> pendingComponents;
    KGlobalAccelPrivateSettingsInterface *globalAccelPrivateSettingsInterface;
};

static QStringList buildActionId(const QString &componentUnique, const QString &componentFriendly, const QString &actionUnique, const QString &actionFriendly)
{
    QStringList actionId(4);
    actionId[KGlobalAccel::ComponentUnique] = componentUnique;
    actionId[KGlobalAccel::ComponentFriendly] = componentFriendly;
    actionId[KGlobalAccel::ActionUnique] = actionUnique;
    actionId[KGlobalAccel::ActionFriendly] = actionFriendly;
    return actionId;
}

GlobalAccelModel::GlobalAccelModel(QObject *parent)
    : BaseModel(parent)
    , d(new GlobalAccelModelPrivate)
{
    d->globalAccelPrivateSettingsInterface = new KGlobalAccelPrivateSettingsInterface(QStringLiteral("org.kde.kglobalaccel"),
                                                                                      QStringLiteral("/kglobalaccel/privatesettings"),
                                                                                      QDBusConnection::sessionBus(),
                                                                                      this);
    if (!d->globalAccelPrivateSettingsInterface->isValid()) {
        qCCritical(KCMKEYS) << "Interface is not valid";
        if (d->globalAccelPrivateSettingsInterface->lastError().isValid()) {
            qCCritical(KCMKEYS) << d->globalAccelPrivateSettingsInterface->lastError().name() << d->globalAccelPrivateSettingsInterface->lastError().message();
        }
    }

    qDBusRegisterMetaType<std::pair<QStringList, QStringList>>();
    qDBusRegisterMetaType<KGlobalShortcutTrigger>();
    qDBusRegisterMetaType<KGlobalShortcutInfo>();
    qDBusRegisterMetaType<KGlobalShortcutInfoExt>();
    qDBusRegisterMetaType<QList<KGlobalShortcutInfoExt>>();
}

GlobalAccelModel::~GlobalAccelModel()
{
}

QVariant GlobalAccelModel::data(const QModelIndex &index, int role) const
{
    if (role == SupportsMultipleKeysRole) {
        return false;
    }
    return BaseModel::data(index, role);
}

void GlobalAccelModel::load()
{
    if (!d->globalAccelPrivateSettingsInterface->isValid()) {
        return;
    }

    auto componentsWatcher = new QDBusPendingCallWatcher(d->globalAccelPrivateSettingsInterface->allComponents());
    connect(componentsWatcher, &QDBusPendingCallWatcher::finished, this, [this](QDBusPendingCallWatcher *componentsWatcher) {
        QDBusPendingReply<QList<QDBusObjectPath>> componentsReply = *componentsWatcher;
        componentsWatcher->deleteLater();
        if (componentsReply.isError()) {
            genericErrorOccured(QStringLiteral("Error while calling allComponents()"), componentsReply.error());

            beginResetModel();
            components().clear();
            d->pendingComponents.clear();
            endResetModel();

            return;
        }
        const QList<QDBusObjectPath> componentPaths = componentsReply.value();
        int *pendingCalls = new int;
        *pendingCalls = componentPaths.size();
        for (const auto &componentPath : componentPaths) {
            const QString path = componentPath.path();
            KGlobalAccelComponentPrivateSettingsInterface component(d->globalAccelPrivateSettingsInterface->service(),
                                                                    path,
                                                                    d->globalAccelPrivateSettingsInterface->connection());
            auto watcher = new QDBusPendingCallWatcher(component.allShortcutInfos());
            connect(watcher, &QDBusPendingCallWatcher::finished, this, [path, pendingCalls, this](QDBusPendingCallWatcher *watcher) {
                QDBusPendingReply<QList<KGlobalShortcutInfoExt>> reply = *watcher;
                if (reply.isError()) {
                    genericErrorOccured(QStringLiteral("Error while calling allShortcutInfos of") + path, reply.error());
                } else if (!reply.value().isEmpty()) {
                    d->pendingComponents.push_back(loadComponent(reply.value()));
                }
                watcher->deleteLater();
                if (--*pendingCalls == 0) {
                    beginResetModel();
                    QCollator collator;
                    collator.setCaseSensitivity(Qt::CaseInsensitive);
                    collator.setNumericMode(true);
                    components() = d->pendingComponents;
                    d->pendingComponents.clear();
                    std::sort(components().begin(), components().end(), [&](const Component &c1, const Component &c2) {
                        return c1.type != c2.type ? c1.type < c2.type : collator.compare(c1.displayName, c2.displayName) < 0;
                    });
                    endResetModel();
                    delete pendingCalls;
                }
            });
        }
    });
}

ComponentType classify(const KService::Ptr service)
{
    if (!service || !service->isApplication()) {
        return ComponentType::SystemService;
    }

    if (service->property<bool>(QStringLiteral("X-KDE-GlobalAccel-CommandShortcut"))) {
        return ComponentType::Command;
    }

    if (service->entryPath().startsWith(QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1String("/applications/"))) {
        // this service was explicitely added by the user. Whether it's actually an "application" is uncertain, but it's probably not an actual system
        // service.
        return ComponentType::Application;
    }

    if ((service->noDisplay() || service->property<QString>(QStringLiteral("X-KDE-GlobalShortcutType")) == QLatin1String("Service"))) {
        // services with noDisplay are typically KCMs or implementation details
        // don't show them as "Application"
        return ComponentType::SystemService;
    }

    return ComponentType::Application;
};

Component GlobalAccelModel::loadComponent(const QList<KGlobalShortcutInfoExt> &infosExt)
{
    const QString &componentUnique = infosExt[0].info().componentUniqueName();
    const QString &componentFriendly = infosExt[0].info().componentFriendlyName();

    KService::Ptr service = KService::serviceByStorageId(componentUnique);
    // Not a normal desktop file but maybe specific file in kglobalaccel dir
    if (!service && componentUnique.endsWith(QLatin1String(".desktop"))) {
        QString path = QStandardPaths::locate(QStandardPaths::ApplicationsLocation, componentUnique);

        if (path.isEmpty()) {
            path = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QStringLiteral("kglobalaccel/") + componentUnique);
        }

        if (!path.isEmpty()) {
            service = new KService(path);
        }
    }

    if (!service) {
        // Do we have a service with that name?
        auto filter = [componentUnique, componentFriendly](const KService::Ptr service) {
            return service->name() == componentUnique || service->name() == componentFriendly;
        };

        const KService::List services = KApplicationTrader::query(filter);
        service = services.value(0, KService::Ptr());
    }

    auto type = classify(service);

    QString icon;

    static const QHash<QString, QString> hardCodedIcons = {
        {"ActivityManager", "preferences-desktop-activities"},
        {"KDE Keyboard Layout Switcher", "input-keyboard"},
        {"org_kde_powerdevil", "preferences-system-power-management"},
        {"wacomtablet", "preferences-desktop-tablet"},
    };

    if (service && !service->icon().isEmpty()) {
        icon = service->icon();
    } else if (hardCodedIcons.contains(componentUnique)) {
        icon = hardCodedIcons[componentUnique];
    } else if (type == ComponentType::Command) {
        icon = QStringLiteral("system-run");
    } else {
        icon = componentUnique;
    }

    Component c{componentUnique, componentFriendly, type, icon, {}, false, false};
    for (const auto &infoExt : infosExt) {
        const auto &actionInfo = infoExt.info();
        const QString &actionUnique = actionInfo.uniqueName();
        const QString &actionFriendly = actionInfo.friendlyName();
        Action action;
        action.id = actionUnique;
        action.displayName = actionFriendly;
        action.inverseAction = infoExt.inverseAction();
        const QList<QKeySequence> defaultShortcuts = actionInfo.defaultKeys();
        for (const auto &keySequence : defaultShortcuts) {
            if (!keySequence.isEmpty()) {
                action.defaultShortcuts.insert(keySequence);
            }
        }
        const QList<QKeySequence> activeShortcuts = actionInfo.keys();
        for (const QKeySequence &keySequence : activeShortcuts) {
            if (!keySequence.isEmpty()) {
                action.activeShortcuts.insert(keySequence);
            }
        }
        action.initialShortcuts = action.activeShortcuts;
        c.actions.push_back(action);

        const QStringList &triggerTypes = infoExt.triggerTypes();
        for (const QString &triggerType : triggerTypes) {
            auto &triggerSets = action.triggersByType[triggerType];
            triggerSets.activeTriggers = infoExt.triggers(triggerType);
            triggerSets.initialTriggers = triggerSets.activeTriggers;
            triggerSets.defaultTriggers = infoExt.defaultTriggers(triggerType);
        }
    }
    QCollator collator;
    collator.setCaseSensitivity(Qt::CaseInsensitive);
    collator.setNumericMode(true);
    std::sort(c.actions.begin(), c.actions.end(), [&](const Action &s1, const Action &s2) {
        return collator.compare(s1.displayName, s2.displayName) < 0;
    });
    return c;
}

void GlobalAccelModel::save()
{
    QList<Component *> componentsToRemove;

    for (auto it = components().rbegin(); it != components().rend(); ++it) {
        if (it->pendingDeletion) {
            componentsToRemove.append(&(*it));
        }
    }

    for (Component *component : std::as_const(componentsToRemove)) {
        removeComponent(*component);
    }

    // removing the keys first ensures there are no temporary conflicts
    for (auto &component : components()) {
        for (auto &action : component.actions) {
            if (action.initialShortcuts != action.activeShortcuts) {
                QSet<QKeySequence> removed = action.initialShortcuts - action.activeShortcuts;
                if (!removed.isEmpty()) {
                    QSet<QKeySequence> unchangedShortcuts = action.activeShortcuts & action.initialShortcuts;
                    bool shortcutsApplied = saveAction(component, action, unchangedShortcuts);
                    if (shortcutsApplied) {
                        action.initialShortcuts = unchangedShortcuts;
                    }
                }
            }
            for (auto [triggerType, triggerSets] : action.triggersByType.asKeyValueRange()) {
                QSet<KGlobalShortcutTrigger> removed = triggerSets.initialTriggers - triggerSets.activeTriggers;
                if (!removed.isEmpty()) {
                    QSet<KGlobalShortcutTrigger> unchangedTriggers = triggerSets.activeTriggers & triggerSets.initialTriggers;
                    bool triggersApplied = saveActionTriggers(component, action, triggerType, unchangedTriggers);
                    if (triggersApplied) {
                        triggerSets.initialTriggers = unchangedTriggers;
                    }
                }
            }
        }
    }

    for (auto &component : components()) {
        for (auto &action : component.actions) {
            if (action.initialShortcuts != action.activeShortcuts) {
                bool shortcutsApplied = saveAction(component, action, action.activeShortcuts);
                if (shortcutsApplied) {
                    action.initialShortcuts = action.activeShortcuts;
                }
            }
            for (auto [triggerType, triggerSets] : action.triggersByType.asKeyValueRange()) {
                if (triggerSets.initialTriggers != triggerSets.activeTriggers) {
                    bool triggersApplied = saveActionTriggers(component, action, triggerType, triggerSets.activeTriggers);
                    if (triggersApplied) {
                        triggerSets.initialTriggers = triggerSets.activeTriggers;
                    }
                }
            }
        }
    }
}

bool GlobalAccelModel::saveAction(const Component &component, const Action &action, const QSet<QKeySequence> &shortcutsToSave)
{
    const QStringList actionId = buildActionId(component.id, component.displayName, action.id, action.displayName);
    qCDebug(KCMKEYS) << "Saving" << actionId << "target" << action.activeShortcuts << "applying" << shortcutsToSave;
    auto reply = d->globalAccelPrivateSettingsInterface->setForeignShortcutKeys(actionId, shortcutsToSave.values());
    reply.waitForFinished();
    if (!reply.isValid()) {
        qCCritical(KCMKEYS) << "Error while saving";
        if (reply.error().isValid()) {
            qCCritical(KCMKEYS) << reply.error().name() << reply.error().message();
        }
        Q_EMIT errorOccured(i18nc("%1 is the name of the component, %2 is the action for which saving failed",
                                  "Error while saving shortcut %1: %2",
                                  component.displayName,
                                  action.displayName));
        return false;
    }
    return true;
}

bool GlobalAccelModel::saveActionTriggers(const Component &component,
                                          const Action &action,
                                          const QString &triggerType,
                                          const QSet<KGlobalShortcutTrigger> &triggersToSave)
{
    const QStringList actionId = buildActionId(component.id, component.displayName, action.id, action.displayName);
    QStringList triggerParamStrings;
    triggerParamStrings.reserve(triggersToSave.size());
    for (const KGlobalShortcutTrigger &trigger : triggersToSave) {
        if (triggerType == trigger.type()) {
            triggerParamStrings.append(trigger.paramString());
        }
    }
    qCDebug(KCMKEYS) << "Saving" << actionId << "applying" << triggerType << triggerParamStrings;
    auto reply = d->globalAccelPrivateSettingsInterface->setForeignShortcutTriggers(actionId, triggerType, triggerParamStrings);
    reply.waitForFinished();
    if (!reply.isValid()) {
        qCCritical(KCMKEYS) << "Error while saving";
        if (reply.error().isValid()) {
            qCCritical(KCMKEYS) << reply.error().name() << reply.error().message();
        }
        Q_EMIT errorOccured(i18nc("%1 is the name of the component, %2 is the action for which saving failed",
                                  "Error while saving shortcut %1: %2",
                                  component.displayName,
                                  action.displayName));
        return false;
    }
    return true;
}

bool GlobalAccelModel::isValid() const
{
    return d->globalAccelPrivateSettingsInterface->isValid();
}

void GlobalAccelModel::exportToConfig(KConfigBase &config)
{
    for (const auto &component : std::as_const(components())) {
        if (component.checked) {
            KConfigGroup mainGroup(&config, component.id);
            KConfigGroup group(&mainGroup, QStringLiteral("Global Shortcuts"));
            for (const auto &action : component.actions) {
                const QList<QKeySequence> shortcutsList(action.activeShortcuts.cbegin(), action.activeShortcuts.cend());
                group.writeEntry(action.id, QKeySequence::listToString(shortcutsList));
            }
        }
    }
}

void GlobalAccelModel::importConfig(const KConfigBase &config)
{
    const auto groupList = config.groupList();
    for (const auto &componentGroupName : groupList) {
        auto component = std::find_if(components().begin(), components().end(), [&](const Component &c) {
            return c.id == componentGroupName;
        });
        if (component == components().end()) {
            qCWarning(KCMKEYS) << "Ignoring unknown component" << componentGroupName;
            continue;
        }
        KConfigGroup componentGroup(&config, componentGroupName);
        if (!componentGroup.hasGroup("Global Shortcuts")) {
            qCWarning(KCMKEYS) << "Group" << componentGroupName << "has no shortcuts group";
            continue;
        }
        KConfigGroup shortcutsGroup(&componentGroup, QStringLiteral("Global Shortcuts"));
        const QStringList keys = shortcutsGroup.keyList();
        for (const auto &key : keys) {
            auto action = std::find_if(component->actions.begin(), component->actions.end(), [&](const Action &a) {
                return a.id == key;
            });
            if (action == component->actions.end()) {
                qCWarning(KCMKEYS) << "Ignoring unknown action" << key;
                continue;
            }
            const auto shortcuts = QKeySequence::listFromString(shortcutsGroup.readEntry(key));
            const QSet<QKeySequence> shortcutsSet(shortcuts.cbegin(), shortcuts.cend());
            if (shortcutsSet != action->activeShortcuts) {
                action->activeShortcuts = shortcutsSet;
                const QModelIndex i = index(action - component->actions.begin(), 0, index(component - components().begin(), 0));
                Q_EMIT dataChanged(i, i, {CustomShortcutsRole, ActiveShortcutsRole});
            }
        }
    }
}

void GlobalAccelModel::addApplication(const QString &desktopFileName, const QString &displayName)
{
    if (desktopFileName.isEmpty()) {
        qCWarning(KCMKEYS()) << "Tried to add empty application" << displayName;
        return;
    }

    // In certain cases, we can get an absolute file name as desktopFileName,
    // but the rest of the code assumes we're using a relative filename. So in
    // that case, strip the paths off and only use the file name.
    QFileInfo info(desktopFileName);

    QString desktopName = desktopFileName;
    if (info.isAbsolute()) {
        desktopName = info.fileName();
    }

    KDesktopFile desktopFile(desktopName);
    KConfigGroup cg = desktopFile.desktopGroup();
    ComponentType type = cg.readEntry<bool>(QStringLiteral("X-KDE-GlobalAccel-CommandShortcut"), false) ? ComponentType::Command : ComponentType::Application;

    QCollator collator;
    collator.setCaseSensitivity(Qt::CaseInsensitive);
    collator.setNumericMode(true);
    auto pos = std::lower_bound(components().begin(), components().end(), displayName, [&](const Component &c, const QString &name) {
        return c.type != ComponentType::SystemService && (c.type != type ? c.type < type : collator.compare(c.displayName, name) < 0);
    });
    auto watcher = new QDBusPendingCallWatcher(d->globalAccelPrivateSettingsInterface->desktopFileComponent(desktopName));
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [=, this] {
        QDBusPendingReply<QDBusObjectPath> reply = *watcher;
        watcher->deleteLater();
        if (!reply.isValid()) {
            genericErrorOccured(QStringLiteral("Error while calling objectPath of added application") + desktopName, reply.error());
            return;
        }
        KGlobalAccelComponentPrivateSettingsInterface component(d->globalAccelPrivateSettingsInterface->service(),
                                                                reply.value().path(),
                                                                d->globalAccelPrivateSettingsInterface->connection());
        auto infoWatcher = new QDBusPendingCallWatcher(component.allShortcutInfos());
        connect(infoWatcher, &QDBusPendingCallWatcher::finished, this, [=, this] {
            QDBusPendingReply<QList<KGlobalShortcutInfoExt>> infoReply = *infoWatcher;
            infoWatcher->deleteLater();
            if (!infoReply.isValid()) {
                genericErrorOccured(QStringLiteral("Error while calling allShortCutInfos on new component") + desktopName, infoReply.error());
                return;
            }
            if (infoReply.value().isEmpty()) {
                qCWarning(KCMKEYS()) << "New component has no shortcuts:" << reply.value().path();
                Q_EMIT errorOccured(i18nc("%1 is the name of an application", "Error while adding %1, it seems it has no actions."));
            }
            qCDebug(KCMKEYS) << "inserting at " << pos - components().begin();
            beginInsertRows(QModelIndex(), pos - components().begin(), pos - components().begin());
            auto c = loadComponent(infoReply.value());
            components().insert(pos, c);
            endInsertRows();
            Q_EMIT applicationAdded(c);
        });
    });
}

void GlobalAccelModel::updateCommandName(const QString &uniqueName)
{
    auto component = std::find_if(components().cbegin(), components().cend(), [&uniqueName](const Component &c) {
        return c.id == uniqueName;
    });
    if (component == components().cend()) {
        return;
    }

    const auto &actions = component->actions;
    auto action = std::find_if(actions.begin(), actions.end(), [](const Action &a) {
        return a.id == QStringLiteral("_launch");
    });
    if (action == actions.end()) {
        return;
    }

    const int componentRow = std::distance(components().cbegin(), component);
    const int actionRow = std::distance(actions.begin(), action);

    const QModelIndex componentIndex = index(componentRow, 0);
    const QModelIndex actionIndex = index(actionRow, 0, componentIndex);

    Q_EMIT dataChanged(actionIndex, actionIndex, {Qt::DisplayRole});
    Q_EMIT dataChanged(actionIndex.parent(), actionIndex.parent(), {Qt::DisplayRole});
}

void GlobalAccelModel::removeComponent(const Component &component)
{
    const QString &uniqueName = component.id;
    auto componentReply = d->globalAccelPrivateSettingsInterface->getComponent(uniqueName);
    componentReply.waitForFinished();
    if (!componentReply.isValid()) {
        genericErrorOccured(QStringLiteral("Error while calling objectPath of component") + uniqueName, componentReply.error());
        return;
    }
    if (component.type == ComponentType::Command) {
        KService::Ptr service = KService::serviceByStorageId(component.id);
        if (service) {
            qCDebug(KCMKEYS) << "Removing " << service->entryPath();
            QFile::remove(service->entryPath());
        }
    }
    KGlobalAccelComponentPrivateSettingsInterface componentInterface(d->globalAccelPrivateSettingsInterface->service(),
                                                                     componentReply.value().path(),
                                                                     d->globalAccelPrivateSettingsInterface->connection());
    qCDebug(KCMKEYS) << "Cleaning up component at" << componentReply.value().path();
    auto cleanUpReply = componentInterface.cleanUp();
    cleanUpReply.waitForFinished();
    if (!cleanUpReply.isValid()) {
        genericErrorOccured(QStringLiteral("Error while calling cleanUp of component") + uniqueName, cleanUpReply.error());
        return;
    }
    auto it = std::find_if(components().begin(), components().end(), [&](const Component &c) {
        return c.id == uniqueName;
    });
    const int row = it - components().begin();
    beginRemoveRows(QModelIndex(), row, row);
    components().remove(row);
    endRemoveRows();
}

void GlobalAccelModel::genericErrorOccured(const QString &description, const QDBusError &error)
{
    qCCritical(KCMKEYS) << description;
    if (error.isValid()) {
        qCCritical(KCMKEYS) << error.name() << error.message();
    }
    Q_EMIT this->errorOccured(i18n("Error while communicating with the global shortcuts service"));
}

#include "moc_globalaccelmodel.cpp"
