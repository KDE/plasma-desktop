/*
    Copyright (C) 2011  Martin Gräßlin <mgraesslin@kde.org>
    Copyright (C) 2012  Gregor Taetzner <gregor@freenet.de>
    Copyright (C) 2012  Marco Martin <mart@kde.org>
    Copyright (C) 2013 2014 David Edmundson <davidedmundson@kde.org>
    Copyright 2014 Sebastian Kügler <sebas@kde.org>

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
import QtQuick 2.3
import org.kde.plasma.plasmoid 2.0
import QtQuick.Layouts 1.1
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.kquickcontrolsaddons 2.0

import org.kde.plasma.private.kicker 0.1 as Kicker

Item {
    id: root
    Layout.minimumWidth: units.gridUnit * 26
    Layout.minimumHeight: units.gridUnit * 34

    property string previousState
    property bool switchTabsOnHover: plasmoid.configuration.switchTabsOnHover
    property Item currentView: mainTabGroup.currentTab.decrementCurrentIndex ? mainTabGroup.currentTab : mainTabGroup.currentTab.item
    property KickoffButton firstButton: null
    property var configMenuItems

    property QtObject globalFavorites: rootModelFavorites

    state: "Normal"

    onFocusChanged: {
        header.input.forceActiveFocus();
    }

    function switchToInitial() {
        if (firstButton != null) {
            mainTabGroup.currentTab = firstButton.tab;
            tabBar.currentTab = firstButton;
            header.query = ""
            header.state = "hint";
            root.state = "Normal";
        }
    }

    Kicker.DragHelper {
        id: dragHelper

        dragIconSize: units.iconSizes.medium
        onDropped: kickoff.dragSource = null
    }

    Kicker.AppsModel {
        id: rootModel

        appletInterface: plasmoid

        appNameFormat: plasmoid.configuration.showAppsByName ? 0 : 1
        flat: false
        sorted: plasmoid.configuration.alphaSort
        showSeparators: false

        favoritesModel: Kicker.KAStatsFavoritesModel {
            id: rootModelFavorites
            favorites: plasmoid.configuration.favorites

            onFavoritesChanged: {
                plasmoid.configuration.favorites = favorites;
            }
        }

        Component.onCompleted: {
            favoritesModel.initForClient("org.kde.plasma.kickoff.favorites.instance-" + plasmoid.id)

            if (!plasmoid.configuration.favoritesPortedToKAstats) {
                favoritesModel.portOldFavorites(plasmoid.configuration.favorites);
                plasmoid.configuration.favoritesPortedToKAstats = true;
            }

            rootModel.refresh();
        }
    }

    Kicker.RunnerModel {
        id: runnerModel

        appletInterface: plasmoid

        runners: {
            var runners = ["services", "places", "desktopsessions",
                           "PowerDevil"];

            if (plasmoid.configuration.useExtraRunners) {
                runners = runners.concat(plasmoid.configuration.runners);
            }
            return runners;
        }
        mergeResults: true

        favoritesModel: globalFavorites
    }

    PlasmaCore.DataSource {
        id: pmSource
        engine: "powermanagement"
        connectedSources: ["PowerDevil"]
    }

    PlasmaCore.Svg {
        id: lineSvg
        imagePath: "widgets/line"
    }

    PlasmaCore.Svg {
        id: arrowsSvg

        imagePath: "widgets/arrows"
        size: "16x16"
    }

    Timer {
        id: clickTimer

        property Item pendingButton

        interval: 250

        onTriggered: pendingButton.clicked()
    }

    Header {
        id: header
    }

    Item {
        id: mainArea
        anchors.topMargin: mainTabGroup.state == "top" ? units.smallSpacing : 0

        PlasmaComponents.TabGroup {
            id: mainTabGroup
            currentTab: favoritesPage

            anchors {
                fill: parent
            }

            //pages
            FavoritesView {
                id: favoritesPage
            }
            PlasmaExtras.ConditionalLoader {
                id: applicationsPage
                when: mainTabGroup.currentTab == applicationsPage
                source: Qt.resolvedUrl("ApplicationsView.qml")
            }
            PlasmaExtras.ConditionalLoader {
                id: systemPage
                when: mainTabGroup.currentTab == systemPage
                source: Qt.resolvedUrl("ComputerView.qml")
            }
            PlasmaExtras.ConditionalLoader {
                id: recentlyUsedPage
                when: mainTabGroup.currentTab == recentlyUsedPage
                source: Qt.resolvedUrl("RecentlyUsedView.qml")
            }
            PlasmaExtras.ConditionalLoader {
                id: oftenUsedPage
                when: mainTabGroup.currentTab == oftenUsedPage
                source: Qt.resolvedUrl("OftenUsedView.qml")
            }
            PlasmaExtras.ConditionalLoader {
                id: leavePage
                when: mainTabGroup.currentTab == leavePage
                source: Qt.resolvedUrl("LeaveView.qml")
            }
            PlasmaExtras.ConditionalLoader {
                id: searchPage
                when: root.state == "Search"
                //when: mainTabGroup.currentTab == searchPage || root.state == "Search"
                source: Qt.resolvedUrl("SearchView.qml")
            }

            state: {
                switch (plasmoid.location) {
                case PlasmaCore.Types.LeftEdge:
                    return LayoutMirroring.enabled ? "right" : "left";
                case PlasmaCore.Types.TopEdge:
                    return "top";
                case PlasmaCore.Types.RightEdge:
                    return LayoutMirroring.enabled ? "left" : "right";
                case PlasmaCore.Types.BottomEdge:
                default:
                    return "bottom";
                }
            }
            states: [
                State {
                    name: "left"
                    AnchorChanges {
                        target: header
                        anchors {
                            left: root.left
                            top: undefined
                            right: root.right
                            bottom: root.bottom
                        }
                    }
                    PropertyChanges {
                        target: header
                        width: header.implicitWidth
                    }

                    AnchorChanges {
                        target: mainArea
                        anchors {
                            left: tabBar.right
                            top: root.top
                            right: root.right
                            bottom: header.top
                        }
                    }

                    AnchorChanges {
                        target: tabBar
                        anchors {
                            left: root.left
                            top: root.top
                            right: undefined
                            bottom: header.top
                        }
                    }
                },
                State {
                    name: "top"
                    AnchorChanges {
                        target: header
                        anchors {
                            left: root.left
                            top: undefined
                            right: root.right
                            bottom: root.bottom
                        }
                    }
                    PropertyChanges {
                        target: header
                        height: header.implicitHeight
                    }

                    AnchorChanges {
                        target: mainArea
                        anchors {
                            left: root.left
                            top: tabBar.bottom
                            right: root.right
                            bottom: header.top
                        }
                    }

                    AnchorChanges {
                        target: tabBar
                        anchors {
                            left: root.left
                            top: root.top
                            right: root.right
                            bottom: undefined
                        }
                    }
                },
                State {
                    name: "right"
                    AnchorChanges {
                        target: header
                        anchors {
                            left: root.left
                            top: undefined
                            right: root.right
                            bottom: root.bottom
                        }
                    }
                    PropertyChanges {
                        target: header
                        width: header.implicitWidth
                    }

                    AnchorChanges {
                        target: mainArea
                        anchors {
                            left: root.left
                            top: root.top
                            right: tabBar.left
                            bottom: header.top
                        }
                    }

                    AnchorChanges {
                        target: tabBar
                        anchors {
                            left: undefined
                            top: root.top
                            right: root.right
                            bottom: header.top
                        }
                    }
                },
                State {
                    name: "bottom"
                    AnchorChanges {
                        target: header
                        anchors {
                            left: root.left
                            top: root.top
                            right: root.right
                            bottom: undefined
                        }
                    }
                    PropertyChanges {
                        target: header
                        height: header.implicitHeight
                    }

                    AnchorChanges {
                        target: mainArea
                        anchors {
                            left: root.left
                            top: header.bottom
                            right: root.right
                            bottom: tabBar.top
                        }
                    }

                    AnchorChanges {
                        target: tabBar
                        anchors {
                            left: root.left
                            top: undefined
                            right: root.right
                            bottom: root.bottom
                        }
                    }
                }
            ]
        } // mainTabGroup
    }

    PlasmaComponents.TabBar {
        id: tabBar

        property int count: 5 // updated in createButtons()

        width: {
            if (!visible) {
                return 0;
            }

            switch (plasmoid.location) {
            case PlasmaCore.Types.LeftEdge:
            case PlasmaCore.Types.RightEdge:
                return units.gridUnit * 5;
            default:
                return 0;
            }
        }

        height: {
            if (!visible) {
                return 0;
            }

            switch (plasmoid.location) {
            case PlasmaCore.Types.LeftEdge:
            case PlasmaCore.Types.RightEdge:
                return 0;
            default:
                return units.gridUnit * 5;
            }
        }

        Behavior on width {
            NumberAnimation { duration: units.longDuration; easing.type: Easing.InQuad; }
            enabled: plasmoid.expanded
        }
        Behavior on height {
            NumberAnimation { duration: units.longDuration; easing.type: Easing.InQuad; }
            enabled: plasmoid.expanded
        }

        tabPosition: {
            switch (plasmoid.location) {
            case PlasmaCore.Types.TopEdge:
                return Qt.TopEdge;
            case PlasmaCore.Types.LeftEdge:
                return Qt.LeftEdge;
            case PlasmaCore.Types.RightEdge:
                return Qt.RightEdge;
            default:
                return Qt.BottomEdge;
            }
        }

        onCurrentTabChanged: header.input.forceActiveFocus();

        Connections {
            target: plasmoid
            onExpandedChanged: {
                if(menuItemsChanged()) {
                    createButtons();
                }
                if (!expanded) {
                    switchToInitial();
                }
            }
        }
    } // tabBar

    Keys.forwardTo: [tabBar.layout]

    Keys.onPressed: {

        if (mainTabGroup.currentTab == applicationsPage) {
            if (event.key != Qt.Key_Tab) {
                root.state = "Applications";
            }
        }

        switch(event.key) {
            case Qt.Key_Up: {
                currentView.decrementCurrentIndex();
                event.accepted = true;
                break;
            }
            case Qt.Key_Down: {
                currentView.incrementCurrentIndex();
                event.accepted = true;
                break;
            }
            case Qt.Key_Left: {
                if (header.input.focus && header.state == "query") {
                    break;
                }
                if (!currentView.deactivateCurrentIndex()) {
                    if (root.state == "Applications") {
                        mainTabGroup.currentTab = firstButton.tab;
                        tabBar.currentTab = firstButton;
                    }
                    root.state = "Normal"
                }
                event.accepted = true;
                break;
            }
            case Qt.Key_Right: {
                if (header.input.focus && header.state == "query") {
                    break;
                }
                currentView.activateCurrentIndex();
                event.accepted = true;
                break;
            }
            case Qt.Key_Tab: {
                root.state == "Applications" ? root.state = "Normal" : root.state = "Applications";
                event.accepted = true;
                break;
            }
            case Qt.Key_Enter:
            case Qt.Key_Return: {
                currentView.activateCurrentIndex(1);
                event.accepted = true;
                break;
            }
            case Qt.Key_Escape: {
                if (header.state != "query") {
                    plasmoid.expanded = false;
                } else {
                    header.query = "";
                }
                event.accepted = true;
                break;
            }
            case Qt.Key_Menu: {
                currentView.openContextMenu();
                event.accepted = true;
                break;
            }
            default:
                if (!header.input.focus) {
                    header.input.forceActiveFocus();
                }
        }
    }

    states: [
        State {
            name: "Normal"
            PropertyChanges {
                target: root
                Keys.forwardTo: [tabBar.layout]
            }
            PropertyChanges {
                target: tabBar
                visible: tabBar.count > 1
            }
        },
        State {
            name: "Applications"
            PropertyChanges {
                target: root
                Keys.forwardTo: [root]
            }
            PropertyChanges {
                target: tabBar
                visible: tabBar.count > 1
            }
        },
        State {
            name: "Search"
            PropertyChanges {
                target: tabBar
                visible: false
            }
            PropertyChanges {
                target: mainTabGroup
                currentTab: searchPage
            }
            PropertyChanges {
                target: root
                Keys.forwardTo: [root]
            }
        }
    ] // states

    function getButtonDefinition(name) {
        switch(name) {
        case "bookmark":
            return {id: "bookmarkButton", tab: favoritesPage, iconSource: "bookmarks", text: i18n("Favorites")};
        case "application":
            return {id: "applicationButton", tab: applicationsPage, iconSource: "applications-other", text: i18n("Applications")};
        case "computer":
            return {id: "computerButton", tab: systemPage, iconSource: pmSource.data["PowerDevil"] && pmSource.data["PowerDevil"]["Is Lid Present"] ? "computer-laptop" : "computer", text: i18n("Computer")};
        case "used":
            return {id: "usedButton", tab: recentlyUsedPage, iconSource: "view-history", text: i18n("History")};
        case "oftenUsed":
            return {id: "usedButton", tab: oftenUsedPage, iconSource: "office-chart-pie", text: i18n("Often Used")};
        case "leave":
            return {id: "leaveButton", tab: leavePage, iconSource: "system-log-out", text: i18n("Leave")};
        }
    }

    Component {
        id: kickoffButton
        KickoffButton {}
    }


    Component.onCompleted: {
        createButtons();
    }

    function getEnabled(configuration) {
        var res = [];
        for(var i = 0; i < configuration.length; i++) {
            var confItemName = configuration[i].substring(0, configuration[i].indexOf(":"));
            var confItemEnabled = configuration[i].substring(configuration[i].length-1) == "t";
            if(confItemEnabled) {
                res.push(confItemName);
            }
        }

        return res;
    }

    function createButtons() {
        configMenuItems = plasmoid.configuration.menuItems;
        var menuItems = getEnabled(plasmoid.configuration.menuItems);
        tabBar.count = menuItems.length

        // remove old menu items
        for(var i = tabBar.layout.children.length -1; i >= 0; i--)  {
            if(tabBar.layout.children[i].objectName == "KickoffButton") {
                tabBar.layout.children[i].destroy();
            }
        }

        for (var i = 0; i < menuItems.length; i++) {
             var props = getButtonDefinition(menuItems[i]);
             var button = kickoffButton.createObject(tabBar.layout, props);
             if(i == 0) {
                 firstButton = button;
                 switchToInitial();
             }
        }

    }

    function menuItemsChanged() {
        if(configMenuItems.length != plasmoid.configuration.menuItems.length) {
            return true;
        }

        for(var i = 0; i < configMenuItems.length; i++) {
            if(configMenuItems[i] != plasmoid.configuration.menuItems[i]) {
                return true;
            }
        }
        return false;
    }
}
