/*
    SPDX-FileCopyrightText: 2011 Martin Gräßlin <mgraesslin@kde.org>
    SPDX-FileCopyrightText: 2012 Gregor Taetzner <gregor@freenet.de>
    SPDX-FileCopyrightText: 2015-2018 Eike Hein <hein@kde.org>
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
    property alias section: view.section
    property alias view: view

    property bool mainContentView: false

    clip: view.height < view.contentHeight

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

    contentItem: ListView {
        id: view

        readonly property real availableWidth: width - leftMargin - rightMargin
        readonly property real availableHeight: height - topMargin - bottomMargin
        property bool movedWithKeyboard: false

        Accessible.role: Accessible.List

        implicitWidth: {
            if (mainContentView) {
                if (plasmoid.configuration.applicationsDisplay === 0
                    || (plasmoid.configuration.favoritesDisplay === 0
                        && KickoffSingleton.rootModel.favoritesModel.count > 16)) {
                    return KickoffSingleton.gridCellSize * 4
                        + KickoffSingleton.leftPadding
                        + KickoffSingleton.rightPadding
                        + verticalScrollBar.implicitWidth
                }
                return KickoffSingleton.gridCellSize * 4
                    + KickoffSingleton.leftPadding
                    + KickoffSingleton.rightPadding
            }
            return contentWidth + leftMargin + rightMargin
        }
        // If either uses a grid, use grid cells to determine size
        implicitHeight: (plasmoid.configuration.favoritesDisplay == 0 || plasmoid.configuration.applicationsDisplay == 0
                ? KickoffSingleton.gridCellSize * 4
                : Math.floor(KickoffSingleton.gridCellSize * 4 / KickoffSingleton.listDelegateHeight)
                    * KickoffSingleton.listDelegateHeight)
            + topMargin + bottomMargin

        leftMargin: if (root.mirrored && verticalScrollBar.visible) {
            return verticalScrollBar.implicitWidth + KickoffSingleton.leftPadding
        } else {
            return KickoffSingleton.leftPadding
        }
        rightMargin: if (!root.mirrored && verticalScrollBar.visible) {
            return verticalScrollBar.implicitWidth + KickoffSingleton.rightPadding
        } else {
            return KickoffSingleton.rightPadding
        }

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

        // This is actually needed. The highlight will animate from thin to wide otherwise.
        highlightResizeDuration: 0
        highlightMoveDuration: 0
        highlight: PlasmaCore.FrameSvgItem {
            // The default Z value for delegates is 1. The default Z value for the section delegate is 2.
            // The highlight gets a value of 3 while the drag is active and then goes back to the default value of 0.
            z: root.currentItem && root.currentItem.Drag.active ?
                3 : 0
            opacity: view.activeFocus
                || (KickoffSingleton.contentArea === root
                    && KickoffSingleton.searchField.activeFocus) ? 1 : 0.5
            imagePath: "widgets/viewitem"
            prefix: "hover"
            visible: KickoffSingleton.contentArea !== root
                || ActionMenu.menu.status !== 1
        }

        delegate: KickoffItemDelegate {
            id: itemDelegate
            extendHoverMargins: true
            width: view.availableWidth
        }

        section {
            property: "group"
            criteria: ViewSection.FullString
            delegate: PC3.Label {
                //readonly property bool visualFocus: false
                width: section.length === 1
                    ? KickoffSingleton.listDelegateContentHeight + leftPadding + rightPadding
                    // Accessing implicitWidth fixes the width being 0 when loaded.
                    : Math.min(Math.ceil(implicitWidth), view.contentWidth)
                height: KickoffSingleton.listDelegateHeight
                leftPadding: view.effectiveLayoutDirection === Qt.LeftToRight
                    ? KickoffSingleton.listItemMetrics.margins.left : 0
                rightPadding: view.effectiveLayoutDirection === Qt.RightToLeft
                    ? KickoffSingleton.listItemMetrics.margins.right : 0
                horizontalAlignment: section.length === 1 ? Text.AlignHCenter : Text.AlignLeft
                verticalAlignment: Text.AlignVCenter
                maximumLineCount: 1
                elide: Text.ElideRight
                font.pixelSize: KickoffSingleton.listDelegateContentHeight
                enabled: false
                text: section.length === 1 ? section.toUpperCase() : section
            }
        }

        move: normalTransition
        moveDisplaced: normalTransition

        Transition {
            id: normalTransition
            NumberAnimation {
                duration: PlasmaCore.Units.shortDuration
                properties: "x, y"
                easing.type: Easing.OutCubic
            }
        }

        Item {
            parent: view
            anchors.fill: parent
            z: 1
            TapHandler { // Filter mouse events to avoid flicking like ScrollView
                // Do not accept stylus as it will cause bug 445111.
                acceptedDevices: PointerDevice.Mouse | PointerDevice.TouchPad | PointerDevice.TouchScreen
                onGrabChanged: {
                    const pressed = transition & (EventPoint.GrabPassive | EventPoint.GrabExclusive) && point.state & EventPoint.Pressed
                    const deviceType = point.event.device.type
                    if (pressed && deviceType & (PointerDevice.Mouse | PointerDevice.TouchPad)) {
                        view.interactive = false
                        verticalScrollBar.interactive = true
                    } else if (pressed && deviceType & PointerDevice.TouchScreen) {
                        // No need for binding. Touching will cause pressed() to be emitted again.
                        view.interactive = view.height < view.contentHeight
                        verticalScrollBar.interactive = false
                    }
                    point.accepted = false
                }
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
            let atFirst = currentIndex === 0
            let atLast = currentIndex === count - 1
            if (count > 1) {
                switch (event.key) {
                    case Qt.Key_Up: if (!atFirst) {
                        decrementCurrentIndex()
                        focusCurrentItem(event, Qt.BacktabFocusReason)
                    } break
                    case Qt.Key_Down: if (!atLast) {
                        incrementCurrentIndex()
                        focusCurrentItem(event, Qt.TabFocusReason)
                    } break
                    case Qt.Key_Home: if (!atFirst) {
                        currentIndex = 0
                        focusCurrentItem(event, Qt.BacktabFocusReason)
                    } break
                    case Qt.Key_End: if (!atLast) {
                        currentIndex = count - 1
                        focusCurrentItem(event, Qt.TabFocusReason)
                    } break
                    case Qt.Key_PageUp: if (!atFirst) {
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
                    case Qt.Key_PageDown: if (!atLast) {
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
