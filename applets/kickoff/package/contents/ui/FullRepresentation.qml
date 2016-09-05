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

    property QtObject globalFavorites: rootModelFavorites

    state: "Normal"
    focus: true

    function switchToFavorites() {
        mainTabGroup.currentTab = favoritesPage;
        tabBar.currentTab = bookmarkButton;
        header.query = ""
        header.state = "hint";
        root.state = "Normal";
    }

    Kicker.AppsModel {
        id: rootModel

        appletInterface: plasmoid

        appNameFormat: plasmoid.configuration.showAppsByName ? 0 : 1
        flat: false
        showSeparators: false

        favoritesModel: Kicker.FavoritesModel {
            id: rootModelFavorites
            favorites: plasmoid.configuration.favorites

            onFavoritesChanged: {
                plasmoid.configuration.favorites = favorites;
            }
        }
    }

    Kicker.RunnerModel {
        id: runnerModel

        appletInterface: plasmoid

        runners: {
            var runners = ["services", "places"];

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
                    return "left";
                case PlasmaCore.Types.TopEdge:
                    return "top";
                case PlasmaCore.Types.RightEdge:
                    return "right";
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
        LayoutMirroring.enabled: Qt.application.layoutDirection == Qt.RightToLeft
        LayoutMirroring.childrenInherit: true

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

        currentTab: bookmarkButton
        onCurrentTabChanged: root.forceActiveFocus();

        Connections {
            target: plasmoid
            onExpandedChanged: {
                if (!expanded) {
                    switchToFavorites()
                }
            }
        }

        KickoffButton {
            id: bookmarkButton
            tab: favoritesPage
            iconSource: "bookmarks"
            text: i18n("Favorites")
        }
        KickoffButton {
            id: applicationButton
            tab: applicationsPage
            iconSource: "applications-other"
            text: i18n("Applications")
        }
        KickoffButton {
            id: computerButton
            tab: systemPage
            iconSource: pmSource.data["PowerDevil"] && pmSource.data["PowerDevil"]["Is Lid Present"] ? "computer-laptop" : "computer"
            text: i18n("Computer")
        }
        KickoffButton {
            id: usedButton
            tab: recentlyUsedPage
            iconSource: "view-history"
            text: i18n("History")
        }
        KickoffButton {
            id: leaveButton
            tab: leavePage
            iconSource: "system-log-out"
            text: i18n("Leave")
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
                if (header.input.focus) {
                    break;
                }
                if (!currentView.deactivateCurrentIndex()) {
                    if (root.state == "Applications") {
                        mainTabGroup.currentTab = favoritesPage;
                        tabBar.currentTab = bookmarkButton;
                    }
                    root.state = "Normal"
                }
                event.accepted = true;
                break;
            }
            case Qt.Key_Right: {
                if (header.input.focus) {
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
                    switchToFavorites();
                }
                event.accepted = true;
                break;
            }
            case Qt.Key_Menu: {
                currentView.openContextMenu();
                event.accepted = true;
                break;
            }

            default: { // forward key to searchView
                //header.query += event.text will break if the key is backspace,
                //since if the user continues to type, it will produce an invalid query,
                //having backspace as the first character
                if (event.key == Qt.Key_Backspace && header.query == "") {
                    return;
                }
                if (event.text != "" && !header.input.focus) {
                    root.currentView.listView.currentIndex = -1;

                    if (event.matches(StandardKey.Paste) ) {
                        header.input.paste();
                    } else if (! (event.key & Qt.Key_Escape)) {
                        //if special key, do nothing. Qt.Escape is 0x10000000 which happens to be a mask used for all special keys in Qt.
                        header.query = "";
                        header.query += event.text;
                    }
                    header.input.forceActiveFocus();
                    event.accepted = true;
                }
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
                visible: true
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
                visible: true
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
        }
    ] // states
}
