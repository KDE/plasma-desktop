/*
    SPDX-FileCopyrightText: 2011 Martin Gräßlin <mgraesslin@kde.org>
    SPDX-FileCopyrightText: 2012 Gregor Taetzner <gregor@freenet.de>
    SPDX-FileCopyrightText: 2015-2018 Eike Hein <hein@kde.org>
    SPDX-FileCopyrightText: 2021 Mikel Johnson <mikel5764@gmail.com>
    SPDX-FileCopyrightText: 2021 Noah Davis <noahadvs@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
import QtQuick 2.15
import QtQuick.Templates 2.15 as T
import QtQml 2.15
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 3.0 as PC3
import org.kde.plasma.extras 2.0 as PlasmaExtras
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
        
        property bool showHighlight: !root.mainContentView || KickoffSingleton.searchField.text !== "" // if we are searching, we want highlight on search items
        
        // turn on highlight when you start searching 
        Connections {
            target: KickoffSingleton.searchField
            function onTextChanged() {
                view.showHighlight = true;
            }
        }
        
        // This is actually needed. The highlight will animate from thin to wide otherwise.
        highlightResizeDuration: 0
        highlightMoveDuration: 0
        highlight: PlasmaCore.FrameSvgItem {
            opacity: view.activeFocus ? 1 : 0.5
            imagePath: "widgets/viewitem"
            prefix: "hover"
            visible: view.showHighlight
        }

        delegate: KickoffItemDelegate {
            id: itemDelegate
            extendHoverMargins: true
            width: view.availableWidth
            isSearchResult: root.objectName == "searchView"
            
            // if menu has closed, remove highlight if not hovering
            onMenuClosedChanged: {
                if (menuClosed && KickoffSingleton.searchField.text === "") {
                    // if we are searching, we want highlight on search items
                    view.showHighlight = mouseArea.containsMouse || KickoffSingleton.searchField.text !== "";
                }
            }
            
            // update whether highlight should be shown based on if item delegate is hovered
            Connections {
                target: mouseArea
                function onContainsMouseChanged() {
                    // don't remove highlight when opening right click menu
                    if (itemDelegate.menuClosed || mouseArea.containsMouse) {
                        view.showHighlight = mouseArea.containsMouse;
                    }
                }
            }
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

        Timer {
            id: movedWithKeyboardTimer
            interval: 200
            onTriggered: view.movedWithKeyboard = false
        }

        function focusCurrentItem(event, focusReason) {
            currentItem.forceActiveFocus(focusReason)
            event.accepted = true
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
            
            if ([Qt.Key_Left, Qt.Key_Right, Qt.Key_Up, Qt.Key_Down, Qt.Key_Home, Qt.Key_End, Qt.Key_PageUp, Qt.Key_PageDown].includes(event.key)) {
                // show delegate highlight on keyboard arrow press
                view.showHighlight = true;
            }
        }
    }
}
