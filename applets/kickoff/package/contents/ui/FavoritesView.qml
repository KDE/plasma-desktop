/*
    SPDX-FileCopyrightText: 2011 Martin Gräßlin <mgraesslin@kde.org>
    SPDX-FileCopyrightText: 2012 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2014 Sebastian Kügler <sebas@kde.org>
    SPDX-FileCopyrightText: 2015-2018 Eike Hein <hein@kde.org>
    SPDX-FileCopyrightText: 2016 Jonathan Liu <net147@gmail.com>
    SPDX-FileCopyrightText: 2016 Kai Uwe Broulik <kde@privat.broulik.de>
    SPDX-FileCopyrightText: 2021 Mikel Johnson <mikel5764@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
import QtQuick 2.0

import org.kde.plasma.core 2.0 as PlasmaCore

FocusScope {

    objectName: "FavoritesView"

    property ListView listView: favoritesView.listView

    function keyNavUp() {
        return favoritesView.keyNavUp();
    }

    function keyNavDown() {
        return favoritesView.keyNavDown();
    }

    function activateCurrentIndex() {
        favoritesView.currentItem.activate();
    }

    function openContextMenu() {
        favoritesView.currentItem.openActionMenu();
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
        anchors.fill: parent
        enabled: plasmoid.immutability !== PlasmaCore.Types.SystemImmutable

        function syncTarget(drag) {
            if (favoritesView.animating) {
                return;
            }

            var pos = mapToItem(listView.contentItem, drag.x, drag.y);
            var above = listView.itemAt(pos.x, pos.y);

            var source = kickoff.dragSource;

            if (above && above !== source && isChildOf(source, favoritesView)) {
                favoritesView.model.moveRow(source.itemIndex, above.itemIndex);
                // itemIndex changes directly after moving,
                // we can just set the currentIndex to it then.
                favoritesView.currentIndex = source.itemIndex;
            }
        }

        onPositionChanged: syncTarget(drag)
        onEntered: syncTarget(drag)
    }

    Transition {
        id: moveTransition
        SequentialAnimation {
            PropertyAction { target: favoritesView; property: "animating"; value: true }

            NumberAnimation {
                duration: favoritesView.animationDuration
                properties: "x, y"
                easing.type: Easing.OutQuad
            }

            PropertyAction { target: favoritesView; property: "animating"; value: false }
        }
    }

    Connections {
        target: plasmoid
        function onExpandedChanged() {
            if (!plasmoid.expanded) {
                favoritesView.currentIndex = 0;
            }
        }
    }

    KickoffListView {
        id: favoritesView

        anchors.fill: parent

        property bool animating: false
        property int animationDuration: resetAnimationDurationTimer.interval
        focus: true

        interactive: contentHeight > height

        move: moveTransition
        moveDisplaced: moveTransition

        model: globalFavorites

        onCountChanged: {
            animationDuration = 0;
            resetAnimationDurationTimer.start();
        }
    }

    Timer {
        id: resetAnimationDurationTimer

        // We don't want drag animation to be affected by "Animation speed" setting cause this is an active interaction (we want this enabled even for those who disabled animations)
        // In other words: it's not a passive animation it should (roughly) follow the drag
        interval: 150

        onTriggered: favoritesView.animationDuration = interval - 20
    }

}
