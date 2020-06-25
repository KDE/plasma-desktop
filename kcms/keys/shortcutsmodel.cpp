/*
 * Copyright (C) 2020  David Redondo <david@david-redondo.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "shortcutsmodel.h"

#include <algorithm>

#include <QIcon>
#include <QDBusPendingCallWatcher>

#include <KDesktopFile>
#include <KConfigGroup>
#include <KGlobalAccel>
#include <kglobalaccel_interface.h>
#include <kglobalaccel_component_interface.h>
#include <KGlobalShortcutInfo>
#include <KLocalizedString>
#include <KService>
#include <KServiceTypeTrader>

#include "kcmkeys_debug.h"

static QStringList buildActionId(const QString &componentUnique, const QString &componentFriendly,
                              const QString &actionUnique, const QString &actionFriendly)
{
    QStringList actionId{"", "", "", ""};
    actionId[KGlobalAccel::ComponentUnique] = componentUnique;
    actionId[KGlobalAccel::ComponentFriendly] = componentFriendly;
    actionId[KGlobalAccel::ActionUnique] = actionUnique;
    actionId[KGlobalAccel::ActionFriendly] = actionFriendly;
    return actionId;
}


ShortcutsModel::ShortcutsModel(KGlobalAccelInterface *interface, QObject *parent)
 : QAbstractItemModel(parent)
 , m_globalAccelInterface{interface}
{
}

void ShortcutsModel::load()
{
    if (!m_globalAccelInterface->isValid()) {
        return;
    }
    beginResetModel();
    m_components.clear();
    auto componentsWatcher = new  QDBusPendingCallWatcher( m_globalAccelInterface->allComponents());
    connect(componentsWatcher, &QDBusPendingCallWatcher::finished, this, [this] (QDBusPendingCallWatcher *componentsWatcher) {
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
            connect(watcher, &QDBusPendingCallWatcher::finished, this, [path, pendingCalls, this] (QDBusPendingCallWatcher *watcher){
                QDBusPendingReply<QList<KGlobalShortcutInfo>> reply = *watcher;
                if (reply.isError()) {
                    genericErrorOccured(QStringLiteral("Error while calling allShortCutInfos of") + path, reply.error());
                } else {
                    m_components.push_back(loadComponent(reply.value()));
                }
                watcher->deleteLater();
                if (--*pendingCalls == 0) {
                    QCollator collator;
                    collator.setCaseSensitivity(Qt::CaseInsensitive);
                    collator.setNumericMode(true);
                    std::sort(m_components.begin(), m_components.end(), [&](const Component &c1, const Component &c2){
                        return c1.type != c2.type ? c1.type < c2.type : collator.compare(c1.friendlyName, c2.friendlyName) < 0;
                    });
                    endResetModel();
                    delete pendingCalls;
                }
            });
        }
    });
}

Component ShortcutsModel::loadComponent(const QList<KGlobalShortcutInfo> &info)
{
    const QString &componentUnique = info[0].componentUniqueName();
    const QString &componentFriendly = info[0].componentFriendlyName();
    KService::Ptr service = KService::serviceByStorageId(componentUnique);
    if (!service) {
        // Do we have an application with that name?
        const KService::List apps = KServiceTypeTrader::self()->query(QStringLiteral("Application"),
                QStringLiteral("Name == '%1' or Name == '%2'").arg(componentUnique, componentFriendly));
        if(!apps.isEmpty()) {
            service = apps[0];
        }
    }
    const QString type = service && service->isApplication() ? i18n("Applications") : i18n("System Services");
    const QString icon = service && !service->icon().isEmpty() ? service->icon() : componentUnique;
    Component c{componentUnique, componentFriendly, type, icon, QVector<Shortcut>(), false, false};
    for (const auto &action : info) {
        const QString &actionUnique = action.uniqueName();
        const QString &actionFriendly = action.friendlyName();
        Shortcut shortcut;
        shortcut.uniqueName = actionUnique;
        shortcut.friendlyName = actionFriendly;
        const QList<QKeySequence> defaultShortcuts = action.defaultKeys();
        for (const auto  &keySequence : defaultShortcuts) {
            if (!keySequence.isEmpty()) {
                shortcut.defaultShortcuts.insert(keySequence);
            }
        }
        const QList<QKeySequence> activeShortcuts = action.keys();
        for (const QKeySequence &keySequence : activeShortcuts) {
            if (!keySequence.isEmpty()) {
                shortcut.activeShortcuts.insert(keySequence);
            }
        }
        shortcut.initialShortcuts = shortcut.activeShortcuts;
        c.shortcuts.push_back(shortcut);
    }
    QCollator collator;
    collator.setCaseSensitivity(Qt::CaseInsensitive);
    collator.setNumericMode(true);
    std::sort(c.shortcuts.begin(), c.shortcuts.end(), [&] (const Shortcut &s1, const Shortcut &s2) {
        return collator.compare(s1.friendlyName, s2.friendlyName) < 0;
    });
    return c;
}

void ShortcutsModel::save()
{
    for (auto it = m_components.rbegin(); it != m_components.rend(); ++it) {
        if (it->pendingDeletion) {
            removeComponent(*it);
            continue;
        }
        for (auto& shortcut : it->shortcuts) {
            if (shortcut.initialShortcuts != shortcut.activeShortcuts) {
                const QStringList actionId = buildActionId(it->uniqueName, it->friendlyName,
                        shortcut.uniqueName, shortcut.friendlyName);
                //operator int of QKeySequence
                QList<int> keys(shortcut.activeShortcuts.cbegin(), shortcut.activeShortcuts.cend());
                qCDebug(KCMKEYS) << "Saving" << actionId << shortcut.activeShortcuts << keys;
                auto reply = m_globalAccelInterface->setForeignShortcut(actionId, keys);
                reply.waitForFinished();
                if (!reply.isValid()) {
                    qCCritical(KCMKEYS) << "Error while saving";
                    if (reply.error().isValid()) {
                        qCCritical(KCMKEYS) << reply.error().name() << reply.error().message();
                    }
                    emit errorOccured(i18nc("%1 is the name of the component, %2 is the action for which saving failed",
                        "Error while saving shortcut %1: %2", it->friendlyName, shortcut.friendlyName));
                } else {
                    shortcut.initialShortcuts = shortcut.activeShortcuts;
                }
            }
        }
    }
}

void ShortcutsModel::defaults() {
    for (auto component = m_components.begin(); component != m_components.end(); ++component) {
        auto componentIndex = index(component - m_components.begin(), 0);
        for (auto shortcut = component->shortcuts.begin(); shortcut != component->shortcuts.end(); ++shortcut) {
            shortcut->activeShortcuts = shortcut->defaultShortcuts;
        }
        emit dataChanged(index(0, 0, componentIndex), index(component->shortcuts.size(), 0, componentIndex),
            {ActiveShortcutsRole, CustomShortcutsRole});
    }
}

bool ShortcutsModel::needsSave() const
{
    for (const auto& component : m_components) {
        if (component.pendingDeletion) {
            return true;
        }
        for (const auto& shortcut : component.shortcuts) {
            if (shortcut.initialShortcuts != shortcut.activeShortcuts) {
                return true;
            }
        }
    }
    return false;
}

bool ShortcutsModel::isDefault() const
{
   for (const auto& component : m_components) {
        for (const auto& shortcut : component.shortcuts) {
            if (shortcut.defaultShortcuts != shortcut.activeShortcuts) {
                return false;
            }
        }
   }
   return true;
}

QModelIndex ShortcutsModel::index(int row, int column, const QModelIndex &parent) const
{
    if (row < 0 || column != 0) {
        return QModelIndex();
    }
    if (parent.isValid() && row < rowCount(parent) && column == 0) {
        return createIndex(row, column, parent.row() + 1);
    } else if (column == 0 && row < m_components.size()) {
       return createIndex(row, column, nullptr);
    }
    return QModelIndex();
}

QModelIndex ShortcutsModel::parent(const QModelIndex &child) const
{
    if (child.internalId()) {
        return createIndex(child.internalId() - 1, 0, nullptr);
    }
    return QModelIndex();
}

int ShortcutsModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        if (parent.parent().isValid()) {
            return 0;
        }
        return m_components[parent.row()].shortcuts.size();
    }
    return m_components.size();
}

int ShortcutsModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 1;
}

QVariant ShortcutsModel::data(const QModelIndex &index, int role) const
{
    if (!checkIndex(index)) {
        return QVariant();
    }

    if (index.parent().isValid()) {
        const Shortcut &shortcut = m_components[index.parent().row()].shortcuts[index.row()];
        switch (role) {
        case Qt::DisplayRole:
            return shortcut.friendlyName.isEmpty() ? shortcut.uniqueName : shortcut.friendlyName;
        case ActionRole:
            return shortcut.uniqueName;
        case ActiveShortcutsRole:
            return QVariant::fromValue(shortcut.activeShortcuts);
        case DefaultShortcutsRole: 
            return QVariant::fromValue(shortcut.defaultShortcuts);
        case CustomShortcutsRole: {
            auto  shortcuts = shortcut.activeShortcuts;
            return QVariant::fromValue(shortcuts.subtract(shortcut.defaultShortcuts));
        }
        }
        return QVariant();
    }
    const Component &component = m_components[index.row()];
    switch(role) {
    case Qt::DisplayRole:
        return component.friendlyName;
    case Qt::DecorationRole:
        return component.icon;
    case SectionRole:
        return component.type;
    case ComponentRole:
        return component.uniqueName;
    case CheckedRole:
        return component.checked;
    case PendingDeletionRole:
        return component.pendingDeletion;
    }
    return QVariant();
}

bool ShortcutsModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!checkIndex(index) || index.parent().isValid()) {
        return false;
    }
    switch (role) {
    case CheckedRole:
        m_components[index.row()].checked = value.toBool();
        emit dataChanged(index, index, {CheckedRole});
        return true;
    case PendingDeletionRole:
        m_components[index.row()].pendingDeletion = value.toBool();
        emit dataChanged(index, index, {PendingDeletionRole});
        return true;
    }
    return false;
}

QHash<int, QByteArray> ShortcutsModel::roleNames() const
{
 return {
        {Qt::DisplayRole, QByteArrayLiteral("display")},
        {Qt::DecorationRole, QByteArrayLiteral("decoration")},
        {SectionRole, QByteArrayLiteral("section")},
        {ComponentRole, QByteArrayLiteral("component")},
        {ActiveShortcutsRole, QByteArrayLiteral("activeShortcuts")},
        {DefaultShortcutsRole, QByteArrayLiteral("defaultShortcuts")},
        {CustomShortcutsRole, QByteArrayLiteral("customShortcuts")},
        {CheckedRole, QByteArrayLiteral("checked")},
        {PendingDeletionRole, QByteArrayLiteral("pendingDeletion")}
        };
}

void ShortcutsModel::toggleDefaultShortcut(const QModelIndex &index, const QKeySequence &shortcut, bool enabled)
{
   if (!checkIndex(index) || !index.parent().isValid())  {
        return;
    }
    qCDebug(KCMKEYS) << "Default shortcut" << index << shortcut << enabled;
    Shortcut &s = m_components[index.parent().row()].shortcuts[index.row()];
    if (enabled) {
        s.activeShortcuts.insert(shortcut);
    } else {
        s.activeShortcuts.remove(shortcut);
    }
    emit dataChanged(index, index, {ActiveShortcutsRole, DefaultShortcutsRole});
}

void ShortcutsModel::addShortcut(const QModelIndex &index, const QKeySequence &shortcut)
{
     if (!checkIndex(index) || !index.parent().isValid())  {
        return;
    }
    if (shortcut.isEmpty()) {
        return;
    }
    qCDebug(KCMKEYS) << "Adding shortcut" << index << shortcut;
    Shortcut &s = m_components[index.parent().row()].shortcuts[index.row()];
    s.activeShortcuts.insert(shortcut);
    emit dataChanged(index, index, {ActiveShortcutsRole, CustomShortcutsRole});
}

void ShortcutsModel::disableShortcut(const QModelIndex &index, const QKeySequence &shortcut)
{
    if (!checkIndex(index) || !index.parent().isValid())  {
        return;
    }
    qCDebug(KCMKEYS) << "Disabling shortcut" << index << shortcut;
    Shortcut &s = m_components[index.parent().row()].shortcuts[index.row()];
    s.activeShortcuts.remove(shortcut);
    emit dataChanged(index, index, {ActiveShortcutsRole, CustomShortcutsRole});

}

void ShortcutsModel::changeShortcut(const QModelIndex &index, const QKeySequence &oldShortcut, const QKeySequence &newShortcut)
{
    if (!checkIndex(index) || !index.parent().isValid()) {
        return;
    }
    if (newShortcut.isEmpty()) {
        return;
    }
    qCDebug(KCMKEYS) << "Changing Shortcut" << index << oldShortcut << " to " << newShortcut;
    Shortcut &s = m_components[index.parent().row()].shortcuts[index.row()];
    s.activeShortcuts.remove(oldShortcut);
    s.activeShortcuts.insert(newShortcut);
    emit dataChanged(index, index, {ActiveShortcutsRole, CustomShortcutsRole});
}

void ShortcutsModel::setShortcuts(const KConfigBase &config)
{
    for (const auto componentGroupName : config.groupList()) {
        auto component = std::find_if(m_components.begin(), m_components.end(), [&] (const Component &c) {
            return c.uniqueName == componentGroupName;
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
        for (const auto& key : shortcutsGroup.keyList()) {
            auto shortcut = std::find_if(component->shortcuts.begin(), component->shortcuts.end(), [&] (const Shortcut &s) {
                return s.uniqueName == key;
            });
            if (shortcut == component->shortcuts.end()) {
                qCWarning(KCMKEYS) << "Ignoring unknown action" << key;
                continue;
            }
            const auto shortcuts = QKeySequence::listFromString(shortcutsGroup.readEntry(key));
            shortcut->activeShortcuts = QSet<QKeySequence>(shortcuts.cbegin(), shortcuts.cend());
        }
    }
    emit dataChanged(index(0, 0), index(0, rowCount()), {ActiveShortcutsRole, CustomShortcutsRole});
}

void ShortcutsModel::addApplication(const QString &desktopFileName, const QString &displayName)
{
    // Register a dummy action to trigger kglobalaccel to parse the desktop file
    QStringList actionId = buildActionId(desktopFileName, displayName, QString(), QString());
    m_globalAccelInterface->doRegister(actionId);
    m_globalAccelInterface->unRegister(actionId);
    QCollator collator;
    collator.setCaseSensitivity(Qt::CaseInsensitive);
    collator.setNumericMode(true);
    auto pos = std::lower_bound(m_components.begin(), m_components.end(), displayName, [&] (const Component &c, const QString &name) {
        return c.type != i18n("System Services") &&  collator.compare(c.friendlyName, name) < 0;
    });
    auto watcher = new QDBusPendingCallWatcher(m_globalAccelInterface->getComponent(desktopFileName));
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [=] {
        QDBusPendingReply<QDBusObjectPath> reply = *watcher;
        watcher->deleteLater();
        if (!reply.isValid()) {
            genericErrorOccured(QStringLiteral("Error while calling objectPath of added application") + desktopFileName, reply.error());
            return;
        }
        KGlobalAccelComponentInterface component(m_globalAccelInterface->service(), reply.value().path(), m_globalAccelInterface->connection());
        auto infoWatcher = new QDBusPendingCallWatcher(component.allShortcutInfos());
        connect(infoWatcher, &QDBusPendingCallWatcher::finished, this, [=] {
            QDBusPendingReply<QList<KGlobalShortcutInfo>> infoReply = *infoWatcher;
            infoWatcher->deleteLater();
            if (!infoReply.isValid()) {
                genericErrorOccured(QStringLiteral("Error while calling allShortCutInfos on new component") + desktopFileName, infoReply.error());
                return;
            }
            qCDebug(KCMKEYS) << "inserting at " << pos - m_components.begin();
            emit beginInsertRows(QModelIndex(), pos - m_components.begin(),  pos - m_components.begin());
            Component c = loadComponent(infoReply.value());
            m_components.insert(pos, c);
            emit endInsertRows();
        });
    });
}

void ShortcutsModel::removeComponent(const Component &component)
{
    const QString &uniqueName = component.uniqueName;
    auto componentReply = m_globalAccelInterface->getComponent(uniqueName);
    componentReply.waitForFinished();
    if (!componentReply.isValid()) {
        genericErrorOccured(QStringLiteral("Error while calling objectPath of component") + uniqueName, componentReply.error());
        return;
    }
    KGlobalAccelComponentInterface componentInterface(m_globalAccelInterface->service(), componentReply.value().path(), m_globalAccelInterface->connection());
    qCDebug(KCMKEYS) << "Cleaning up component at" << componentReply.value();
    auto cleanUpReply = componentInterface.cleanUp();
    cleanUpReply.waitForFinished();
    if (!cleanUpReply.isValid()) {
        genericErrorOccured(QStringLiteral("Error while calling cleanUp of component") + uniqueName, cleanUpReply.error());
        return;
    }
    auto it =  std::find_if(m_components.begin(), m_components.end(), [&](const Component &c) {
        return c.uniqueName == uniqueName;
    });
    const int row = it - m_components.begin();
    beginRemoveRows(QModelIndex(), row, row);
    m_components.remove(row);
    endRemoveRows();
}

void ShortcutsModel::genericErrorOccured(const QString &description, const QDBusError &error)
{
    qCCritical(KCMKEYS) << description;
    if (error.isValid()) {
        qCCritical(KCMKEYS) << error.name() << error.message();
    }
    emit this->errorOccured(i18n("Error while communicating with the global shortcuts service"));
}
