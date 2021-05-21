/*
    SPDX-FileCopyrightText: 2020 David Redondo <kde@david-redondo.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "standardshortcutsmodel.h"

#include <QCollator>
#include <QMetaEnum>

#include <KConfigBase>
#include <KConfigGroup>
#include <KLocalizedString>
#include <KStandardShortcut>

#include "kcmkeys_debug.h"

StandardShortcutsModel::StandardShortcutsModel(QObject *parent)
    : BaseModel(parent)
{
}

void StandardShortcutsModel::load()
{
    beginResetModel();
    m_components.clear();
    m_components.resize(6);
    const QString sectionName = i18n("Common Actions");
    auto index = [](KStandardShortcut::Category category) {
        return static_cast<int>(category);
    };
    m_components[index(KStandardShortcut::Category::File)] = Component{"File", i18n("File"), sectionName, "document-multiple", {}, false, false};
    m_components[index(KStandardShortcut::Category::Edit)] = Component{"Edit", i18n("Edit"), sectionName, "edittext", {}, false, false};
    m_components[index(KStandardShortcut::Category::Navigation)] =
        Component{"Navigation", i18n("Navigation"), sectionName, "preferences-desktop-navigation", {}, false, false};
    m_components[index(KStandardShortcut::Category::View)] = Component{"View", i18n("View"), sectionName, "view-preview", {}, false, false};
    m_components[index(KStandardShortcut::Category::Settings)] = Component{"Settings", i18n("Settings"), sectionName, "settings-configure", {}, false, false};
    m_components[index(KStandardShortcut::Category::Help)] = Component{"Help", i18n("Help"), sectionName, "system-help", {}, false, false};
    for (int i = KStandardShortcut::AccelNone + 1; i < KStandardShortcut::StandardShortcutCount; ++i) {
        const auto id = static_cast<KStandardShortcut::StandardShortcut>(i);
        const QList<QKeySequence> activeShortcuts = KStandardShortcut::shortcut(id);
        const QList<QKeySequence> defaultShortcuts = KStandardShortcut::hardcodedDefaultShortcut(id);
        auto listToSet = [](const QList<QKeySequence> &list) {
            return QSet<QKeySequence>{list.cbegin(), list.cend()};
        };
        const Action a{KStandardShortcut::name(id),
                       KStandardShortcut::label(id),
                       listToSet(activeShortcuts),
                       listToSet(defaultShortcuts),
                       listToSet(activeShortcuts)};
        m_components[static_cast<int>(category(id))].actions.append(a);
    }
    QCollator collator;
    collator.setCaseSensitivity(Qt::CaseInsensitive);
    std::for_each(m_components.begin(), m_components.end(), [&](Component &c) {
        std::sort(c.actions.begin(), c.actions.end(), [&](const Action &a1, const Action &a2) {
            return collator.compare(a1.displayName, a2.displayName) < 0;
        });
    });
    std::sort(m_components.begin(), m_components.end(), [&](Component &c1, Component &c2) {
        return collator.compare(c1.displayName, c2.displayName) < 0;
    });
    endResetModel();
}

void StandardShortcutsModel::exportToConfig(const KConfigBase &config)
{
    KConfigGroup group(&config, "StandardShortcuts");
    for (const auto &component : qAsConst(m_components)) {
        if (component.checked) {
            for (const auto &action : component.actions) {
                const QList<QKeySequence> shortcutsList(action.activeShortcuts.cbegin(), action.activeShortcuts.cend());
                group.writeEntry(action.id, QKeySequence::listToString(shortcutsList));
            }
        }
    }
}

void StandardShortcutsModel::importConfig(const KConfigBase &config)
{
    if (!config.hasGroup("StandardShortcuts")) {
        qCDebug(KCMKEYS) << "Config has no StandardShortcuts group";
        return;
    }
    const KConfigGroup group = config.group("StandardShortcuts");
    const QStringList keys = group.keyList();
    for (const auto &key : keys) {
        KStandardShortcut::StandardShortcut id = KStandardShortcut::findByName(key);
        if (id == KStandardShortcut::AccelNone) {
            qCWarning(KCMKEYS) << "Unknown standard shortcut" << key;
            continue;
        }
        QString name;
        switch (category(id)) {
        case KStandardShortcut::Category::File:
            name = "File";
            break;
        case KStandardShortcut::Category::Edit:
            name = "Edit";
            break;
        case KStandardShortcut::Category::Navigation:
            name = "Navigation";
            break;
        case KStandardShortcut::Category::View:
            name = "View";
            break;
        case KStandardShortcut::Category::Settings:
            name = "Settings";
            break;
        case KStandardShortcut::Category::Help:
            name = "Help";
            break;
        }
        auto cat = std::find_if(m_components.begin(), m_components.end(), [&name](const Component &c) {
            return c.id == name;
        });
        if (cat == m_components.end()) {
            qCWarning(KCMKEYS) << "No category for standard shortcut" << key;
            continue;
        }
        auto action = std::find_if(cat->actions.begin(), cat->actions.end(), [&](const Action &a) {
            return a.id == key;
        });
        if (action == cat->actions.end()) {
            qCWarning(KCMKEYS) << "Model doesn't include action" << key;
            continue;
        }
        const auto shortcuts = QKeySequence::listFromString(group.readEntry(key));
        const QSet<QKeySequence> shortcutsSet(shortcuts.cbegin(), shortcuts.cend());
        if (shortcutsSet != action->activeShortcuts) {
            action->activeShortcuts = shortcutsSet;
            const QModelIndex i = index(action - cat->actions.begin(), 0, index(cat - m_components.begin(), 0));
            Q_EMIT dataChanged(i, i, {CustomShortcutsRole, ActiveShortcutsRole});
        }
    }
}

void StandardShortcutsModel::save()
{
    for (auto &component : m_components) {
        for (auto &action : component.actions) {
            if (action.initialShortcuts != action.activeShortcuts) {
                QList<QKeySequence> keys(action.activeShortcuts.cbegin(), action.activeShortcuts.cend());
                KStandardShortcut::saveShortcut(KStandardShortcut::findByName(action.id), keys);
                action.initialShortcuts = action.activeShortcuts;
            }
        }
    }
}
