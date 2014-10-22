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
    property bool isFavorite: model != undefined ? favoritesModel.isFavorite(contextMenu.model.url) : false
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
    PlasmaComponents.MenuItem {
        id: addToFavorites

        text: contextMenu.isFavorite ? i18n("Remove From Favorites") : i18n("Add To Favorites")
        icon: contextMenu.isFavorite ? "list-remove" : "bookmark-new"
        visible: contextMenu.isApp

        onClicked: {
            // FIXME: apparently contextMenu.model is empty
            if (contextMenu.isFavorite) {
                favoritesModel.remove(contextMenu.model.url);
            } else {
                favoritesModel.add(contextMenu.model.url);
                switchToFavorites();
            }
        }
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

