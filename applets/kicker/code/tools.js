/*
    SPDX-FileCopyrightText: 2013 Aurélien Gâteau <agateau@kde.org>
    SPDX-FileCopyrightText: 2013-2015 Eike Hein <hein@kde.org>
    SPDX-FileCopyrightText: 2017 Ivan Cukic <ivan.cukic@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

.pragma library

function fillActionMenu(i18n, actionMenu, actionList, favoriteModel, favoriteId) {
    // Accessing actionList can be a costly operation, so we don't
    // access it until we need the menu.

    const favoriteActions = createFavoriteActions(i18n, favoriteModel, favoriteId);

    if (favoriteActions) {
        if (actionList && actionList.length > 0) {
            const actionListCopy = Array.from(actionList);
            // same logic as in kickoff's openActionMenu() in
            // AbstractKickoffItemDelegate
            // Insert favoriteActions before the "Add to" actions (called
            // "addLauncherActions" by libkicker)
            const firstAddToActionIndex = actionListCopy.findIndex((action) =>
                ["addToDesktop", "addToTaskManager", "addToPanel"].includes(action?.actionId),
            );
            if (firstAddToActionIndex >= 0) {
                actionListCopy.splice(firstAddToActionIndex, 0, ...favoriteActions);
            } else {
                // Or at the end (with a separator) if they're not found
                actionListCopy.push({ type: "separator" }, ...favoriteActions);
            }
            // actionList = actions.concat(actionList); // this crashes Qt O.o
            actionList = actionListCopy;
        } else {
            actionList = favoriteActions;
        }
    }

    actionMenu.actionList = actionList;
}

function createFavoriteActions(i18n, favoriteModel, favoriteId) {
    if (!favoriteModel || !favoriteModel.enabled || !favoriteId) {
        return null;
    }

    if (favoriteModel.activities === undefined ||
        favoriteModel.activities.activities.length <= 1) {
        const action = {};

        if (favoriteModel.isFavorite(favoriteId)) {
            action.text = i18n("Remove from Favorites");
            action.icon = "bookmark-remove";
            action.actionId = "_kicker_favorite_remove";
        } else if (favoriteModel.maxFavorites === -1 || favoriteModel.count < favoriteModel.maxFavorites) {
            action.text = i18n("Add to Favorites");
            action.icon = "bookmark-new";
            action.actionId = "_kicker_favorite_add";
        } else {
            return null;
        }

        action.actionArgument = {
            favoriteModel,
            favoriteId,
        };

        return [action];

    } else {
        const actions = [];

        const linkedActivities = favoriteModel.linkedActivitiesFor(favoriteId);

        const activities = favoriteModel.activities.activities;

        // Adding the item to link/unlink to all activities

        const linkedToAllActivities = linkedActivities.includes(":global");

        actions.push({
            text: i18n("On All Activities"),
            checkable: true,

            actionId: linkedToAllActivities
                ? "_kicker_favorite_remove_from_activity"
                : "_kicker_favorite_set_to_activity",

            checked: linkedToAllActivities,

            actionArgument: {
                favoriteModel,
                favoriteId,
                favoriteActivity: "",
            },
        });


        // Adding items for each activity separately

        const addActivityItem = (activityId, activityName) => {
            const linkedToThisActivity = linkedActivities.includes(activityId);

            actions.push({
                text: activityName,
                checkable: true,
                checked: linkedToThisActivity && !linkedToAllActivities,

                actionId:
                    // If we are on all activities, and the user clicks just one
                    // specific activity, unlink from everything else
                    linkedToAllActivities ? "_kicker_favorite_set_to_activity" :

                    // If we are linked to the current activity, just unlink from
                    // that single one
                    linkedToThisActivity ? "_kicker_favorite_remove_from_activity" :

                    // Otherwise, link to this activity, but do not unlink from
                    // other ones
                    "_kicker_favorite_add_to_activity",

                actionArgument: {
                    favoriteModel,
                    favoriteId,
                    favoriteActivity: activityId,
                },
            });
        };

        // Adding the item to link/unlink to the current activity

        addActivityItem(favoriteModel.activities.currentActivity, i18n("On the Current Activity"));

        actions.push({
            type: "separator",
            actionId: "_kicker_favorite_separator",
        });

        // Adding the items for each activity

        activities.forEach(activityId => {
            addActivityItem(activityId, favoriteModel.activityNameForId(activityId));
        });

        return [{
            text: i18n("Show in Favorites"),
            icon: "favorite",
            subActions: actions,
        }];
    }
}

function triggerAction(model, index, actionId, actionArgument) {
    if (actionId.startsWith("_kicker_favorite_")) {
        handleFavoriteAction(actionId, actionArgument);
        return;
    }

    const closeRequested = model.trigger(index, actionId, actionArgument);
    return closeRequested;
}

function handleFavoriteAction(actionId, actionArgument) {
    const { favoriteId, favoriteModel, favoriteActivity } = actionArgument;

    if (favoriteModel === null || favoriteId === null) {
        return null;
    }

    if (actionId === "_kicker_favorite_remove") {
        favoriteModel.removeFavorite(favoriteId);
    } else if (actionId === "_kicker_favorite_add") {
        favoriteModel.addFavorite(favoriteId);
    } else if (actionId === "_kicker_favorite_remove_from_activity") {
        favoriteModel.removeFavoriteFrom(favoriteId, favoriteActivity);
    } else if (actionId === "_kicker_favorite_add_to_activity") {
        favoriteModel.addFavoriteTo(favoriteId, favoriteActivity);
    } else if (actionId === "_kicker_favorite_set_to_activity") {
        favoriteModel.setFavoriteOn(favoriteId, favoriteActivity);
    }
}
