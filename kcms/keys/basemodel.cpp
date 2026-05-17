/*
    SPDX-FileCopyrightText: 2020 David Redondo <kde@david-redondo.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "basemodel.h"

#include "component.h"

#include <KConfigGroup>
#include <KDesktopFile>
#include <KLocalizedString>
#include <KService>

#include <algorithm>

#include "kcmkeys_debug.h"

struct BaseModelPrivate {
    QList<Component> components;
};

namespace
{
QKeySequence suggestedInverseKeys(const QKeySequence &s)
{
    if (s.isEmpty()) {
        return QKeySequence();
    }
    std::array<QKeyCombination, 4> inverse = {s[0], s[1], s[2], s[3]};
    QKeyCombination &last = inverse[s.count() - 1]; // get reference from an array, this doesn't work on a QKeySequence

    if (last.key() == Qt::Key::Key_Left) {
        last = last.keyboardModifiers() | Qt::Key::Key_Right;
    } else if (last.key() == Qt::Key::Key_Right) {
        last = last.keyboardModifiers() | Qt::Key::Key_Left;
    } else if (last.key() == Qt::Key::Key_Up) {
        last = last.keyboardModifiers() | Qt::Key::Key_Down;
    } else if (last.key() == Qt::Key::Key_Down) {
        last = last.keyboardModifiers() | Qt::Key::Key_Up;
    } else if (last.key() == Qt::Key::Key_PageUp) {
        last = last.keyboardModifiers() | Qt::Key::Key_PageDown;
    } else if (last.key() == Qt::Key::Key_PageDown) {
        last = last.keyboardModifiers() | Qt::Key::Key_PageUp;
    } else {
        last = last.key() | (last.keyboardModifiers() ^ Qt::KeyboardModifier::ShiftModifier); // toggle Shift
    }
    return last == s[s.count() - 1] ? QKeySequence() : QKeySequence(inverse[0], inverse[1], inverse[2], inverse[3]);
}
} // namespace

InverseActionReassignmentSuggestion::InverseActionReassignmentSuggestion(const BaseModel *model, const QString &inverseId, const Data &data)
    : QObject() // QML is meant to take ownership
    , m_inverseId(inverseId)
    , m_data(data)
{
}

QString InverseActionReassignmentSuggestion::shortcutNativeText(const QKeySequence &s) const
{
    return s.toString(QKeySequence::NativeText);
}

QModelIndex InverseActionReassignmentSuggestion::inverseActionModelIndex() const
{
    return m_data.inverseIndex;
}

QString InverseActionReassignmentSuggestion::inverseDisplayName() const
{
    return m_data.inverseDisplayName;
}

QKeySequence InverseActionReassignmentSuggestion::shortcutToReplace() const
{
    return m_data.shortcutToReplace;
}

QKeySequence InverseActionReassignmentSuggestion::shortcutSuggestion() const
{
    return m_data.shortcutSuggestion;
}

BaseModel::BaseModel(QObject *parent)
    : QAbstractItemModel(parent)
    , d(new BaseModelPrivate)
{
}

BaseModel::~BaseModel()
{
}

void BaseModel::addShortcut(const QModelIndex &index, const QKeySequence &shortcut)
{
    if (!checkIndex(index, QAbstractItemModel::CheckIndexOption::IndexIsValid) || !index.parent().isValid()) {
        return;
    }
    if (shortcut.isEmpty()) {
        return;
    }
    qCDebug(KCMKEYS) << "Adding shortcut" << index << shortcut;
    Action &a = d->components[index.parent().row()].actions[index.row()];
    a.activeShortcuts.insert(shortcut);
    Q_EMIT dataChanged(index, index, {ActiveShortcutsRole, CustomShortcutsRole, IsDefaultRole});
    Q_EMIT dataChanged(index.parent(), index.parent(), {IsDefaultRole});
}

void BaseModel::disableShortcut(const QModelIndex &index, const QKeySequence &shortcut)
{
    if (!checkIndex(index, QAbstractItemModel::CheckIndexOption::IndexIsValid) || !index.parent().isValid()) {
        return;
    }
    qCDebug(KCMKEYS) << "Disabling shortcut" << index << shortcut;
    Action &a = d->components[index.parent().row()].actions[index.row()];
    a.activeShortcuts.remove(shortcut);
    Q_EMIT dataChanged(index, index, {ActiveShortcutsRole, CustomShortcutsRole, IsDefaultRole});
    Q_EMIT dataChanged(index.parent(), index.parent(), {IsDefaultRole});
}

void BaseModel::changeShortcut(const QModelIndex &index, const QKeySequence &oldShortcut, const QKeySequence &newShortcut)
{
    if (!checkIndex(index, QAbstractItemModel::CheckIndexOption::IndexIsValid) || !index.parent().isValid()) {
        return;
    }
    if (newShortcut.isEmpty() || oldShortcut == newShortcut) {
        return;
    }
    qCDebug(KCMKEYS) << "Changing Shortcut" << index << oldShortcut << " to " << newShortcut;
    auto &actions = d->components[index.parent().row()].actions;

    Action &a = actions[index.row()];
    a.activeShortcuts.remove(oldShortcut);
    a.activeShortcuts.insert(newShortcut);
    Q_EMIT dataChanged(index, index, {ActiveShortcutsRole, CustomShortcutsRole});

    const auto inverse = std::find_if(actions.cbegin(), actions.cend(), [id = a.inverseAction](const Action &other) {
        return other.id == id;
    });
    if (inverse != actions.cend()) {
        const QKeySequence inverseOldKeys = suggestedInverseKeys(oldShortcut);
        if (inverse->activeShortcuts.contains(inverseOldKeys)) {
            const QModelIndex inverseIndex = index.siblingAtRow(inverse - actions.cbegin());
            const QKeySequence suggestedKeys = suggestedInverseKeys(newShortcut);
            a.inverseActionReassignmentSuggestion =
                InverseActionReassignmentSuggestion::Data{inverse->displayName, inverseIndex, suggestedKeys, inverseOldKeys};
            Q_EMIT dataChanged(index, index, {InverseActionReassignmentSuggestionRole});
        }
    }
}

void BaseModel::defaults()
{
    for (int i = 0; i < d->components.size(); ++i) {
        const auto componentIndex = index(i, 0);
        for (auto action_it = d->components[i].actions.begin(); action_it != d->components[i].actions.end(); ++action_it) {
            action_it->activeShortcuts = action_it->defaultShortcuts;
        }
        Q_EMIT dataChanged(index(0, 0, componentIndex),
                           index(d->components[i].actions.size() - 1, 0, componentIndex),
                           {ActiveShortcutsRole, CustomShortcutsRole, IsDefaultRole});
    }
    Q_EMIT dataChanged(index(0, 0), index(d->components.size() - 1, 0), {IsDefaultRole});
}

bool BaseModel::needsSave() const
{
    for (const auto &component : std::as_const(d->components)) {
        if (component.pendingDeletion) {
            return true;
        }
        for (const auto &action : std::as_const(component.actions)) {
            if (action.initialShortcuts != action.activeShortcuts) {
                return true;
            }
        }
    }
    return false;
}

bool BaseModel::isDefault() const
{
    for (const auto &component : std::as_const(d->components)) {
        for (const auto &action : std::as_const(component.actions)) {
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
    if (parent.isValid() && row < rowCount(parent)) {
        return createIndex(row, column, parent.row() + 1);
    } else if (row < d->components.size()) {
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
        return d->components[parent.row()].actions.size();
    }
    return d->components.size();
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
        const Action &action = d->components[index.parent().row()].actions[index.row()];
        switch (role) {
        case Qt::DisplayRole: {
            KDesktopFile desktopFile(d->components[index.parent().row()].id);
            KConfigGroup cg = desktopFile.desktopGroup();
            if (cg.readEntry<bool>("X-KDE-GlobalAccel-CommandShortcut", false)) {
                return cg.readEntry("Exec").replace("%%", "%");
            }
            if (action.id == "_launch") {
                return i18nc("@title:group Launch app", "Launch");
            }
            return action.displayName.isEmpty() ? action.id : action.displayName;
        }
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
        case IsDefaultRole:
            return action.activeShortcuts == action.defaultShortcuts;
        case SupportsMultipleKeysRole:
            return true;
        case InverseActionReassignmentSuggestionRole:
            if (action.inverseActionReassignmentSuggestion.has_value()) {
                return QVariant::fromValue(new InverseActionReassignmentSuggestion(this, action.inverseAction, *action.inverseActionReassignmentSuggestion));
            }
            return QVariant::fromValue(static_cast<InverseActionReassignmentSuggestion *>(nullptr));
        }
        return QVariant();
    }
    const Component &component = d->components[index.row()];
    switch (role) {
    case Qt::DisplayRole: {
        KDesktopFile desktopFile(component.id);
        KConfigGroup cg = desktopFile.desktopGroup();
        if (cg.readEntry<bool>("X-KDE-GlobalAccel-CommandShortcut", false)) {
            return cg.readEntry("Name");
        }

        // Override the names for Plasma and KWin here because their KAboutData strings are
        // rather technical, but aren't strictly speaking wrong, so they won't be changed there.
        if (component.id == QLatin1String("kwin")) {
            return i18nc("@item:intable", "Window Management");
        }
        if (component.id == QLatin1String("plasmashell")) {
            return i18nc("@item:intable", "Plasma Workspace");
        }

        return component.displayName;
    }
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
    case IsDefaultRole:
        return std::all_of(component.actions.begin(), component.actions.end(), [](const Action &action) {
            return action.activeShortcuts == action.defaultShortcuts;
        });
    case IsRemovableRole: {
        // commands can always be removed
        if (component.type == ComponentNS::Command) {
            return true;
        }

        if (component.id.endsWith(".desktop")) {
            // desktopfile-based system services cannot be removed
            // ideally we also wouldn't allow removing runtime-registered
            // system service components, but we cannot reliably detect those
            // and it would make it impossible to remove shortcuts from no longer
            // installed third-party components
            if (component.type == ComponentNS::SystemService) {
                return false;
            }

            auto service = KService::serviceByStorageId(component.id);

            if (!service) {
                // no desktop file in the applications folder, it's a system service, so don't remove
                return false;
            }

            // Check whether the app has shortcuts by default
            // If yes then don't allow removing it or it will reappear automatically
            // If no the user added it explicitly and should be able to remove it again
            bool hasShortcutByDefault = [](const KService::Ptr &service) {
                if (!service->property<QString>(QStringLiteral("X-KDE-Shortcuts")).isEmpty()) {
                    return true;
                }

                const auto actions = service->actions();
                return std::any_of(actions.cbegin(), actions.cend(), [](const KServiceAction &action) {
                    return !action.property<QStringList>(QStringLiteral("X-KDE-Shortcuts")).isEmpty();
                });
            }(service);

            return !hasShortcutByDefault;
        } else {
            // TODO find a way to determine whether runtime registered shortcuts
            // are from core system components and should not be removable
            return true;
        }
    }
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
        if (d->components[index.row()].checked != boolValue) {
            d->components[index.row()].checked = boolValue;
            Q_EMIT dataChanged(index, index, {CheckedRole});
            return true;
        }
        break;
    case PendingDeletionRole:
        if (d->components[index.row()].pendingDeletion != boolValue) {
            d->components[index.row()].pendingDeletion = boolValue;
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
        {IsDefaultRole, QByteArrayLiteral("isDefault")},
        {SupportsMultipleKeysRole, QByteArrayLiteral("supportsMultipleKeys")},
        {IsRemovableRole, QByteArrayLiteral("isRemovable")},
        {InverseActionReassignmentSuggestionRole, QByteArrayLiteral("inverseActionReassignmentSuggestion")},
    };
}

QList<Component> &BaseModel::components()
{
    return d->components;
}

const QList<Component> &BaseModel::components() const
{
    return d->components;
}

#include "moc_basemodel.cpp"
