/*
    SPDX-FileCopyrightText: 2015 Eike Hein <hein@kde.org>
    SPDX-FileCopyrightText: 2021 Mikel Johnson <mikel5764@gmail.com>
    SPDX-FileCopyrightText: 2021 Noah Davis <noahadvs@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Templates as T

import org.kde.plasma.components as PC3
import org.kde.plasma.extras as PlasmaExtras

import org.kde.ksvg as KSvg
import org.kde.kirigami as Kirigami

// ScrollView makes it difficult to control implicit size using the contentItem.
// Using EmptyPage instead.
EmptyPage {
    id: root
    property alias model: view.model
    property alias count: view.count
    property alias currentIndex: view.currentIndex
    property alias currentItem: view.currentItem
    property alias delegate: view.delegate
    property alias blockTargetWheel: wheelHandler.blockTargetWheel
    property alias view: view

    clip: view.height < view.contentHeight

    header: MouseArea {
        implicitHeight: KickoffSingleton.listItemMetrics.margins.top
        hoverEnabled: true
        onEntered: {
            if (containsMouse) {
                const targetIndex = view.indexAt(mouseX + view.contentX, view.contentY)
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
                const targetIndex = view.indexAt(mouseX + view.contentX, view.height + view.contentY - 1)
                if (targetIndex >= 0) {
                    view.currentIndex = targetIndex
                    view.forceActiveFocus(Qt.MouseFocusReason)
                }
            }
        }
    }

    /* Not setting GridView as the contentItem because GridView has no way to
     * set horizontal alignment. I don't want to use leftPadding/rightPadding
     * for that because I'd have to change the implicitWidth formula and use a
     * more complicated calculation to get the correct padding.
     */
    GridView {
        id: view

        // Not storing them as properties somehow avoids a dubious warning
        // about *Binding loop detected for property "rows"*
        function availableWidth(): real {
            return width - leftMargin - rightMargin;
        }
        function availableHeight(): real {
            return height - topMargin - bottomMargin;
        }

        readonly property int columns: Math.floor(availableWidth() / cellWidth)
        readonly property int rows: Math.floor(availableHeight() / cellHeight)
        property bool movedWithKeyboard: false
        property bool movedWithWheel: false

        // NOTE: parent is the contentItem that Control subclasses automatically
        // create when no contentItem is set, but content is added.
        height: parent.height
        // There are lots of ways to try to center the content of a GridView
        // and many of them have bad visual flaws. This way works pretty well.
        // Not center aligning when there might be a scrollbar to keep click target positions consistent.
        anchors.horizontalCenter: kickoff.mayHaveGridWithScrollBar ? undefined : parent.horizontalCenter
        anchors.horizontalCenterOffset: if (kickoff.mayHaveGridWithScrollBar) {
            if (root.mirrored) {
                return verticalScrollBar.implicitWidth/2
            } else {
                return -verticalScrollBar.implicitWidth/2
            }
        } else {
            return 0
        }
        width: Math.min(parent.width, Math.floor((parent.width - leftMargin - rightMargin - (kickoff.mayHaveGridWithScrollBar ? verticalScrollBar.implicitWidth : 0)) / cellWidth) * cellWidth + leftMargin + rightMargin)

        Accessible.description: i18n("Grid with %1 rows, %2 columns", rows, columns) // can't use i18np here


        implicitWidth: {
            let w = view.cellWidth * kickoff.minimumGridRowCount + leftMargin + rightMargin
            if (kickoff.mayHaveGridWithScrollBar) {
                w += verticalScrollBar.implicitWidth
            }
            return w
        }
        implicitHeight: view.cellHeight * kickoff.minimumGridRowCount + topMargin + bottomMargin

        leftMargin: kickoff.backgroundMetrics.leftPadding
        rightMargin: kickoff.backgroundMetrics.rightPadding

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
        highlight: PlasmaExtras.Highlight {
            // The default Z value for delegates is 1. The default Z value for the section delegate is 2.
            // The highlight gets a value of 3 while the drag is active and then goes back to the default value of 0.
            z: (root.currentItem?.Drag.active ?? false) ? 3 : 0

            pressed: (view.currentItem as T.AbstractButton)?.down ?? false

            active: view.activeFocus
                || (kickoff.contentArea === root
                    && kickoff.searchField.activeFocus)

            width: view.cellWidth
            height: view.cellHeight
        }

        delegate: KickoffGridDelegate {
            id: itemDelegate
            width: view.cellWidth
            Accessible.role: Accessible.Cell
        }

        move: normalTransition
        moveDisplaced: normalTransition

        Transition {
            id: normalTransition
            NumberAnimation {
                duration: Kirigami.Units.shortDuration
                properties: "x, y"
                easing.type: Easing.OutCubic
            }
        }

        PC3.ScrollBar.vertical: PC3.ScrollBar {
            id: verticalScrollBar
            parent: root
            z: 2
            height: root.height
            anchors.right: parent.right
        }

        Kirigami.WheelHandler {
            id: wheelHandler
            target: view
            filterMouseEvents: true
            // `20 * Qt.styleHints.wheelScrollLines` is the default speed.
            horizontalStepSize: 20 * Qt.styleHints.wheelScrollLines
            verticalStepSize: 20 * Qt.styleHints.wheelScrollLines

            onWheel: wheel => {
                view.movedWithWheel = true
                view.movedWithKeyboard = false
                movedWithWheelTimer.restart()
            }
        }

        Connections {
            target: kickoff
            function onExpandedChanged() {
                if (!kickoff.expanded) {
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

        Timer {
            id: movedWithWheelTimer
            interval: 200
            onTriggered: view.movedWithWheel = false
        }

        function focusCurrentItem(event, focusReason) {
            currentItem.forceActiveFocus(focusReason)
            event.accepted = true
        }

        Keys.onMenuPressed: event => {
            const delegate = currentItem as AbstractKickoffItemDelegate;
            if (delegate !== null) {
                delegate.forceActiveFocus(Qt.ShortcutFocusReason)
                delegate.openActionMenu()
            }
        }

        Keys.onPressed: event => {
            const targetX = currentItem ? currentItem.x : contentX
            let targetY = currentItem ? currentItem.y : contentY
            let targetIndex = currentIndex
            // supports mirroring
            const atLeft = currentIndex % columns === (Qt.application.layoutDirection == Qt.RightToLeft ? columns - 1 : 0)
            // at the beginning of a line
            const isLeading = currentIndex % columns === 0
            // at the top of a given column and in the top row
            const atTop = currentIndex < columns
            // supports mirroring
            const atRight = currentIndex % columns === (Qt.application.layoutDirection == Qt.RightToLeft ? 0 : columns - 1)
            // at the end of a line
            const isTrailing = currentIndex % columns === columns - 1
            // at bottom of a given column, not necessarily in the last row
            let atBottom = currentIndex >= count - columns
            // Implements the keyboard navigation described in https://www.w3.org/TR/wai-aria-practices-1.2/#grid
            if (count > 1) {
                switch (event.key) {
                    case Qt.Key_Left: if (!atLeft && !kickoff.searchField.activeFocus) {
                        moveCurrentIndexLeft()
                        focusCurrentItem(event, Qt.BacktabFocusReason)
                    } break
                    case Qt.Key_H: if (!atLeft && !kickoff.searchField.activeFocus && event.modifiers & Qt.ControlModifier) {
                        moveCurrentIndexLeft()
                        focusCurrentItem(event, Qt.BacktabFocusReason)
                    } break
                    case Qt.Key_Up: if (!atTop) {
                        moveCurrentIndexUp()
                        focusCurrentItem(event, Qt.BacktabFocusReason)
                    } break
                    case Qt.Key_K: if (!atTop && event.modifiers & Qt.ControlModifier) {
                        moveCurrentIndexUp()
                        focusCurrentItem(event, Qt.BacktabFocusReason)
                    } break
                    case Qt.Key_Right: if (!atRight && !kickoff.searchField.activeFocus) {
                        moveCurrentIndexRight()
                        focusCurrentItem(event, Qt.TabFocusReason)
                    } break
                    case Qt.Key_L: if (!atRight && !kickoff.searchField.activeFocus && event.modifiers & Qt.ControlModifier) {
                        moveCurrentIndexRight()
                        focusCurrentItem(event, Qt.TabFocusReason)
                    } break
                    case Qt.Key_Down: if (!atBottom) {
                        moveCurrentIndexDown()
                        focusCurrentItem(event, Qt.TabFocusReason)
                    } break
                    case Qt.Key_J: if (!atBottom && event.modifiers & Qt.ControlModifier) {
                        moveCurrentIndexDown()
                        focusCurrentItem(event, Qt.TabFocusReason)
                    } break
                    case Qt.Key_Home: if (event.modifiers === Qt.ControlModifier && currentIndex !== 0) {
                        currentIndex = 0
                        focusCurrentItem(event, Qt.BacktabFocusReason)
                    } else if (!isLeading) {
                        targetIndex -= currentIndex % columns
                        currentIndex = Math.max(targetIndex, 0)
                        focusCurrentItem(event, Qt.BacktabFocusReason)
                    } break
                    case Qt.Key_End: if (event.modifiers === Qt.ControlModifier && currentIndex !== count - 1) {
                        currentIndex = count - 1
                        focusCurrentItem(event, Qt.TabFocusReason)
                    } else if (!isTrailing) {
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
                    case Qt.Key_Return:
                        /* Fall through*/
                    case Qt.Key_Enter:
                        root.currentItem.action.triggered();
                        root.currentItem.forceActiveFocus(Qt.ShortcutFocusReason);
                        event.accepted = true;
                        break;
                }
            }
            movedWithKeyboard = event.accepted
            if (movedWithKeyboard) {
                movedWithKeyboardTimer.restart()
            }
        }
    }
}
