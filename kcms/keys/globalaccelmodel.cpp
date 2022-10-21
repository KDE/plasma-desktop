/*
    SPDX-FileCopyrightText: 2020 David Redondo <david@david-redondo.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "globalaccelmodel.h"

#include <QDBusPendingCallWatcher>
#include <QIcon>

#include <KApplicationTrader>
#include <KConfigGroup>
#include <KGlobalAccel>
#include <KGlobalShortcutInfo>
#include <KLocalizedString>
#include <KService>
#include <kglobalaccel_component_interface.h>
#include <kglobalaccel_interface.h>

#include "kcmkeys_debug.h"

static QStringList buildActionId(const QString &componentUnique, const QString &componentFriendly, const QString &actionUnique, const QString &actionFriendly)
{
    QStringList actionId{"", "", "", ""};
    actionId[KGlobalAccel::ComponentUnique] = componentUnique;
    actionId[KGlobalAccel::ComponentFriendly] = componentFriendly;
    actionId[KGlobalAccel::ActionUnique] = actionUnique;
    actionId[KGlobalAccel::ActionFriendly] = actionFriendly;
    return actionId;
}

static void writeModifiers(ModifierOnlyShortcutsSettings *settings, Qt::KeyboardModifiers modifiers, const QStringList &value)
{
    if (modifiers & Qt::ShiftModifier) {
        settings->setShift(value);
    }
    if (modifiers & Qt::AltModifier) {
        settings->setAlt(value);
    }
    if (modifiers & Qt::ControlModifier) {
        settings->setControl(value);
    }
    if (modifiers & Qt::MetaModifier) {
        settings->setMeta(value);
    }
}

static std::pair<QString, QString> getIdsforModifier(ModifierOnlyShortcutsSettings *settings, Qt::KeyboardModifier modifier)
{
    QStringList value;
    switch (modifier) {
    case Qt::ShiftModifier:
        value = settings->shift();
        break;
        case Qt::Shift
    }
}

GlobalAccelModel::GlobalAccelModel(KGlobalAccelInterface *interface, QObject *parent)
    :
            BaseModel(parent), m_globalAccelInterface{interface}
            {
            }

QVariant GlobalAccelModel::data(const QModelIndex &index, int role) const
{
    if (role == SupportsMultipleKeysRole) {
        return false;
    }
    if (role == SupportsModifierOnlyShortcutsRole) {
        return true;
    }
    if (role == ModifierOnlyShortcutsRole) {
        if (index.parent().isValid()) {
            const Component &component = m_components[index.parent().row()];
            const Action &action = component.actions[index.row()];
            return QVariant::fromValue(m_modiferOnlyShortcutsMap.value(index));
        }
    }
    return BaseModel::data(index, role);
}

bool GlobalAccelModel::isDefault() const
{
    return m_modifieronlyShortcuts.isDefaults() && BaseModel::isDefault();
}

bool GlobalAccelModel::needsSave() const
{
    return m_modifieronlyShortcuts.isSaveNeeded() && BaseModel::needsSave();
}

void GlobalAccelModel::load()
{
    if (!m_globalAccelInterface->isValid()) {
        return;
    }
    beginResetModel();
    m_modifieronlyShortcuts.load();
    m_components.clear();
    auto componentsWatcher = new QDBusPendingCallWatcher(m_globalAccelInterface->allComponents());
    connect(componentsWatcher, &QDBusPendingCallWatcher::finished, this, [this](QDBusPendingCallWatcher *componentsWatcher) {
        QDBusPendingReply<QList<QDBusObjectPath>> componentsReply = *componentsWatcher;
        componentsWatcher->deleteLater();
        if (componentsReply.isError()) {
            genericErrorOccured(QStringLiteral("Error while calling allComponents()"), componentsReply.error());
            endResetModel();
            return;
        }
        const QList<QDBusObjectPath> componentPaths = componentsReply.value();
        int *pendingCalls = new int;
        *pendingCalls = componentPaths.size();
        for (const auto &componentPath : componentPaths) {
            const QString path = componentPath.path();
            KGlobalAccelComponentInterface component(m_globalAccelInterface->service(), path, m_globalAccelInterface->connection());
            auto watcher = new QDBusPendingCallWatcher(component.allShortcutInfos());
            connect(watcher, &QDBusPendingCallWatcher::finished, this, [path, pendingCalls, this](QDBusPendingCallWatcher *watcher) {
                QDBusPendingReply<QList<KGlobalShortcutInfo>> reply = *watcher;
                if (reply.isError()) {
                    genericErrorOccured(QStringLiteral("Error while calling allShortCutInfos of") + path, reply.error());
                } else if (!reply.value().isEmpty()) {
                    m_components.push_back(loadComponent(reply.value()));
                }
                watcher->deleteLater();
                if (--*pendingCalls == 0) {
                    QCollator collator;
                    collator.setCaseSensitivity(Qt::CaseInsensitive);
                    collator.setNumericMode(true);
                    std::sort(m_components.begin(), m_components.end(), [&](const Component &c1, const Component &c2) {
                        return c1.type != c2.type ? c1.type < c2.type : collator.compare(c1.displayName, c2.displayName) < 0;
                    });
                    matchModifierOnlyShortcuts();
                    endResetModel();
                    delete pendingCalls;
                }
            });
        }
    });
}

Component GlobalAccelModel::loadComponent(const QList<KGlobalShortcutInfo> &info)
{
    const QString &componentUnique = info[0].componentUniqueName();
    const QString &componentFriendly = info[0].componentFriendlyName();

    KService::Ptr service = KService::serviceByStorageId(componentUnique);
    // Not a normal desktop file but maybe specific file in kglobalaccel dir
    if (!service && componentUnique.endsWith(QLatin1String(".desktop"))) {
        service = new KService(QStandardPaths::locate(QStandardPaths::GenericDataLocation, QStringLiteral("kglobalaccel/") + componentUnique));
    }

    if (!service) {
        // Do we have a service with that name?
        auto filter = [componentUnique, componentFriendly](const KService::Ptr service) {
            return service->name() == componentUnique || service->name() == componentFriendly;
        };

        const KService::List services = KApplicationTrader::query(filter);
        service = services.value(0, KService::Ptr());
    }
    const QString type = service && service->isApplication() ? i18n("Applications") : i18n("System Services");
    QString icon;

    static const QHash<QString, QString> hardCodedIcons = {
        {"ActivityManager", "preferences-desktop-activities"},
        {"KDE Keyboard Layout Switcher", "input-keyboard"},
        {"org_kde_powerdevil", "preferences-system-power-management"},
    };

    if (service && !service->icon().isEmpty()) {
        icon = service->icon();
    } else if (hardCodedIcons.contains(componentUnique)) {
        icon = hardCodedIcons[componentUnique];
    } else {
        icon = componentUnique;
    }

    Component c{componentUnique, componentFriendly, type, icon, {}, false, false};
    for (const auto &actionInfo : info) {
        const QString &actionUnique = actionInfo.uniqueName();
        const QString &actionFriendly = actionInfo.friendlyName();
        Action action;
        action.id = actionUnique;
        action.displayName = actionFriendly;
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
    }
    QCollator collator;
    collator.setCaseSensitivity(Qt::CaseInsensitive);
    collator.setNumericMode(true);
    std::sort(c.actions.begin(), c.actions.end(), [&](const Action &s1, const Action &s2) {
        return collator.compare(s1.displayName, s2.displayName) < 0;
    });
    return c;
}

void GlobalAccelModel::matchModifierOnlyShortcuts()
{
    const auto modifiers = std::array{std::pair{Qt::ShiftModifier, m_modifieronlyShortcuts.shift()},
                                      std::pair{Qt::ControlModifier, m_modifieronlyShortcuts.control()},
                                      std::pair{Qt::AltModifier, m_modifieronlyShortcuts.alt()},
                                      std::pair{Qt::MetaModifier, m_modifieronlyShortcuts.meta()}};
    for (const auto &modifier : modifiers) {
        if (modifier.second.size() != 5) {
            continue;
        }
        const auto &service = modifier.second.at(0);
        const auto &path = modifier.second.at(1);
        const auto &interface = modifier.second.at(2);
        const auto &method = modifier.second.at(3);
        const auto &argument = modifier.second.at(4);

        if (service != m_globalAccelInterface->service() || interface != KGlobalAccelComponentInterface::staticInterfaceName()
            || method != QLatin1String("invokeShortcut")) {
            continue;
        }

        if (!path.startsWith(QLatin1String("/component"))) {
            continue;
        }

        const auto componentName = path.mid(strlen("/component"));

        auto component = std::find_if(m_components.cbegin(), m_components.cend(), [componentName](const Component &component) {
            return component.id == componentName;
        });
        if (component == m_components.cend()) {
            continue;
        }

        auto action = std::find_if(component->actions.cbegin(), component->actions.cend(), [argument](const Action &action) {
            return action.id == argument;
        });
        if (action == component->actions.cend()) {
            continue;
        }

        QModelIndex i = index(action - component->actions.begin(), 0, index(component - m_components.cbegin(), 0));
        m_modiferOnlyShortcutsMap[QPersistentModelIndex(i)].setFlag(modifier.first);
    }
}

void GlobalAccelModel::save()
{
    m_modifieronlyShortcuts.save();
    for (auto it = m_components.rbegin(); it != m_components.rend(); ++it) {
        if (it->pendingDeletion) {
            removeComponent(*it);
            continue;
        }
        for (auto &action : it->actions) {
            if (action.initialShortcuts != action.activeShortcuts) {
                const QStringList actionId = buildActionId(it->id, it->displayName, action.id, action.displayName);
                // TODO: pass action.activeShortcuts to m_globalAccelInterface->setForeignShortcut() as a QSet<QKeySequence>
                // or QList<QKeySequence>?
                QList<QKeySequence> keys;
                keys.reserve(action.activeShortcuts.size());
                for (const QKeySequence &key : qAsConst(action.activeShortcuts)) {
                    keys.append(key);
                }
                qCDebug(KCMKEYS) << "Saving" << actionId << action.activeShortcuts << keys;
                auto reply = m_globalAccelInterface->setForeignShortcutKeys(actionId, keys);
                reply.waitForFinished();
                if (!reply.isValid()) {
                    qCCritical(KCMKEYS) << "Error while saving";
                    if (reply.error().isValid()) {
                        qCCritical(KCMKEYS) << reply.error().name() << reply.error().message();
                    }
                    Q_EMIT errorOccured(i18nc("%1 is the name of the component, %2 is the action for which saving failed",
                                              "Error while saving shortcut %1: %2",
                                              it->displayName,
                                              it->displayName));
                } else {
                    action.initialShortcuts = action.activeShortcuts;
                }
            }
        }
    }
}

void GlobalAccelModel::defaults()
{
    m_modifieronlyShortcuts.setDefaults();
    matchModifierOnlyShortcuts();
    BaseModel::defaults();
}

void GlobalAccelModel::exportToConfig(const KConfigBase &config)
{
    for (const auto &component : qAsConst(m_components)) {
        if (component.checked) {
            KConfigGroup mainGroup(&config, component.id);
            KConfigGroup group(&mainGroup, "Global Shortcuts");
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
        auto component = std::find_if(m_components.begin(), m_components.end(), [&](const Component &c) {
            return c.id == componentGroupName;
        });
        if (component == m_components.end()) {
            qCWarning(KCMKEYS) << "Ignoring unknown component" << componentGroupName;
            continue;
        }
        KConfigGroup componentGroup(&config, componentGroupName);
        if (!componentGroup.hasGroup("Global Shortcuts")) {
            qCWarning(KCMKEYS) << "Group" << componentGroupName << "has no shortcuts group";
            continue;
        }
        KConfigGroup shortcutsGroup(&componentGroup, "Global Shortcuts");
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
                const QModelIndex i = index(action - component->actions.begin(), 0, index(component - m_components.begin(), 0));
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

    // Register a dummy action to trigger kglobalaccel to parse the desktop file
    QStringList actionId = buildActionId(desktopName, displayName, QString(), QString());
    m_globalAccelInterface->doRegister(actionId);
    m_globalAccelInterface->unRegister(actionId);
    QCollator collator;
    collator.setCaseSensitivity(Qt::CaseInsensitive);
    collator.setNumericMode(true);
    auto pos = std::lower_bound(m_components.begin(), m_components.end(), displayName, [&](const Component &c, const QString &name) {
        return c.type != i18n("System Services") && collator.compare(c.displayName, name) < 0;
    });
    auto watcher = new QDBusPendingCallWatcher(m_globalAccelInterface->getComponent(desktopName));
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [=] {
        QDBusPendingReply<QDBusObjectPath> reply = *watcher;
        watcher->deleteLater();
        if (!reply.isValid()) {
            genericErrorOccured(QStringLiteral("Error while calling objectPath of added application") + desktopName, reply.error());
            return;
        }
        KGlobalAccelComponentInterface component(m_globalAccelInterface->service(), reply.value().path(), m_globalAccelInterface->connection());
        auto infoWatcher = new QDBusPendingCallWatcher(component.allShortcutInfos());
        connect(infoWatcher, &QDBusPendingCallWatcher::finished, this, [=] {
            QDBusPendingReply<QList<KGlobalShortcutInfo>> infoReply = *infoWatcher;
            infoWatcher->deleteLater();
            if (!infoReply.isValid()) {
                genericErrorOccured(QStringLiteral("Error while calling allShortCutInfos on new component") + desktopName, infoReply.error());
                return;
            }
            if (infoReply.value().isEmpty()) {
                qCWarning(KCMKEYS()) << "New component has no shortcuts:" << reply.value().path();
                Q_EMIT errorOccured(i18nc("%1 is the name of an application", "Error while adding %1, it seems it has no actions."));
            }
            qCDebug(KCMKEYS) << "inserting at " << pos - m_components.begin();
            beginInsertRows(QModelIndex(), pos - m_components.begin(), pos - m_components.begin());
            auto c = loadComponent(infoReply.value());
            m_components.insert(pos, c);
            endInsertRows();
        });
    });
}

void GlobalAccelModel::removeComponent(const Component &component)
{
    const QString &uniqueName = component.id;
    auto componentReply = m_globalAccelInterface->getComponent(uniqueName);
    componentReply.waitForFinished();
    if (!componentReply.isValid()) {
        genericErrorOccured(QStringLiteral("Error while calling objectPath of component") + uniqueName, componentReply.error());
        return;
    }
    KGlobalAccelComponentInterface componentInterface(m_globalAccelInterface->service(), componentReply.value().path(), m_globalAccelInterface->connection());
    qCDebug(KCMKEYS) << "Cleaning up component at" << componentReply.value().path();
    auto cleanUpReply = componentInterface.cleanUp();
    cleanUpReply.waitForFinished();
    if (!cleanUpReply.isValid()) {
        genericErrorOccured(QStringLiteral("Error while calling cleanUp of component") + uniqueName, cleanUpReply.error());
        return;
    }
    auto it = std::find_if(m_components.begin(), m_components.end(), [&](const Component &c) {
        return c.id == uniqueName;
    });
    const int row = it - m_components.begin();

    if (auto it = std::remove_if(m_modiferOnlyShortcutsMap.begin()
    beginRemoveRows(QModelIndex(), row, row);
    m_components.remove(row);
    endRemoveRows();
}

void GlobalAccelModel::toggleModifier(const QModelIndex &index, Qt::KeyboardModifier modifier)
{
    if (!checkIndex(index, QAbstractItemModel::CheckIndexOption::IndexIsValid) || !index.parent().isValid()) {
        return;
    }

    const bool used = m_modifieronlyShortcuts->

                      if (used != m_modiferOnlyShortcutsMap.keyValueEnd())
    {
        used->second.setFlag(modifier, false);
        Q_EMIT dataChanged(us);
    }

    const Component &component = m_components[index.parent().row()];
    const Action &action = component.actions[index.row()];
    m_modiferOnlyShortcutsMap[{component.id, action.id}] = value.value<Qt::KeyboardModifiers>();
}

void GlobalAccelModel::genericErrorOccured(const QString &description, const QDBusError &error)
{
    qCCritical(KCMKEYS) << description;
    if (error.isValid()) {
        qCCritical(KCMKEYS) << error.name() << error.message();
    }
    Q_EMIT this->errorOccured(i18n("Error while communicating with the global shortcuts service"));
}
