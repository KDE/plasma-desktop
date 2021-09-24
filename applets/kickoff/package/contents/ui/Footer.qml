/*
 *    Copyright (C) 2021 by Mikel Johnson <mikel5764@gmail.com>
 *    Copyright (C) 2021 by Noah Davis <noahadvs@gmail.com>
 *
 *    This program is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License along
 *    with this program; if not, write to the Free Software Foundation, Inc.,
 *    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

import QtQuick 2.15
import QtQml 2.15
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 3.0 as PC3
import org.kde.plasma.extras 2.0 as PlasmaExtras

PlasmaExtras.PlasmoidHeading {
    id: root

    readonly property alias tabBar: tabBar
    property real preferredTabBarWidth: 0
    readonly property alias leaveButtons: leaveButtons

    contentWidth: tabBar.implicitWidth + root.spacing + root.spacing + leaveButtons.implicitWidth
    contentHeight: leaveButtons.implicitHeight

    // We use an increased vertical padding to improve touch usability
    leftPadding: KickoffSingleton.leftPadding
    rightPadding: KickoffSingleton.rightPadding
    topPadding: PlasmaCore.Units.smallSpacing*2
    bottomPadding: PlasmaCore.Units.smallSpacing*2

    leftInset: 0
    rightInset: 0
    topInset: 0
    bottomInset: 0

    spacing: KickoffSingleton.spacing
    position: PC3.ToolBar.Footer

    PC3.TabBar {
        id: tabBar
        property real tabWidth: Math.max(applicationsTab.implicitWidth, placesTab.implicitWidth)
        focus: true
        width: root.preferredTabBarWidth > 0 ? root.preferredTabBarWidth : implicitWidth
        implicitWidth: contentWidth + leftPadding + rightPadding
        implicitHeight: contentHeight + topPadding + bottomPadding

        // This is needed to keep the sparators horizontally aligned
        leftPadding: mirrored ? root.spacing : 0
        rightPadding: !mirrored ? root.spacing : 0

        anchors {
            left: parent.left
            top: parent.top
            bottom:parent.bottom
        }

        position: PC3.TabBar.Footer

        contentItem: ListView {
            id: tabBarListView
            focus: true
            model: tabBar.contentModel
            currentIndex: tabBar.currentIndex

            spacing: tabBar.spacing
            orientation: ListView.Horizontal
            boundsBehavior: Flickable.StopAtBounds
            flickableDirection: Flickable.AutoFlickIfNeeded
            snapMode: ListView.SnapToItem

            highlightMoveDuration: PlasmaCore.Units.longDuration
            highlightRangeMode: ListView.ApplyRange
            preferredHighlightBegin: tabBar.tabWidth
            preferredHighlightEnd: width - tabBar.tabWidth
            highlight: PlasmaCore.FrameSvgItem {
                anchors.top: tabBarListView.contentItem.top
                anchors.bottom: tabBarListView.contentItem.bottom
                anchors.topMargin: -root.topPadding
                anchors.bottomMargin: -root.bottomPadding
                imagePath: "widgets/tabbar"
                prefix: tabBar.position == PC3.TabBar.Header ? "north-active-tab" : "south-active-tab"
                colorGroup: PlasmaCore.ColorScope.colorGroup
            }
            keyNavigationEnabled: false
        }

        PC3.TabButton {
            id: applicationsTab
            focus: true
            width: tabBar.tabWidth
            anchors.top: tabBarListView.contentItem.top
            anchors.bottom: tabBarListView.contentItem.bottom
            anchors.topMargin: -root.topPadding
            anchors.bottomMargin: -root.bottomPadding
            icon.width: PlasmaCore.Units.iconSizes.smallMedium
            icon.height: PlasmaCore.Units.iconSizes.smallMedium
            icon.name: "applications-other"
            text: i18n("Applications")
            KeyNavigation.backtab: KickoffSingleton.contentArea ? KickoffSingleton.contentArea : null
        }
        PC3.TabButton {
            id: placesTab
            width: tabBar.tabWidth
            anchors.top: tabBarListView.contentItem.top
            anchors.bottom: tabBarListView.contentItem.bottom
            anchors.topMargin: -root.topPadding
            anchors.bottomMargin: -root.bottomPadding
            icon.width: PlasmaCore.Units.iconSizes.smallMedium
            icon.height: PlasmaCore.Units.iconSizes.smallMedium
            icon.name: "compass"
            text: i18n("Places") //Explore?
        }

        Connections {
            target: plasmoid
            function onExpandedChanged() {
                if(!plasmoid.expanded) {
                    tabBar.currentIndex = 0
                }
            }
        }

        Keys.onLeftPressed: {
            let moved = false
            if (LayoutMirroring.enabled && currentIndex === 0) {
                incrementCurrentIndex()
                currentItem.forceActiveFocus(Qt.TabFocusReason)
                moved = true
            } else if (currentIndex === 1) {
                decrementCurrentIndex()
                currentItem.forceActiveFocus(Qt.BacktabFocusReason)
                moved = true
            }
            if (!moved && currentIndex === 1) {
                leaveButtons.nextItemInFocusChain().forceActiveFocus(Qt.TabFocusReason)
            }
        }
        Keys.onRightPressed: {
            let moved = false
            if (LayoutMirroring.enabled && currentIndex === 1) {
                decrementCurrentIndex()
                currentItem.forceActiveFocus(Qt.BacktabFocusReason)
                moved = true
            } else if (currentIndex === 0) {
                incrementCurrentIndex()
                currentItem.forceActiveFocus(Qt.TabFocusReason)
                moved = true
            }
            if (!moved && currentIndex === 1) {
                leaveButtons.nextItemInFocusChain().forceActiveFocus(Qt.TabFocusReason)
            }
        }
        Keys.onUpPressed: KickoffSingleton.sideBar.forceActiveFocus(Qt.BacktabFocusReason)
    }

    LeaveButtons {
        id: leaveButtons
        anchors {
            right: parent.right
            top: parent.top
            bottom: parent.bottom
            leftMargin: root.spacing
        }
        Keys.onUpPressed: KickoffSingleton.contentArea.forceActiveFocus(Qt.BacktabFocusReason)
    }

    Behavior on height {
        enabled: plasmoid.expanded
        NumberAnimation {
            duration: PlasmaCore.Units.longDuration
            easing.type: Easing.InQuad
        }
    }

    // Using item containing WheelHandler instead of MouseArea because
    // MouseArea doesn't keep track to the total amount of rotation.
    // Keeping track of the total amount of rotation makes it work
    // better for touch pads.
    Item {
        id: mouseItem
        parent: root
        anchors.left: parent.left
        height: root.height
        width: tabBar.width
        z: 1 // Has to be above contentItem to recieve mouse wheel events
        WheelHandler {
            id: tabScrollHandler
            acceptedDevices: PointerDevice.Mouse | PointerDevice.TouchPad
            onWheel: {
                let shouldDec = rotation >= 15
                let shouldInc = rotation <= -15
                let shouldReset = (rotation > 0 && tabBar.currentIndex == 0) || (rotation < 0 && tabBar.currentIndex == tabBar.count-1)
                if (shouldDec) {
                    tabBar.decrementCurrentIndex();
                    rotation = 0
                } else if (shouldInc) {
                    tabBar.incrementCurrentIndex();
                    rotation = 0
                } else if (shouldReset) {
                    rotation = 0
                }
            }
        }
    }
}
