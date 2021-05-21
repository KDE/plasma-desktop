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

    objectName: "FavoritesGridView"

    property GridView gridView: favoritesGridView.gridView

    function keyNavLeft() { return favoritesGridView.keyNavLeft() }
    function keyNavRight() { return favoritesGridView.keyNavRight() }
    function keyNavUp() { return favoritesGridView.keyNavUp() }
    function keyNavDown() { return favoritesGridView.keyNavDown() }

    function activateCurrentIndex() {
        favoritesGridView.currentItem.activate();
    }

    function openContextMenu() {
        favoritesGridView.currentItem.openActionMenu();
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
            if (favoritesGridView.animating) {
                return;
            }

            var pos = mapToItem(gridView.contentItem, drag.x, drag.y);
            var above = gridView.itemAt(pos.x, pos.y);

            var source = kickoff.dragSource;

            if (above && above !== source && isChildOf(source, favoritesGridView)) {
                favoritesGridView.model.moveRow(source.itemIndex, above.itemIndex);
                // itemIndex changes directly after moving,
                // we can just set the currentIndex to it then.
                favoritesGridView.currentIndex = source.itemIndex;
            }
        }

        onPositionChanged: syncTarget(drag)
        onEntered: syncTarget(drag)
    }

    Transition {
        id: moveTransition
        SequentialAnimation {
            PropertyAction { target: favoritesGridView; property: "animating"; value: true }

            NumberAnimation {
                duration: favoritesGridView.animationDuration
                properties: "x, y"
                easing.type: Easing.OutQuad
            }

            PropertyAction { target: favoritesGridView; property: "animating"; value: false }
        }
    }

    Connections {
        target: plasmoid
        function onExpandedChanged() {
            if (!plasmoid.expanded) {
                favoritesGridView.currentIndex = 0;
            }
        }
    }

    KickoffGridView {
        id: favoritesGridView

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

        onTriggered: favoritesGridView.animationDuration = interval - 20
    }

}
