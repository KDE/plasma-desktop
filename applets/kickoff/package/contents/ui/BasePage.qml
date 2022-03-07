/*
    Copyright (C) 2011  Martin Gräßlin <mgraesslin@kde.org>
    Copyright (C) 2012 Marco Martin <mart@kde.org>
    Copyright (C) 2015-2018  Eike Hein <hein@kde.org>
    Copyright (C) 2021 by Mikel Johnson <mikel5764@gmail.com>
    Copyright (C) 2021 by Noah Davis <noahadvs@gmail.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/
import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Templates 2.15 as T
import QtQml 2.15
import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 3.0 as PC3
import org.kde.plasma.private.kicker 0.1 as Kicker

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

    implicitWidth: root.preferredSideBarWidth + separator.implicitWidth + contentAreaLoader.implicitWidth
    implicitHeight: Math.max(root.preferredSideBarHeight, contentAreaLoader.implicitHeight)

    Kicker.TriangleMouseFilter {
        id: sideBarFilter
        anchors {
            left: parent.left
            top: parent.top
            bottom: parent.bottom
        }
        implicitWidth: root.preferredSideBarWidth
        implicitHeight: root.preferredSideBarHeight
        edge: LayoutMirroring.enabled ? Qt.LeftEdge : Qt.RightEdge
        Loader {
            id: sideBarLoader
            anchors.fill: parent
            // backtab is implicitly set by the last button in Header.qml
            KeyNavigation.tab: root.contentAreaItem
            KeyNavigation.right: contentAreaLoader
            Keys.onUpPressed: Plasmoid.rootItem.header.nextItemInFocusChain().forceActiveFocus(Qt.BacktabFocusReason)
            Keys.onDownPressed: Plasmoid.rootItem.footer.tabBar.forceActiveFocus(Qt.TabFocusReason)
        }
    }
    PlasmaCore.SvgItem {
        id: separator
        anchors {
            left: sideBarFilter.right
            top: parent.top
            bottom: parent.bottom
        }
        implicitWidth: naturalSize.width
        implicitHeight: implicitWidth
        elementId: "vertical-line"
        svg: KickoffSingleton.lineSvg
    }
    Loader {
        id: contentAreaLoader
        focus: true
        anchors {
            left: separator.right
            right: parent.right
            top: parent.top
            bottom: parent.bottom
        }
        KeyNavigation.backtab: root.sideBarItem
        // Tab should go to the start of the footer focus chain
        KeyNavigation.tab: Plasmoid.rootItem.footer.nextItemInFocusChain()
        KeyNavigation.left: sideBarLoader
        Keys.onUpPressed: Plasmoid.rootItem.searchField.forceActiveFocus(Qt.BacktabFocusReason)
        Keys.onDownPressed: Plasmoid.rootItem.footer.leaveButtons.nextItemInFocusChain().forceActiveFocus(Qt.TabFocusReason)
    }
}
