/*
    Copyright (C) 2011  Martin Gräßlin <mgraesslin@kde.org>
    Copyright (C) 2012 Marco Martin <mart@kde.org>
    Copyright 2014 Sebastian Kügler <sebas@kde.org>
    Copyright (C) 2015  Eike Hein <hein@kde.org>
    Copyright (C) 2016 Jonathan Liu <net147@gmail.com>
    Copyright (C) 2016 Kai Uwe Broulik <kde@privat.broulik.de>

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

import org.kde.plasma.private.kicker 0.1 as Kicker

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
        kickoffListView.currentItem.openActionMenu();
    }

    // QQuickItem::isAncestorOf is not invokable...
    function isChildOf(item, parent) {
        if (!item || !parent) {
            return false;
        }

        if (item.parent === parent) {
            return true;
        }

        return isChildOf(item, item.parent);
    }

    DropArea {
        property int startRow: -1

        anchors.fill: scrollArea
        enabled: plasmoid.immutability !== PlasmaCore.Types.SystemImmutable

        function syncTarget(event) {
            if (kickoffListView.animating) {
                return;
            }

            var pos = mapToItem(kickoffListView.contentItem, event.x, event.y);
            var above = kickoffListView.itemAt(pos.x, pos.y);

            var source = kickoff.dragSource;

            if (above && above !== source && isChildOf(source, kickoffListView)) {
                kickoffListView.model.moveRow(source.itemIndex, above.itemIndex);
                // itemIndex changes directly after moving,
                // we can just set the currentIndex to it then.
                kickoffListView.currentIndex = source.itemIndex;
            }
        }

        onDragEnter: {
            syncTarget(event);
            startRow = kickoffListView.currentIndex;
        }

        onDragMove: syncTarget(event);
    }

    Transition {
        id: moveTransition
        SequentialAnimation {
            PropertyAction { target: kickoffListView; property: "animating"; value: true }

            NumberAnimation {
                duration: kickoffListView.animationDuration
                properties: "x, y"
                easing.type: Easing.OutQuad
            }

            PropertyAction { target: kickoffListView; property: "animating"; value: false }
        }
    }

    PlasmaExtras.ScrollArea {
        id: scrollArea

        anchors.fill: parent

        ListView {
            id: kickoffListView

            property bool animating: false
            property int animationDuration: resetAnimationDurationTimer.interval

            currentIndex: -1
            boundsBehavior: Flickable.StopAtBounds
            keyNavigationWraps: true
            interactive: contentHeight > height

            delegate: KickoffItem {}
            highlight: KickoffHighlight {}
            highlightMoveDuration : 0
            highlightResizeDuration: 0

            model: globalFavorites

            onCountChanged: {
                animationDuration = 0;
                resetAnimationDurationTimer.start();
            }

            section {
                property: "group"
                criteria: ViewSection.FullString
            }

            move: moveTransition
            moveDisplaced: moveTransition

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

    Timer {
        id: resetAnimationDurationTimer

        interval: 150

        onTriggered: kickoffListView.animationDuration = interval - 20
    }

}
