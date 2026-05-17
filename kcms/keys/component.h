/*
    SPDX-FileCopyrightText: 2020 David Redondo <kde@david-redondo.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#pragma once

#include "basemodel.h" // for InverseActionReassignmentSuggestion

#include <KGlobalShortcutTrigger>

#include <QHash>
#include <QKeySequence>
#include <QList>
#include <QSet>
#include <QString>
#include <optional>

struct Action {
    struct TriggerSets {
        QSet<KGlobalShortcutTrigger> activeTriggers;
        QSet<KGlobalShortcutTrigger> defaultTriggers;
        QSet<KGlobalShortcutTrigger> initialTriggers;
    };
    QString id;
    QString displayName;
    QSet<QKeySequence> activeShortcuts;
    QSet<QKeySequence> defaultShortcuts;
    QSet<QKeySequence> initialShortcuts;
    QHash<QString, TriggerSets> triggersByType;
    QString inverseAction;
    std::optional<InverseActionReassignmentSuggestion::Data> inverseActionReassignmentSuggestion;
};

struct Component {
    QString id;
    QString displayName;
    ComponentType type;
    QString icon;
    QList<Action> actions;
    bool checked;
    bool pendingDeletion;
};
