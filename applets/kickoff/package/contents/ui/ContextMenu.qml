/*
    Copyright (C) 2011  Martin Gräßlin <mgraesslin@kde.org>
    Copyright (C) 2012 Marco Martin <mart@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/
import QtQuick 2.0
import org.kde.plasma.private.kickoff 0.1 as Kickoff
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.kquickcontrolsaddons 2.0

PlasmaComponents.ContextMenu {
    id: contextMenu

    property string title
    property variant model
    property bool isGlobalFavorite: model != undefined ?
        favoritesModel.isResourceLinkedToActivity(contextMenu.model.url, ":global") : false
    property bool isActivityFavorite: model != undefined ?
        favoritesModel.isResourceLinkedToActivity(contextMenu.model.url, ":current") : false
    property bool isApp: contextMenu.model != undefined ? contextMenu.model.url.indexOf(".desktop") !== -1 : false

    function openAt(title, model, x, y) {
        contextMenu.title = title
        contextMenu.model = model

        open(x, y)
    }

   /*
    * context menu items
    */
    PlasmaComponents.MenuItem {
        id: titleMenuItem

        text: contextMenu.title
        icon: contextMenu.model != undefined ? contextMenu.model.decoration : ""
        checkable: false
        enabled: false
    }
    PlasmaComponents.MenuItem {
        id: titleSeparator

        separator: true
    }
    // For applications that are not favorite at all
    //     Add to favorites
    //     Add to favorites of the current activity
    PlasmaComponents.MenuItem {
        id: addToGlobalFavorites
        text: i18n("Add To Favorites")
        onClicked: favoritesModel.linkResourceToActivity(contextMenu.model.url, ":global", function(){});

        visible: contextMenu.isApp && !contextMenu.isGlobalFavorite && !contextMenu.isActivityFavorite
    }
    PlasmaComponents.MenuItem {
        id: addToActivityFavorites
        text: i18n("Add To Favorites In This Activity Only")
        onClicked: {
            favoritesModel.linkResourceToActivity(contextMenu.model.url, ":current", function(){});
        }

        visible: contextMenu.isApp && !contextMenu.isGlobalFavorite && !contextMenu.isActivityFavorite
    }
    // For applications that are favorite
    //     Remove from favorites
    //     Favorite only in the current activity
    // For applications that are favorite in the current activity
    //     Remove from favorites
    //     Favorite in all activities
    PlasmaComponents.MenuItem {
        id: removeFromFavorites
        text: i18n("Remove From Favorites")
        onClicked: {
            var url = contextMenu.model.url;
            favoritesModel.unlinkResourceFromActivity(url, ":current", function(){});
            favoritesModel.unlinkResourceFromActivity(url, ":global", function(){});
        }

        visible: contextMenu.isApp && contextMenu.isGlobalFavorite || contextMenu.isActivityFavorite
    }
    PlasmaComponents.MenuItem {
        id: switchToGlobalFavorites
        text: i18n("Set Favorite In All Activities")
        onClicked: {
            var url = contextMenu.model.url;
            favoritesModel.unlinkResourceFromActivity(url, ":current", function(){
                favoritesModel.linkResourceToActivity(url, ":global", function(){});
            });
        }

        visible: contextMenu.isApp && !contextMenu.isGlobalFavorite && contextMenu.isActivityFavorite
    }
    PlasmaComponents.MenuItem {
        id: switchToActivityFavorites
        text: i18n("Set Favorite In Current Activity")
        onClicked: {
            var url = contextMenu.model.url;
            favoritesModel.unlinkResourceFromActivity(url, ":global", function(){
                favoritesModel.linkResourceToActivity(url, ":current", function(){});
            });
        }

        visible: contextMenu.isApp && !contextMenu.isActivityFavorite && contextMenu.isGlobalFavorite
    }
    PlasmaComponents.MenuItem {
        id: uninstallApp

        text: i18n("Uninstall")
        visible: packagekitSource.data["Status"] !== undefined && contextMenu.isApp

        onClicked: {
            var service = packagekitSource.serviceForSource("Status")
            var operation = service.operationDescription("uninstallApplication")
            operation.Url = contextMenu.model.url;
            var job = service.startOperationCall(operation)
        }
    }
} // contextMenu

