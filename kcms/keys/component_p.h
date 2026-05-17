/*
    SPDX-FileCopyrightText: 2020 David Redondo <kde@david-redondo.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#pragma once

#include <QKeySequence>
#include <QList>
#include <QSet>
#include <QString>

#include <memory>

struct InverseActionReassignmentSuggestionPrivate {
    QString inverseId;
    QString inverseDisplayName;
    QPersistentModelIndex inverseIndex;
    QKeySequence shortcutSuggestion;
    QKeySequence shortcutToReplace;
};

struct Action {
    QString id;
    QString displayName;
    QSet<QKeySequence> activeShortcuts;
    QSet<QKeySequence> defaultShortcuts;
    QSet<QKeySequence> initialShortcuts;
    QString inverseAction;
    std::shared_ptr<const InverseActionReassignmentSuggestionPrivate> inverseActionReassignmentSuggestion;
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
