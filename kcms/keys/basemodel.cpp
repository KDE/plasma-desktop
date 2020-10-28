/*
 * Copyright 2020 David Redondo <kde@david-redondo.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "basemodel.h"

#include "kcmkeys_debug.h"

BaseModel::BaseModel(QObject *parent)
 : QAbstractItemModel(parent)
{
}

void BaseModel::addShortcut(const QModelIndex &index, const QKeySequence &shortcut)
{
   if (!checkIndex(index, QAbstractItemModel::CheckIndexOption::IndexIsValid) || !index.parent().isValid())  {
        return;
    }
    if (shortcut.isEmpty()) {
        return;
    }
    qCDebug(KCMKEYS) << "Adding shortcut" << index << shortcut;
    Action &a = m_components[index.parent().row()].actions[index.row()];
    a.activeShortcuts.insert(shortcut);
    Q_EMIT dataChanged(index, index, {ActiveShortcutsRole, CustomShortcutsRole});
}

void BaseModel::disableShortcut(const QModelIndex &index, const QKeySequence &shortcut)
{
   if (!checkIndex(index, QAbstractItemModel::CheckIndexOption::IndexIsValid) || !index.parent().isValid())  {
        return;
    }
    qCDebug(KCMKEYS) << "Disabling shortcut" << index << shortcut;
    Action &a = m_components[index.parent().row()].actions[index.row()];
    a.activeShortcuts.remove(shortcut);
    Q_EMIT dataChanged(index, index, {ActiveShortcutsRole, CustomShortcutsRole});

}

void BaseModel::changeShortcut(const QModelIndex &index, const QKeySequence &oldShortcut, const QKeySequence &newShortcut)
{
   if (!checkIndex(index, QAbstractItemModel::CheckIndexOption::IndexIsValid) || !index.parent().isValid())  {
        return;
    }
    if (newShortcut.isEmpty()) {
        return;
    }
    qCDebug(KCMKEYS) << "Changing Shortcut" << index << oldShortcut << " to " << newShortcut;
    Action &a = m_components[index.parent().row()].actions[index.row()];
    a.activeShortcuts.remove(oldShortcut);
    a.activeShortcuts.insert(newShortcut);
    Q_EMIT dataChanged(index, index, {ActiveShortcutsRole, CustomShortcutsRole});
}

void BaseModel::defaults()
{
    for (int i = 0;  i < m_components.size(); ++i) {
        const auto componentIndex = index(i, 0);
        for (auto action_it = m_components[i].actions.begin(); action_it != m_components[i].actions.end(); ++action_it) {
            action_it->activeShortcuts = action_it->defaultShortcuts;
        }
        Q_EMIT dataChanged(index(0, 0, componentIndex), index(m_components[i].actions.size() - 1, 0, componentIndex),
            {ActiveShortcutsRole, CustomShortcutsRole});
    }
}

bool BaseModel::needsSave() const
{
    for (const auto& component : qAsConst(m_components)) {
        if (component.pendingDeletion) {
            return true;
        }
        for (const auto& action : qAsConst(component.actions)) {
            if (action.initialShortcuts != action.activeShortcuts) {
                return true;
            }
        }
    }
    return false;
}

bool BaseModel::isDefault() const
{
   for (const auto& component : qAsConst(m_components)) {
        for (const auto& action : qAsConst(component.actions)) {
            if (action.defaultShortcuts != action.activeShortcuts) {
                return false;
            }
        }
   }
   return true;
}

QModelIndex BaseModel::index(int row, int column, const QModelIndex &parent) const
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

QModelIndex BaseModel::parent(const QModelIndex &child) const
{
    if (child.internalId()) {
        return createIndex(child.internalId() - 1, 0, nullptr);
    }
    return QModelIndex();
}

int BaseModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        if (parent.parent().isValid()) {
            return 0;
        }
        return m_components[parent.row()].actions.size();
    }
    return m_components.size();
}

int BaseModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 1;
}

QVariant BaseModel::data(const QModelIndex &index, int role) const
{
    if (!checkIndex(index, QAbstractItemModel::CheckIndexOption::IndexIsValid)) {
        return QVariant();
    }

    if (index.parent().isValid()) {
        const Action &action = m_components[index.parent().row()].actions[index.row()];
        switch (role) {
        case Qt::DisplayRole:
            return action.displayName.isEmpty() ? action.id : action.displayName;
        case ActionRole:
            return action.id;
        case ActiveShortcutsRole:
            return QVariant::fromValue(action.activeShortcuts);
        case DefaultShortcutsRole: 
            return QVariant::fromValue(action.defaultShortcuts);
        case CustomShortcutsRole: {
            auto shortcuts = action.activeShortcuts;
            return QVariant::fromValue(shortcuts.subtract(action.defaultShortcuts));
        }
        case SupportsMultipleKeysRole:
            return true;
        }
        return QVariant();
    }
    const Component &component = m_components[index.row()];
    switch(role) {
    case Qt::DisplayRole:
        return component.displayName;
    case Qt::DecorationRole:
        return component.icon;
    case SectionRole:
        return component.type;
    case ComponentRole:
        return component.id;
    case CheckedRole:
        return component.checked;
    case PendingDeletionRole:
        return component.pendingDeletion;
    }
    return QVariant();
}

bool BaseModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!checkIndex(index, QAbstractListModel::CheckIndexOption::IndexIsValid | QAbstractListModel::CheckIndexOption::ParentIsInvalid)) {
        return false;
    }
    const bool boolValue = value.toBool();
    switch (role) {
    case CheckedRole:
        if (m_components[index.row()].checked != boolValue) {
            m_components[index.row()].checked = boolValue;
            Q_EMIT dataChanged(index, index, {CheckedRole});
            return true;
        }
        break;
    case PendingDeletionRole:
        if (m_components[index.row()].pendingDeletion != boolValue) {
            m_components[index.row()].pendingDeletion = boolValue;
            Q_EMIT dataChanged(index, index, {PendingDeletionRole});
            return true;
        }
        break;
    }
    return false;
}

QHash<int, QByteArray> BaseModel::roleNames() const
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
        {PendingDeletionRole, QByteArrayLiteral("pendingDeletion")},
        {SupportsMultipleKeysRole, QByteArrayLiteral("supportsMultipleKeys")}
        };
}
