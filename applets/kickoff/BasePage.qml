/*
    SPDX-FileCopyrightText: 2011 Martin Gräßlin <mgraesslin@kde.org>
    SPDX-FileCopyrightText: 2012 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2015-2018 Eike Hein <hein@kde.org>
    SPDX-FileCopyrightText: 2021 Mikel Johnson <mikel5764@gmail.com>
    SPDX-FileCopyrightText: 2021 Noah Davis <noahadvs@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Templates as T
import org.kde.ksvg as KSvg
import org.kde.plasma.plasmoid
import org.kde.plasma.workspace.trianglemousefilter

FocusScope {
    id: root

    property real preferredSideBarWidth: implicitSideBarWidth
    property real preferredSideBarHeight: implicitSideBarHeight

    property alias sideBarComponent: sideBarLoader.sourceComponent
    property alias sideBarItem: sideBarLoader.item
    property alias contentAreaComponent: contentAreaLoader.sourceComponent
    property alias contentAreaItem: contentAreaLoader.item

    property alias implicitSideBarWidth: sideBarLoader.implicitWidth
    property alias implicitSideBarHeight: sideBarLoader.implicitHeight

    implicitWidth: preferredSideBarWidth + separator.implicitWidth + contentAreaLoader.implicitWidth
    implicitHeight: Math.max(preferredSideBarHeight, contentAreaLoader.implicitHeight)

    TriangleMouseFilter {
        id: sideBarFilter
        active: Plasmoid.configuration.switchCategoryOnHover
        anchors {
            top: parent.top
            left: parent.left
            bottom: parent.bottom
        }
        LayoutMirroring.enabled: kickoff.sideBarOnRight
        implicitWidth: root.preferredSideBarWidth
        implicitHeight: root.preferredSideBarHeight
        edge: kickoff.sideBarOnRight ? Qt.LeftEdge : Qt.RightEdge
        blockFirstEnter: true
        Loader {
            id: sideBarLoader
            anchors.fill: parent
            // When positioned after the content area, Tab should go to the start of the footer focus chain
            Keys.onTabPressed: event => {
                (kickoff.paneSwap ? kickoff.footer.nextItemInFocusChain() : contentAreaLoader)
                    .forceActiveFocus(Qt.TabFocusReason);
            }
            Keys.onBacktabPressed: event => {
                (kickoff.paneSwap ? contentAreaLoader : kickoff.header.pinButton)
                    .forceActiveFocus(Qt.BacktabFocusReason);
            }
            Keys.onLeftPressed: event => {
                if (kickoff.sideBarOnRight) {
                    contentAreaLoader.forceActiveFocus();
                }
            }
            Keys.onRightPressed: event => {
                if (!kickoff.sideBarOnRight) {
                    contentAreaLoader.forceActiveFocus();
                }
            }
            Keys.onUpPressed: event => {
                kickoff.header.nextItemInFocusChain()
                    .forceActiveFocus(Qt.BacktabFocusReason);
            }
            Keys.onDownPressed: event => {
                (kickoff.paneSwap ? kickoff.footer.leaveButtons.nextItemInFocusChain() : kickoff.footer.tabBar)
                    .forceActiveFocus(Qt.TabFocusReason);
            }
        }
    }
    KSvg.SvgItem {
        id: separator
        anchors {
            top: parent.top
            left: sideBarFilter.right
            bottom: parent.bottom
        }
        LayoutMirroring.enabled: kickoff.sideBarOnRight
        implicitWidth: naturalSize.width
        implicitHeight: implicitWidth
        elementId: "vertical-line"
        svg: KickoffSingleton.lineSvg
    }
    Loader {
        id: contentAreaLoader
        focus: true
        anchors {
            top: parent.top
            left: separator.right
            right: parent.right
            bottom: parent.bottom
        }
        LayoutMirroring.enabled: kickoff.sideBarOnRight
        // When positioned after the sidebar, Tab should go to the start of the footer focus chain
        Keys.onTabPressed: event => {
            (kickoff.paneSwap ? sideBarLoader : kickoff.footer.nextItemInFocusChain())
                .forceActiveFocus(Qt.TabFocusReason)
        }
        Keys.onBacktabPressed: event => {
            (kickoff.paneSwap ? kickoff.header.avatar : sideBarLoader)
                .forceActiveFocus(Qt.BacktabFocusReason)
        }
        Keys.onLeftPressed: event => {
            if (!kickoff.sideBarOnRight) {
                sideBarLoader.forceActiveFocus();
            }
        }
        Keys.onRightPressed: event => {
            if (kickoff.sideBarOnRight) {
                sideBarLoader.forceActiveFocus();
            }
        }
        Keys.onUpPressed: event => {
            kickoff.searchField.forceActiveFocus(Qt.BacktabFocusReason);
        }
        Keys.onDownPressed: event => {
            (kickoff.paneSwap ? kickoff.footer.tabBar : kickoff.footer.leaveButtons.nextItemInFocusChain())
                .forceActiveFocus(Qt.TabFocusReason)
        }
    }
}
