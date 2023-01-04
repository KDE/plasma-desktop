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
import org.kde.plasma.components 2.0 as PC2
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
    property bool hasSectionView: false

    /**
     * Request showing the section view
     */
    signal showSectionViewRequested(string sectionName)

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

        Accessible.role: Accessible.List

        implicitWidth: {
            let totalMargins = leftMargin + rightMargin
            if (mainContentView) {
                if (plasmoid.rootItem.mayHaveGridWithScrollBar) {
                    totalMargins += verticalScrollBar.implicitWidth
                }
                return KickoffSingleton.gridCellSize * plasmoid.rootItem.minimumGridRowCount + totalMargins
            }
            return contentWidth + totalMargins
        }
        implicitHeight: {
            // use grid cells to determine size
            let h = KickoffSingleton.gridCellSize * plasmoid.rootItem.minimumGridRowCount
            // If no grids are used, use the number of items that would fit in the grid height
            if (plasmoid.configuration.favoritesDisplay !== 0 && plasmoid.configuration.applicationsDisplay !== 0) {
                h = Math.floor(h / plasmoid.rootItem.listDelegateHeight) * plasmoid.rootItem.listDelegateHeight
            }
            return h + topMargin + bottomMargin
        }

        leftMargin: plasmoid.rootItem.backgroundMetrics.leftPadding
        rightMargin: plasmoid.rootItem.backgroundMetrics.rightPadding

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
                || (plasmoid.rootItem.contentArea === root
                    && plasmoid.rootItem.searchField.activeFocus) ? 1 : 0.5
            imagePath: "widgets/viewitem"
            prefix: "hover"
        }

        delegate: KickoffListDelegate {
            width: view.availableWidth
        }

        section {
            property: "group"
            criteria: ViewSection.FullString
            delegate: PC3.AbstractButton {
                width: view.availableWidth
                height: KickoffSingleton.compactListDelegateHeight

                PC3.Label {
                    id: contentLabel
                    anchors.left: parent.left
                    width: section.length === 1
                        ? KickoffSingleton.compactListDelegateContentHeight + leftPadding + rightPadding
                        : parent.width
                    height: parent.height
                    leftPadding: view.effectiveLayoutDirection === Qt.LeftToRight
                        ? KickoffSingleton.listItemMetrics.margins.left : 0
                    rightPadding: view.effectiveLayoutDirection === Qt.RightToLeft
                        ? KickoffSingleton.listItemMetrics.margins.right : 0
                    horizontalAlignment: section.length === 1 ? Text.AlignHCenter : Text.AlignLeft
                    verticalAlignment: Text.AlignVCenter
                    maximumLineCount: 1
                    elide: Text.ElideRight
                    font.pixelSize: KickoffSingleton.compactListDelegateContentHeight
                    enabled: hoverHandler.hovered
                    text: section.length === 1 ? section.toUpperCase() : section
                    textFormat: Text.PlainText
                }

                HoverHandler {
                    id: hoverHandler
                    enabled: root.hasSectionView
                    cursorShape: enabled ? Qt.PointingHandCursor : undefined
                }

                onClicked: root.showSectionViewRequested(contentLabel.text)
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
            // `* PlasmaCore.Units.devicePixelRatio` is needed on X11
            // because Plasma doesn't support Qt scaling.
            horizontalStepSize: 20 * Qt.styleHints.wheelScrollLines * PlasmaCore.Units.devicePixelRatio
            verticalStepSize: 20 * Qt.styleHints.wheelScrollLines * PlasmaCore.Units.devicePixelRatio
        }

        Connections {
            target: plasmoid
            function onExpandedChanged() {
                if (plasmoid.expanded) {
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
                }
            }
            movedWithKeyboard = event.accepted
            if (movedWithKeyboard) {
                movedWithKeyboardTimer.restart()
            }
        }
    }
}
