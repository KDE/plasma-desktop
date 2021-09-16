/*
    SPDX-FileCopyrightText: 2015 Eike Hein <hein@kde.org>
    SPDX-FileCopyrightText: 2021 Mikel Johnson <mikel5764@gmail.com>
    SPDX-FileCopyrightText: 2021 Noah Davis <noahadvs@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15
import QtQml 2.15
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 3.0 as PC3
import org.kde.kirigami 2.16 as Kirigami

// ScrollView makes it difficult to control implicit size using the contentItem.
// Using EmptyPage instead.
EmptyPage {
    id: root
    property alias model: view.model
    property alias count: view.count
    property alias currentIndex: view.currentIndex
    property alias currentItem: view.currentItem
    property alias delegate: view.delegate
    property alias view: view

    clip: view.interactive

    header: MouseArea {
        implicitHeight: KickoffSingleton.listItemMetrics.margins.top
        hoverEnabled: true
        onEntered: {
            if (containsMouse) {
                let targetIndex = view.indexAt(mouseX + view.contentX, view.contentY)
                if (targetIndex >= 0) {
                    view.currentIndex = targetIndex
                    view.forceActiveFocus(Qt.MouseFocusReason)
                }
            }
        }
    }

    footer: MouseArea {
        implicitHeight: KickoffSingleton.listItemMetrics.margins.bottom
        hoverEnabled: true
        onEntered: {
            if (containsMouse) {
                let targetIndex = view.indexAt(mouseX + view.contentX, view.height + view.contentY - 1)
                if (targetIndex >= 0) {
                    view.currentIndex = targetIndex
                    view.forceActiveFocus(Qt.MouseFocusReason)
                }
            }
        }
    }

    contentItem: GridView {
        id: view
        readonly property real availableWidth: width - leftMargin - rightMargin
        readonly property real availableHeight: height - topMargin - bottomMargin
        readonly property int columns: Math.floor(availableWidth / cellWidth)
        readonly property int rows: Math.floor(availableHeight / cellHeight)
        property bool movedWithKeyboard: false

        Accessible.description: i18n("Grid with %1 rows, %2 columns", rows, columns) // can't use i18np here

        implicitWidth: {
            if (plasmoid.configuration.applicationsDisplay === 0
                || (plasmoid.configuration.favoritesDisplay === 0
                    && KickoffSingleton.rootModel.favoritesModel.count > 16)) {
                return view.cellWidth * 4
                    + KickoffSingleton.leftPadding
                    + KickoffSingleton.rightPadding
                    + verticalScrollBar.implicitWidth
            }
            return view.cellWidth * 4 + leftMargin + rightMargin
        }
        implicitHeight: view.cellHeight * 4 + topMargin + bottomMargin

        leftMargin: KickoffSingleton.leftPadding
            + (verticalScrollBar.visible && root.mirrored ? verticalScrollBar.implicitWidth : 0)
        rightMargin: KickoffSingleton.rightPadding
            + (verticalScrollBar.visible && !root.mirrored ? verticalScrollBar.implicitWidth : 0)

        cellHeight: KickoffSingleton.gridCellSize
        cellWidth: KickoffSingleton.gridCellSize

        currentIndex: count > 0 ? 0 : -1
        focus: true
        interactive: height < contentHeight
        pixelAligned: true
        reuseItems: true
        boundsBehavior: Flickable.StopAtBounds
        // default keyboard navigation doesn't allow focus reasons to be used
        // and eats up/down key events when at the beginning or end of the list.
        keyNavigationEnabled: false
        keyNavigationWraps: false

        highlightMoveDuration: 0
        highlight: PlasmaCore.FrameSvgItem {
            opacity: view.activeFocus
                || (KickoffSingleton.contentArea === root
                    && KickoffSingleton.searchField.activeFocus) ? 1 : 0.5
            width: view.cellWidth
            height: view.cellHeight
            imagePath: "widgets/viewitem"
            prefix: "hover"
            visible: KickoffSingleton.contentArea !== root
                || ActionMenu.menu.status !== 1
        }

        delegate: KickoffItemDelegate {
            id: itemDelegate
            icon.width: PlasmaCore.Units.iconSizes.large
            icon.height: PlasmaCore.Units.iconSizes.large
            display: PC3.AbstractButton.TextUnderIcon
            width: view.cellWidth
            Accessible.role: Accessible.Cell
        }

        move: normalTransition
        moveDisplaced: normalTransition

        Transition {
            id: normalTransition
            NumberAnimation {
                duration: Plasma.Units.shortDuration
                properties: "x, y"
                easing.type: Easing.OutCubic
            }
        }

        PC3.ScrollBar.vertical: PC3.ScrollBar {
            id: verticalScrollBar
            parent: root
            visible: size < 1 && policy !== PC3.ScrollBar.AlwaysOff
            z: 2
            height: root.height
            anchors.right: parent.right
        }

        Kirigami.WheelHandler {
            target: view
        }

        Connections {
            target: plasmoid
            function onExpandedChanged() {
                if(!plasmoid.expanded) {
                    view.currentIndex = 0
                    view.positionViewAtBeginning()
                }
            }
        }

        // Used to block hover events temporarily after using keyboard navigation.
        // If you have one hand on the touch pad or mouse and another hand on the keyboard,
        // it's easy to accidentally reset the highlight/focus position to the mouse position.
        Timer {
            id: movedWithKeyboardTimer
            interval: 200
            onTriggered: view.movedWithKeyboard = false
        }

        function focusCurrentItem(event, focusReason) {
            currentItem.forceActiveFocus(focusReason)
            event.accepted = true
        }

        Keys.onMenuPressed: if (currentItem !== null) {
            currentItem.forceActiveFocus(Qt.ShortcutFocusReason)
            currentItem.openActionMenu()
        }
        Keys.onPressed: {
            let targetX = currentItem ? currentItem.x : contentX
            let targetY = currentItem ? currentItem.y : contentY
            let targetIndex = currentIndex
            // supports mirroring
            let atLeft = currentIndex % columns === 0
            // at the top of a given column and in the top row
            let atTop = currentIndex < columns
            // supports mirroring
            let atRight = currentIndex % columns === columns - 1
            // at bottom of a given column, not necessarily in the last row
            let atBottom = currentIndex >= count - columns
            // Implements the keyboard navigation described in https://www.w3.org/TR/wai-aria-practices-1.2/#grid
            if (count > 1) {
                switch (event.key) {
                    case Qt.Key_Left: if (!atLeft && !KickoffSingleton.searchField.activeFocus) {
                        moveCurrentIndexLeft()
                        focusCurrentItem(event, Qt.BacktabFocusReason)
                    } break
                    case Qt.Key_Up: if (!atTop) {
                        moveCurrentIndexUp()
                        focusCurrentItem(event, Qt.BacktabFocusReason)
                    } break
                    case Qt.Key_Right: if (!atRight && !KickoffSingleton.searchField.activeFocus) {
                        moveCurrentIndexRight()
                        focusCurrentItem(event, Qt.TabFocusReason)
                    } break
                    case Qt.Key_Down: if (!atBottom) {
                        moveCurrentIndexDown()
                        focusCurrentItem(event, Qt.TabFocusReason)
                    } break
                    case Qt.Key_Home: if (event.modifiers === Qt.ControlModifier && currentIndex !== 0) {
                        currentIndex = 0
                        focusCurrentItem(event, Qt.BacktabFocusReason)
                    } else if (!atLeft) {
                        targetIndex -= currentIndex % columns
                        currentIndex = Math.max(targetIndex, 0)
                        focusCurrentItem(event, Qt.BacktabFocusReason)
                    } break
                    case Qt.Key_End: if (event.modifiers === Qt.ControlModifier && currentIndex !== count - 1) {
                        currentIndex = count - 1
                        focusCurrentItem(event, Qt.TabFocusReason)
                    } else if (!atRight) {
                        targetIndex += columns - 1 - (currentIndex % columns)
                        currentIndex = Math.min(targetIndex, count - 1)
                        focusCurrentItem(event, Qt.TabFocusReason)
                    } break
                    case Qt.Key_PageUp: if (!atTop) {
                        targetY = targetY - height + 1
                        targetIndex = indexAt(targetX, targetY)
                        // TODO: Find a more efficient, but accurate way to do this
                        while (targetIndex === -1) {
                            targetY += 1
                            targetIndex = indexAt(targetX, targetY)
                        }
                        currentIndex = Math.max(targetIndex, 0)
                        focusCurrentItem(event, Qt.BacktabFocusReason)
                    } break
                    case Qt.Key_PageDown: if (!atBottom) {
                        targetY = targetY + height - 1
                        targetIndex = indexAt(targetX, targetY)
                        // TODO: Find a more efficient, but accurate way to do this
                        while (targetIndex === -1) {
                            targetY -= 1
                            targetIndex = indexAt(targetX, targetY)
                        }
                        currentIndex = Math.min(targetIndex, count - 1)
                        focusCurrentItem(event, Qt.TabFocusReason)
                    } break
                }
            }
            movedWithKeyboard = event.accepted
            if (movedWithKeyboard) {
                movedWithKeyboardTimer.restart()
            }
        }
    }
}
