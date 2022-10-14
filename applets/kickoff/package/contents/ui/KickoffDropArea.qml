/*
 * SPDX-FileCopyrightText: 2021 Noah Davis <noahadvs@gmail.com>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick 2.15
import QtQml 2.15
import org.kde.plasma.core 2.0 as PlasmaCore

DropArea {
    id: root
    required property Flickable targetView
    readonly property bool enableAutoScroll: targetView.height < targetView.contentHeight
    property real scrollUpMargin: 0
    property real scrollDownMargin: 0
    enabled: plasmoid.immutability !== PlasmaCore.Types.SystemImmutable
    onPositionChanged: if (drag.source instanceof KickoffGridDelegate || drag.source instanceof KickoffListDelegate) {
        const source = drag.source
        const view = drag.source.view
        if (source.view === root.targetView && !view.move.running && !view.moveDisplaced.running) {
            const pos = mapToItem(view.contentItem, drag.x, drag.y)
            const targetIndex = view.indexAt(pos.x, pos.y)
            if (targetIndex >= 0 && targetIndex !== source.index) {
                view.model.moveRow(source.index, targetIndex)
                // itemIndex changes directly after moving,
                // we can just set the currentIndex to it then.
                view.currentIndex = source.index
            }
        }
    }

    function moveRow(targetIndex) {
        if (targetIndex < 0 || targetIndex >= targetView.count) {
            return;
        }

        targetView.model.moveRow(targetView.currentIndex, targetIndex);
        targetView.currentIndex = targetIndex;
    }

    Shortcut {
        enabled: (targetView instanceof GridView && targetView.currentIndex >= targetView.columns)
              || (targetView instanceof ListView && targetView.currentIndex > 0)
        sequence: "Ctrl+Shift+Up"
        onActivated: moveRow(targetView.currentIndex - (targetView instanceof GridView ? targetView.columns : 1))
    }

    Shortcut {
        enabled: (targetView instanceof GridView && targetView.currentIndex < targetView.count - targetView.columns)
              || (targetView instanceof ListView && targetView.currentIndex + 1 < targetView.count)
        sequence: "Ctrl+Shift+Down"
        onActivated: moveRow(targetView.currentIndex + (targetView instanceof GridView ? targetView.columns : 1))
    }

    Shortcut {
        enabled: targetView instanceof GridView && targetView.currentIndex % targetView.columns > 0
        sequence: "Ctrl+Shift+Left"
        onActivated: moveRow(targetView.currentIndex - 1)
    }

    Shortcut {
        enabled: targetView instanceof GridView && targetView.currentIndex % targetView.columns !== targetView.columns - 1
        sequence: "Ctrl+Shift+Right"
        onActivated: moveRow(targetView.currentIndex + 1)
    }

    SmoothedAnimation {
        target: root.targetView
        property: "contentY"
        to: 0
        velocity: 200 * PlasmaCore.Units.devicePixelRatio
        running: root.enableAutoScroll && root.containsDrag && root.drag.y <= root.scrollUpMargin
    }
    SmoothedAnimation {
        target: root.targetView
        property: "contentY"
        to: root.targetView.contentHeight - root.targetView.height
        velocity: 200 * PlasmaCore.Units.devicePixelRatio
        running: root.enableAutoScroll && root.containsDrag && root.drag.y >= root.height - root.scrollDownMargin
    }
}

