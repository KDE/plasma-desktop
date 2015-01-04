/*
    Copyright (C) 2011  Martin Gräßlin <mgraesslin@kde.org>
    Copyright (C) 2012 Marco Martin <mart@kde.org>
    Copyright 2014 Sebastian Kügler <sebas@kde.org>

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
import org.kde.kquickcontrolsaddons 2.0 as KQuickControlsAddons

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.draganddrop 2.0

import org.kde.plasma.private.kickoff 0.1 as Kickoff


Item {
    anchors.fill: parent

    objectName: "FavoritesView"

    property ListView listView: kickoffListView

    function decrementCurrentIndex() {
        kickoffListView.decrementCurrentIndex();
    }

    function incrementCurrentIndex() {
        kickoffListView.incrementCurrentIndex();
    }

    function activateCurrentIndex() {
        kickoffListView.currentItem.activate();
    }

    function openContextMenu() {
        listView.currentItem.openContextMenu();
    }

    ContextMenu {
        id: contextMenu

        PlasmaComponents.MenuItem {
            id: actionsSeparator

            separator: true
        }
        PlasmaComponents.MenuItem {
            id: sortFavoritesAscending

            text: i18n("Sort Alphabetically (A to Z)")
            icon: "view-sort-ascending"

            onClicked: {
                favoritesModel.sortFavoritesAscending();
            }
        }
        PlasmaComponents.MenuItem {
            id: sortFavoritesDescending

            text: i18n("Sort Alphabetically (Z to A)")
            icon: "view-sort-descending"

            onClicked: {
                favoritesModel.sortFavoritesDescending();
            }
        }
    }

    DropArea {
        property string dragUrl: ""
        property int startRow: -1

        anchors.fill: scrollArea

        function syncTarget(event) {
            var pos = mapToItem(kickoffListView.contentItem, event.x, event.y);

            var hoveredIndex = kickoffListView.indexAt(pos.y, pos.y);

            if (hoveredIndex != -1) {
                kickoffListView.currentIndex = hoveredIndex;
            } else {
                kickoffListView.currentIndex = kickoffListView.count - 1;
            }

            if (Math.abs(startRow - kickoffListView.currentIndex) <= 1) {
                dropTarget.visible = false;
                return;
            }

            var targetY = kickoffListView.currentItem.y;

            pos = kickoffListView.contentItem.mapToItem(kickoffListView.currentItem, pos.x, pos.y);

            if (pos.y > kickoffListView.currentItem.height / 2) {
                targetY += kickoffListView.currentItem.height;
            }

            dropTarget.y = kickoffListView.mapFromItem(kickoffListView.contentItem, 0, targetY).y;
            dropTarget.visible = true;
        }

        onDrop: {
            if (kickoffListView.currentItem == null || !dropTarget.visible) {
                return;
            }

            var pos = mapToItem(kickoffListView.contentItem, event.x, event.y);
            pos = kickoffListView.contentItem.mapToItem(kickoffListView.currentItem, pos.x, pos.y);

            var targetRow = kickoffListView.currentIndex;

            if (kickoffListView.currentIndex < startRow) {
                ++targetRow;
            }

            if (pos.y <= kickoffListView.currentItem.height / 2) {
                    --targetRow;
            }

            targetRow = Math.min(kickoffListView.count, targetRow);

            kickoffListView.model.move(startRow,  targetRow);

            dropTarget.visible = false;
        }

        onDragEnter: {
            dragUrl = kickoffListView.currentItem.url;
            startRow = kickoffListView.currentIndex;
            syncTarget(event);
        }

        onDragMove: syncTarget(event);

        onDragLeave: {
            dropTarget.visible = false;
        }

        Rectangle {
            id: dropTarget

            width: kickoffListView.width
            height: Math.max(2, units.smallSpacing)

            visible: false
            color: theme.highlightColor
        }
    }

    PlasmaExtras.ScrollArea {
        id: scrollArea

        anchors.fill: parent

        ListView {
            id: kickoffListView

            //anchors.fill: parent
            currentIndex: -1
            boundsBehavior: Flickable.StopAtBounds
            keyNavigationWraps: true
            interactive: contentHeight > height

            delegate: KickoffItem {}
            highlight: KickoffHighlight {}
            highlightMoveDuration : 0
            highlightResizeDuration: 0

            model: favoritesModel

            section {
                property: "group"
                criteria: ViewSection.FullString
                //delegate: SectionDelegate {}
            }
            Connections {
                target: plasmoid
                onExpandedChanged: {
                    if (!expanded) {
                        kickoffListView.currentIndex = -1;
                    }
                }
            }
        }
    }

}
