/*
    SPDX-FileCopyrightText: 2011 Martin Gräßlin <mgraesslin@kde.org>
    SPDX-FileCopyrightText: 2012 Gregor Taetzner <gregor@freenet.de>
    SPDX-FileCopyrightText: 2015-2018 Eike Hein <hein@kde.org>
    SPDX-FileCopyrightText: 2021 Mikel Johnson <mikel5764@gmail.com>
    SPDX-FileCopyrightText: 2021 Noah Davis <noahadvs@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Templates as T

import org.kde.plasma.plasmoid
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
    property alias section: view.section
    property alias highlight: view.highlight
    property alias view: view
    property bool isSidebar: false

    property bool mainContentView: false
    property bool hasSectionView: false

    /**
     * Request showing the section view
     */
    signal showSectionViewRequested(string sectionName)

    clip: view.height < view.contentHeight

    header: MouseArea {
        implicitHeight: KickoffSingleton.listItemMetrics.margins.top
        hoverEnabled: mainContentView || Plasmoid.configuration.switchCategoryOnHover
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
        hoverEnabled: mainContentView || Plasmoid.configuration.switchCategoryOnHover
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

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            contentWidth, // exclude padding to avoid scrollbars automatically affecting implicitWidth
                            implicitHeaderWidth2,
                            implicitFooterWidth2)

    leftPadding: verticalScrollBar.visible && root.mirrored ? verticalScrollBar.implicitWidth : 0
    rightPadding: verticalScrollBar.visible && !root.mirrored ? verticalScrollBar.implicitWidth : 0

    contentItem: ListView {
        id: view

        readonly property real availableWidth: width - leftMargin - rightMargin
        readonly property real availableHeight: height - topMargin - bottomMargin
        property bool movedWithKeyboard: false
        property bool movedWithWheel: false

        Accessible.role: Accessible.List

        implicitWidth: {
            let totalMargins = leftMargin + rightMargin
            if (mainContentView) {
                if (kickoff.mayHaveGridWithScrollBar) {
                    totalMargins += verticalScrollBar.implicitWidth
                }
                return KickoffSingleton.gridCellSize * kickoff.minimumGridRowCount + totalMargins
            }
            return contentWidth + totalMargins
        }
        implicitHeight: {
            // use grid cells to determine size
            let h = KickoffSingleton.gridCellSize * kickoff.minimumGridRowCount
            // If no grids are used, use the number of items that would fit in the grid height
            if (Plasmoid.configuration.favoritesDisplay !== 0 && Plasmoid.configuration.applicationsDisplay !== 0) {
                h = Math.floor(h / kickoff.listDelegateHeight) * kickoff.listDelegateHeight
            }
            return h + topMargin + bottomMargin
        }

        leftMargin: kickoff.backgroundMetrics.leftPadding
        rightMargin: kickoff.backgroundMetrics.rightPadding

        currentIndex: count > 0 ? 0 : -1
        focus: true
        interactive: height < contentHeight
        pixelAligned: true
        reuseItems: false // explicitly disabled because it doesn't work correctly with switching models like we do
        boundsBehavior: Flickable.StopAtBounds
        // default keyboard navigation doesn't allow focus reasons to be used
        // and eats up/down key events when at the beginning or end of the list.
        keyNavigationEnabled: false
        keyNavigationWraps: false

        // This is actually needed. The highlight will animate from thin to wide otherwise.
        highlightResizeDuration: 0
        highlightMoveDuration: 0
        highlight: PlasmaExtras.Highlight {
            // The default Z value for delegates is 1. The default Z value for the section delegate is 2.
            // The highlight gets a value of 3 while the drag is active and then goes back to the default value of 0.
            z: (root.currentItem?.Drag.active ?? false) ? 3 : 0

            pressed: ((view.currentItem as T.AbstractButton)?.down ?? false)
                && !((view.currentItem as AbstractKickoffItemDelegate)?.isCategoryListItem ?? false)

            active: view.activeFocus
                || (kickoff.contentArea === root
                    && kickoff.searchField.activeFocus)
        }

        onCountChanged: {
            if (!activeFocus) {
                currentIndex = (count > 0 ? 0 : -1)
            } else if (count > 0 && currentIndex !== -1) {
                positionViewAtIndex(currentIndex, ListView.Contain)
            }
        }

        delegate: KickoffListDelegate {
            width: view.availableWidth
        }

        // Without switch-on-hover, it's possible for the selected category and the hovered category to be adjacent.
        // When this happens, their highlights tuoch and look ungly without some artificial spacing added.
        spacing: root.isSidebar && !Plasmoid.configuration.switchCategoryOnHover ? Kirigami.Units.smallSpacing : 0

        section {
            property: "group"
            criteria: ViewSection.FullString
            delegate: PlasmaExtras.ListSectionHeader {
                required property string section

                width: view.availableWidth
                height: KickoffSingleton.compactListDelegateHeight
                text: section.length === 1 ? section.toUpperCase() : section

                HoverHandler {
                    enabled: root.hasSectionView
                    cursorShape: enabled ? Qt.PointingHandCursor : undefined
                }

                onClicked: root.showSectionViewRequested(text)
            }
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
            const atFirst = currentIndex === 0
            const atLast = currentIndex === count - 1
            if (count >= 1) {
                switch (event.key) {
                    case Qt.Key_Up: if (!atFirst) {
                        decrementCurrentIndex()

                        if (currentItem.isSeparator) {
                            decrementCurrentIndex()
                        }

                        focusCurrentItem(event, Qt.BacktabFocusReason)
                    } break
                    case Qt.Key_K: if (!atFirst && event.modifiers & Qt.ControlModifier) {
                        decrementCurrentIndex()
                        focusCurrentItem(event, Qt.BacktabFocusReason)
                    } break
                    case Qt.Key_Down: if (!atLast) {
                        incrementCurrentIndex()

                        if (currentItem.isSeparator) {
                            incrementCurrentIndex()
                        }

                        focusCurrentItem(event, Qt.TabFocusReason)
                    } break
                    case Qt.Key_J: if (!atLast && event.modifiers & Qt.ControlModifier) {
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
