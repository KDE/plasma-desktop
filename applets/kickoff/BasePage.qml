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
    // default for first run, to fit everything on the default sidebar
    // Plasma only resizes based on implicit size on first run
    property real initialSideBarHeight: {
        const sB = sideBarLoader.item as KickoffListView
        return sB.calculateTotalHeightForContentHeight(sB.view.contentHeight)
    }

    property alias sideBarComponent: sideBarLoader.sourceComponent
    property alias sideBarItem: sideBarLoader.item
    property alias contentAreaComponent: contentAreaLoader.sourceComponent
    property alias contentAreaItem: contentAreaLoader.item

    property alias implicitSideBarWidth: sideBarLoader.implicitWidth
    property alias implicitSideBarHeight: sideBarLoader.implicitHeight

    implicitWidth: preferredSideBarWidth + separator.implicitWidth + contentAreaLoader.implicitWidth
    implicitHeight: Math.max(preferredSideBarHeight, contentAreaLoader.implicitHeight, Math.min(initialSideBarHeight, Math.round(Screen.desktopAvailableHeight * 0.75)))

    Component.onCompleted: {
        // break binding so later category changes on first run don't change kickoff's size
        initialSideBarHeight = initialSideBarHeight
    }


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
            // Handles both the arrow keys and Ctrl+H/J/K/L
            Keys.onPressed: event => {
                switch (KickoffSingleton.navigationKey(event)) {
                    case Qt.Key_Left: if (kickoff.sideBarOnRight) {
                        contentAreaLoader.forceActiveFocus();
                    } break
                    case Qt.Key_Right: if (!kickoff.sideBarOnRight) {
                        contentAreaLoader.forceActiveFocus();
                    } break
                    case Qt.Key_Up:
                        kickoff.header.nextItemInFocusChain()
                            .forceActiveFocus(Qt.BacktabFocusReason);
                        break
                    case Qt.Key_Down:
                        (kickoff.paneSwap ? kickoff.footer.leaveButtons.nextItemInFocusChain() : kickoff.footer.tabBar)
                            .forceActiveFocus(Qt.TabFocusReason);
                        break
                    default:
                        return;
                }
                event.accepted = true;
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
        // Handles both the arrow keys and Ctrl+H/J/K/L
        Keys.onPressed: event => {
            switch (KickoffSingleton.navigationKey(event)) {
                case Qt.Key_Left: if (!kickoff.sideBarOnRight) {
                    sideBarLoader.forceActiveFocus();
                } break
                case Qt.Key_Right: if (kickoff.sideBarOnRight) {
                    sideBarLoader.forceActiveFocus();
                } break
                case Qt.Key_Up:
                    kickoff.searchField.forceActiveFocus(Qt.BacktabFocusReason);
                    break
                case Qt.Key_Down:
                    (kickoff.paneSwap ? kickoff.footer.tabBar : kickoff.footer.leaveButtons.nextItemInFocusChain())
                        .forceActiveFocus(Qt.TabFocusReason)
                    break
                default:
                    return;
            }
            event.accepted = true;
        }
    }
}
