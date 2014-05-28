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


BaseView {
    objectName: "RecentlyUsedView"

    model: Kickoff.RecentlyUsedModel {
        id: recentlyUsedModel
    }

    ContextMenu {
        id: contextMenu

        PlasmaComponents.MenuItem {
            id: clearRecentApplications

            text: i18n("Clear Recent Applications")
            icon: "edit-clear-history"
            onClicked: recentlyUsedModel.clearRecentApplications();
            visible: contextMenu.isApp
        }
        PlasmaComponents.MenuItem {
            id: clearRecentDocuments

            text: i18n("Clear Recent Documents")
            icon: "edit-clear-history"
            onClicked: recentlyUsedModel.clearRecentDocuments();
            visible: !contextMenu.isApp
        }
    }
}
