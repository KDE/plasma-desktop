/***************************************************************************
 *   Copyright (C) 2013 by Aurélien Gâteau <agateau@kde.org>               *
 *   Copyright (C) 2013-2015 by Eike Hein <hein@kde.org>                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

function fillActionMenu(actionMenu, actionList, favoriteModel, favoriteId) {
    // Accessing actionList can be a costly operation, so we don't
    // access it until we need the menu.

    var action = createFavoriteAction(favoriteModel, favoriteId);

    if (action) {
        if (actionList && actionList.length > 0) {
            var separator = { "type": "separator" };
            actionList.push(separator, action);
        } else {
            actionList = [action];
        }
    }

    actionMenu.actionList = actionList;
}

function createFavoriteAction(favoriteModel, favoriteId) {
    // Don't allow changes to favorites when system is immutable.
    if (plasmoid.immutability === PlasmaCore.Types.SystemImmutable) {
        return null;
    }

    if (favoriteModel === null || !favoriteModel.enabled || favoriteId == null) {
        return null;
    }

    var action = {};

    if (favoriteModel.isFavorite(favoriteId)) {
        action.text = i18n("Remove from Favorites");
        action.icon = "list-remove";
        action.actionId = "_kicker_favorite_remove";
    } else if (favoriteModel.maxFavorites == -1 || favoriteModel.count < favoriteModel.maxFavorites) {
        action.text = i18n("Add to Favorites");
        action.icon = "bookmark-new";
        action.actionId = "_kicker_favorite_add";
    } else {
        return null;
    }

    action.actionArgument = { favoriteModel: favoriteModel, favoriteId: favoriteId };

    return action;
}

function triggerAction(model, index, actionId, actionArgument) {
    function startsWith(txt, needle) {
        return txt.substr(0, needle.length) === needle;
    }

    if (startsWith(actionId, "_kicker_favorite_")) {
        handleFavoriteAction(actionId, actionArgument);
        return;
    }

    var closeRequested = model.trigger(index, actionId, actionArgument);

    if (closeRequested) {
        plasmoid.expanded = false;
    }
}

function handleFavoriteAction(actionId, actionArgument) {
    var favoriteId = actionArgument.favoriteId;
    var favoriteModel = actionArgument.favoriteModel;

    if (favoriteModel === null || favoriteId == null) {
        return null;
    }
    if (actionId == "_kicker_favorite_remove") {
        favoriteModel.removeFavorite(favoriteId);
    } else if (actionId == "_kicker_favorite_add") {
        favoriteModel.addFavorite(favoriteId);
    }
}
