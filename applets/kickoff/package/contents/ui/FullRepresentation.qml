/*
    Copyright (C) 2011  Martin Gräßlin <mgraesslin@kde.org>
    Copyright (C) 2012  Gregor Taetzner <gregor@freenet.de>
    Copyright (C) 2012  Marco Martin <mart@kde.org>
    Copyright (C) 2013 2014 David Edmundson <davidedmundson@kde.org>
    Copyright 2014 Sebastian Kügler <sebas@kde.org>
    Copyright (C) 2021 by Mikel Johnson <mikel5764@gmail.com>

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
import QtQuick 2.12
import QtQuick.Layouts 1.1
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents // for TabGroup
import org.kde.plasma.components 3.0 as PC3
import org.kde.plasma.extras 2.0 as PlasmaExtras

import org.kde.plasma.private.kicker 0.1 as Kicker

Item {
    id: root
    Layout.minimumWidth: Math.round(PlasmaCore.Units.gridUnit * 26 * 1.5)
    Layout.maximumWidth: Layout.minimumWidth

    Layout.minimumHeight: PlasmaCore.Units.gridUnit * 30
    Layout.maximumHeight: Layout.minimumHeight

    property string previousState
    property Item currentView: mainTabGroup.currentTab.keyNavDown ? mainTabGroup.currentTab : mainTabGroup.currentTab.item
    property Item currentContentView: contentTabGroup.currentItem.keyNavDown ? contentTabGroup.currentItem : contentTabGroup.currentItem.item

    property QtObject globalFavorites: rootModel.favoritesModel
    property QtObject systemFavorites: rootModel.systemFavoritesModel

    onFocusChanged: {
        header.input.forceActiveFocus();
    }

    function switchToInitial() {
        root.state = "Normal";
        tabBar.currentIndex = 0;
        header.query = ""
        keyboardNavigation.state = "LeftColumn"
        navigationMethod.state = "mouse"
    }

    Kicker.DragHelper {
        id: dragHelper

        dragIconSize: PlasmaCore.Units.iconSizes.medium
        onDropped: kickoff.dragSource = null
    }

    Kicker.RootModel {
        id: rootModel

        autoPopulate: false

        appletInterface: plasmoid

        flat: false
        sorted: plasmoid.configuration.alphaSort
        showSeparators: false
        showTopLevelItems: true

        showAllApps: true
        showAllAppsCategorized: false
        showRecentApps: false
        showRecentDocs: false
        showRecentContacts: false
        showPowerSession: false
        showFavoritesPlaceholder: true

        Component.onCompleted: {
            favoritesModel.initForClient("org.kde.plasma.kickoff.favorites.instance-" + plasmoid.id)

            if (!plasmoid.configuration.favoritesPortedToKAstats) {
                favoritesModel.portOldFavorites(plasmoid.configuration.favorites);
                plasmoid.configuration.favoritesPortedToKAstats = true;
            }

            rootModel.refresh();
        }
    }

    onSystemFavoritesChanged: {
        systemFavorites.favorites = String(plasmoid.configuration.systemFavorites).split(',');
    }

    Connections {
        target: plasmoid.configuration

        function onFavoritesChanged() {
            globalFavorites.favorites = plasmoid.configuration.favorites;
        }

        function onSystemFavoritesChanged() {
            systemFavorites.favorites = String(plasmoid.configuration.systemFavorites).split(',');
        }
    }

    Connections {
        target: globalFavorites

        function onFavoritesChanged() {
            plasmoid.configuration.favorites = target.favorites;
        }
    }

    Header {
        id: header
        anchors.left: parent.left
        anchors.right: parent.right
        Component.onCompleted: {
            header.input.forceActiveFocus();
        }
    }

    Item {
        id: mainArea
        anchors.left: parent.left
        anchors.right:parent.right
        anchors.rightMargin: Math.round(parent.width/1.5)
        clip: true
        PlasmaComponents.TabGroup {
            id: mainTabGroup
            currentTab: tabBar.currentIndex == 0 ? applicationsGroupPage : placesPage

            anchors.fill: parent

            //pages
            ApplicationsGroupView {
                id: applicationsGroupPage
            }

            Loader {
                id: placesPage
                active: mainTabGroup.currentTab == placesPage
                source: Qt.resolvedUrl("PlacesView.qml")

                // we want to keep pages always loaded to keep animations nice
                onLoaded: {
                    active = true
                }
            }

            state: {
                switch (plasmoid.location) {
                case PlasmaCore.Types.LeftEdge:
                case PlasmaCore.Types.RightEdge:
                case PlasmaCore.Types.TopEdge:
                    return "top";
                case PlasmaCore.Types.BottomEdge:
                default:
                    return "bottom";
                }
            }
            states: [
                State {
                    name: "top"
                    AnchorChanges {
                        target: header
                        anchors {
                            top: undefined
                            bottom: root.bottom
                        }
                    }
                    PropertyChanges {
                        target: header
                        height: header.implicitHeight
                        location: PlasmaExtras.PlasmoidHeading.Location.Footer
                    }
                    AnchorChanges {
                        target: footer
                        anchors {
                            top: root.top
                            bottom: undefined
                        }
                    }
                    PropertyChanges {
                        target: footer
                        height: footer.implicitHeight
                        location: PlasmaExtras.PlasmoidHeading.Location.Header
                    }
                    AnchorChanges {
                        target: mainArea
                        anchors {
                            top: footer.bottom
                            bottom: header.top
                        }
                    }
                    AnchorChanges {
                        target: contentArea
                        anchors {
                            top: footer.bottom
                            bottom: header.top
                        }
                    }
                    PropertyChanges {
                        target: verticalSeparator
                        anchors {
                            topMargin: footer.topInset
                            bottomMargin: header.bottomInset
                        }
                    }
                    PropertyChanges {
                        target: tabBar
                        anchors {
                            topMargin: footer.topInset
                            bottomMargin: -footer.bottomPadding
                        }
                    }
                },
                State {
                    name: "bottom"
                    AnchorChanges {
                        target: header
                        anchors {
                            top: root.top
                            bottom: undefined
                        }
                    }
                    PropertyChanges {
                        target: header
                        height: header.implicitHeight
                        location: PlasmaExtras.PlasmoidHeading.Location.Header
                    }
                    AnchorChanges {
                        target: footer
                        anchors {
                            top: undefined
                            bottom: root.bottom
                        }
                    }
                    PropertyChanges {
                        target: footer
                        height: footer.implicitHeight
                        location: PlasmaExtras.PlasmoidHeading.Location.Footer
                    }
                    AnchorChanges {
                        target: mainArea
                        anchors {
                            top: header.bottom
                            bottom: footer.top
                        }
                    }
                    AnchorChanges {
                        target: contentArea
                        anchors {
                            top: header.bottom
                            bottom: footer.top
                        }
                    }
                    PropertyChanges {
                        target: verticalSeparator
                        anchors {
                            topMargin: header.topInset
                            bottomMargin: footer.bottomInset
                        }
                    }
                    PropertyChanges {
                        target: tabBar
                        anchors {
                            topMargin: -footer.topPadding
                            bottomMargin: footer.bottomInset
                        }
                    }
                }
            ]
        } // mainTabGroup
    }

    Connections {
        target: applicationsGroupPage
        function onAppModelChange() {
            if (applicationsGroupPage.activatedSection.description == "KICKER_FAVORITES_MODEL") {
                contentTabGroup.isFavorites = true
            } else {
                applicationsPage.activatedSection = applicationsGroupPage.activatedSection
                applicationsPage.rootBreadcrumbName = applicationsGroupPage.newBreadcrumbName
                applicationsPage.appModelChange()
                contentTabGroup.isFavorites = false
            }

        }
    }

    Connections {
        target: placesPage.item
        function onSectionTrigger(index) {
            placesContentPage.currentIndex = index;
        }
    }

    PlasmaCore.SvgItem {
        id: verticalSeparator
        z: 1
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: PlasmaCore.Units.devicePixelRatio
        elementId: "vertical-line"
        anchors.left: parent.left
        anchors.leftMargin: Math.round(parent.width/3)
        svg: PlasmaCore.Svg {
            id: lineSvg;
            imagePath: "widgets/line"
        }
    }

    // we make our model ourselves
    ListModel {
        id: placesViewModel
        signal trigger(int index)

        function getI18nName(index) {
            switch(index) {
                case 0: return i18n("Computer");
                case 1: return i18n("History");
                case 2: return i18n("Frequently Used");
                default: return ""
            }
        }

        ListElement { filename: "ComputerView.qml"; decoration: "computer-laptop"; managesChildrenOutside: true }
        ListElement { filename: "RecentlyUsedView.qml"; decoration: "view-history"; managesChildrenOutside: true }
        ListElement { filename: "FrequentlyUsedView.qml"; decoration: "clock"; managesChildrenOutside: true }
    }

    Item {
        id: contentArea
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.leftMargin: mainArea.opacity == 0 ? 0 : Math.round(parent.width/3)
        clip: true
        PlasmaComponents.TabGroup {
            id: contentTabGroup
            property bool isFavorites: true
            property Item currentItem: currentTab == searchPage ? searchPage : currentTab.currentItem
            currentTab: root.state == "Search" ? searchPage : mainTabGroup.currentTab == applicationsGroupPage ? applicationsContentPage : placesContentPage

            onCurrentItemChanged: {
                if (root.currentContentView) {
                    if (root.currentContentView.gridView) {
                        root.currentContentView.gridView.positionAtBeginning();
                    } else {
                        root.currentContentView.listView.positionAtBeginning();
                    }
                }
            }

            anchors.fill: parent

            //pages
            Item {
                id: applicationsContentPage
                property Item currentItem: contentTabGroup.isFavorites ? (plasmoid.configuration.favoritesDisplay == 0 ? favoritesGridPage : favoritesPage) : applicationsPage
                FavoritesView {
                    id: favoritesPage
                    visible: parent.currentItem == favoritesPage
                    anchors.fill: parent
                }
                FavoritesGridView {
                    id: favoritesGridPage
                    visible: parent.currentItem == favoritesGridPage
                    anchors.fill: parent
                }
                ApplicationsView {
                    id: applicationsPage
                    visible: parent.currentItem == applicationsPage
                    anchors.fill: parent
                }
            }

            // this allows us to track/set index without having to worry about presentation
            Item {
                id: placesContentPage
                property int currentIndex: -1
                property Item currentItem: placesRepeater.itemAt(currentIndex)
                Repeater {
                    id: placesRepeater
                    model: placesViewModel
                    delegate: Loader {
                        // so we don't load immediately
                        active: contentTabGroup.currentTab == placesContentPage && visible
                        visible: placesContentPage.currentIndex == index
                        source: Qt.resolvedUrl(filename)
                        anchors.fill: parent

                        // we don't want to load/unload constantly because it's performance heavy
                        onLoaded: {
                            active = true
                        }
                    }
                    onItemAdded: {
                        if (index == 0) {
                            parent.currentIndex = 0
                        }
                    }
                }
            }

            Loader {
                id: searchPage
                active: root.state == "Search"
                source: Qt.resolvedUrl("SearchView.qml")

                // keeps animations nice
                onLoaded: {
                    active = true
                }
            }
        } // contentTabGroup
    }
    PlasmaExtras.PlasmoidHeading {
        id: footer

        implicitHeight: (footer.opacity == 0) ? 0 : Math.round(PlasmaCore.Units.gridUnit * 2.5)

        anchors.left: parent.left
        anchors.right: parent.right

        Behavior on height {
            NumberAnimation {
                duration: PlasmaCore.Units.longDuration
                easing.type: Easing.InQuad
            }
            enabled: plasmoid.expanded
        }

        PC3.TabBar {
            id: tabBar
            anchors.fill: parent
            anchors.rightMargin: Math.round(parent.width/1.5)
            onCurrentIndexChanged: {
                header.input.forceActiveFocus();
                keyboardNavigation.state = "LeftColumn"
                root.currentView.listView.positionAtBeginning();
            }

            PC3.TabButton {
                id: applicationButton
                implicitHeight: parent.height
                icon.width: PlasmaCore.Units.iconSizes.smallMedium
                icon.height: PlasmaCore.Units.iconSizes.smallMedium
                icon.name: "applications-other"
                text: i18n("Applications")
                hoverEnabled: false
                Keys.onPressed: {
                    // On back tab focus on right pane
                    if (event.key == Qt.Key_Backtab) {
                        if (mainTabGroup.state == "top") {
                            header.configureButton.forceActiveFocus(Qt.TabFocusReason)
                        } else {
                            navigationMethod.state = "keyboard"
                            keyboardNavigation.state = "RightColumn"
                            root.currentContentView.forceActiveFocus()
                        }
                        event.accepted = true;
                        return;
                    }
                }
            }
            PC3.TabButton {
                id: computerButton
                icon.width: PlasmaCore.Units.iconSizes.smallMedium
                icon.height: PlasmaCore.Units.iconSizes.smallMedium
                icon.name: "compass"
                text: i18n("Places") //Explore?
                hoverEnabled: false
            }
            position: {
                switch (plasmoid.location) {
                case PlasmaCore.Types.TopEdge:
                case PlasmaCore.Types.LeftEdge:
                case PlasmaCore.Types.RightEdge:
                    return PC3.TabBar.Header;
                default:
                    return PC3.TabBar.Footer;
                }
            }

            Connections {
                target: plasmoid
                function onExpandedChanged() {
                    header.input.forceActiveFocus();
                    switchToInitial();
                }
            }
        } // tabBar

        LeaveButtons {
            id: leaveButtons
            anchors.fill: parent
            anchors.rightMargin: PlasmaCore.Units.gridUnit
            anchors.leftMargin: Math.round(parent.width/3) + PlasmaCore.Units.gridUnit
        }
    }

    // we need to reverse left and right arrows for RTL support
    function keyBackwardFunction() {
        if (header.input.activeFocus) { return; }
        if (keyboardNavigation.state == "RightColumn") {
            if (root.currentContentView.gridView) {
                if (!root.currentContentView.keyNavLeft()) { //go left if we're on the first column
                    keyboardNavigation.state = "LeftColumn";
                }
                return;
            }
            if (root.currentContentView !== applicationsPage || !root.currentContentView.deactivateCurrentIndex()) {
                if (root.state != "Search") {
                    keyboardNavigation.state = "LeftColumn"
                }
            }
        }
        return;
    }
    function keyForwardFunction() {
        // allow going to right panel immediately even if left panel is not focused
        if (root.currentContentView.gridView) {
            if (keyboardNavigation.state != "RightColumn") {
                keyboardNavigation.state = "RightColumn";
                if (root.currentContentView.gridView.currentIndex === -1) {
                    root.currentContentView.keyNavRight()
                }
            } else if (root.currentContentView.activeFocus) { // only if focused
                root.currentContentView.keyNavRight()
            }
            return;
        }
        // search ignores keyboardNavigation and is always current
        if ((keyboardNavigation.state == "RightColumn" || root.state == "Search") && root.currentContentView.activeFocus) { // only if focused
            currentContentView.activateCurrentIndex();
        } else {
            keyboardNavigation.state = "RightColumn";
        }
        return;
    }
    Keys.onPressed: {
        // switch tabs on CTRL+Tab (and CTRL+Shift+Tab)
        if ((event.key == Qt.Key_Tab || event.key == Qt.Key_Backtab) && (event.modifiers & Qt.ControlModifier)) {
            // only 2 tabs so no need to handle Shift
            tabBar.currentIndex = !tabBar.currentIndex
            event.accepted = true;
            return;
        }

        // handle tab navigation for main columns
        if (event.key == Qt.Key_Tab) {
            navigationMethod.state = "keyboard"
            if (root.currentView.activeFocus) {
                keyboardNavigation.state = "RightColumn"
                event.accepted = true;
                return;
            } else if (root.currentContentView.activeFocus) {
                // There's no footer when we search
                if (root.state == "Search" || mainTabGroup.state == "top") {
                    header.avatar.forceActiveFocus(Qt.TabFocusReason)
                } else {
                    applicationButton.forceActiveFocus(Qt.TabFocusReason)
                }
                event.accepted = true;
                return;
            }
        }
        // and backtab navigation
        if (event.key == Qt.Key_Backtab) {
            navigationMethod.state = "keyboard"
            if (root.currentContentView.activeFocus) {
                // There's no left panel when we search
                if (root.state == "Search") {
                    header.configureButton.forceActiveFocus(Qt.BacktabFocusReason)
                } else {
                    keyboardNavigation.state = "LeftColumn"
                }
                event.accepted = true;
                return;
            } else if (root.currentView.activeFocus) {
                if (mainTabGroup.state == "top" && root.state != "Search") {
                    leaveButtons.leave.forceActiveFocus(Qt.TabFocusReason)
                } else {
                    header.configureButton.forceActiveFocus(Qt.BacktabFocusReason)
                }
                event.accepted = true;
                return;
            }
        }
        var headerUpFooterDown = (event.key == Qt.Key_Down && footer.activeFocus) || (event.key == Qt.Key_Up && header.activeFocus)
        var headerDownFooterUp = (event.key == Qt.Key_Down && header.activeFocus) || (event.key == Qt.Key_Up && footer.activeFocus)
        // Don't react on down presses with active footer or up presses with active header (this is inverse when upside down)
        if ((mainTabGroup.state == "bottom" && headerUpFooterDown) || (mainTabGroup.state == "top" && headerDownFooterUp)) {
            return;
        }
        if (event.key != Qt.Key_Shift) {
            navigationMethod.state = "keyboard"
            // Focus on content when pressing down and up from header and footer respectively (this is inverse when upside down)
            // Note however that we don't filter left and right keys. We still want to move between columns even without focus
            // Instead we block moving (grid) and submenus (list) and *only* change focus
            if (((mainTabGroup.state == "bottom" && headerDownFooterUp) || (mainTabGroup.state == "top" && headerUpFooterDown)) && !root.currentView.activeFocus && !root.currentContentView.activeFocus) {
                if (root.state == "Search") {
                    keyboardNavigation.state = "RightColumn"
                }
                if (keyboardNavigation.state == "LeftColumn") {
                    root.currentView.forceActiveFocus()
                } else {
                    root.currentContentView.forceActiveFocus()
                }

                return;
            }
        }

        switch(event.key) {
            case Qt.Key_Up: {
                // Focus on header when reaching the beginning of the list/grid
                if (keyboardNavigation.state == "LeftColumn" && root.state != "Search") {
                    if (!root.currentView.keyNavUp()) {
                        if (mainTabGroup.state == "top") {
                            tabBar.currentItem.forceActiveFocus(Qt.TabFocusReason);
                        } else {
                            header.forceActiveFocus();
                        }
                    }
                } else {
                    if (!root.currentContentView.keyNavUp()) {
                        if (mainTabGroup.state == "top") {
                            if (root.state != "Search") {
                                tabBar.currentItem.forceActiveFocus(Qt.TabFocusReason);
                            }
                        } else {
                            header.forceActiveFocus();
                        }
                    }
                }
                event.accepted = true;
                break;
            }
            case Qt.Key_Down: {
                // Focus on footer when reaching the end of the list/grid
                if (keyboardNavigation.state == "LeftColumn" && root.state != "Search") {
                    if (!root.currentView.keyNavDown()) {
                        if (mainTabGroup.state == "top") {
                            header.forceActiveFocus();
                        } else {
                            // forces activeFocus visuals
                            tabBar.currentItem.forceActiveFocus(Qt.TabFocusReason);
                        }
                    }
                } else {
                    if (!root.currentContentView.keyNavDown()) {
                        if (mainTabGroup.state == "top") {
                            header.forceActiveFocus();
                        } else if (root.state != "Search") {
                            tabBar.currentItem.forceActiveFocus(Qt.TabFocusReason);
                        }
                    }
                }
                event.accepted = true;
                break;
            }
            case Qt.Key_Left: {
                if (!LayoutMirroring.enabled) {
                    keyBackwardFunction()
                } else {
                    keyForwardFunction()
                }
                event.accepted = true;
                break;
            }
            case Qt.Key_Right: {
                if (!LayoutMirroring.enabled) {
                    keyForwardFunction()
                } else {
                    keyBackwardFunction()
                }
                event.accepted = true;
                break;
            }
            case Qt.Key_Escape: {
                if (header.query.length == 0) {
                    plasmoid.expanded = false;
                } else {
                    header.query = "";
                }
                event.accepted = true;
                break;
            }
            case Qt.Key_Enter:
            case Qt.Key_Return: {
                if (keyboardNavigation.state == "LeftColumn" && root.state != "Search") {
                    currentView.activateCurrentIndex();
                    keyboardNavigation.state = "RightColumn";
                } else {
                    currentContentView.activateCurrentIndex();
                }
                event.accepted = true;
                break;
            }
            case Qt.Key_Menu: {
                if (keyboardNavigation.state == "RightColumn" || root.state == "Search") {
                    currentContentView.openContextMenu();
                } else if (root.currentView == applicationsGroupPage && applicationsGroupPage.listView.currentIndex != -1) {
                    if (!applicationsGroupPage.listView.currentItem.modelChildren) {
                        currentView.openContextMenu();
                    }
                }
                event.accepted = true;
                break;
            }
            default: {
                // Relay first key press to search field, make sure it's a proper character
                if (event.text != "" && !header.input.activeFocus && event.key != Qt.Key_Backspace && event.key != Qt.Key_Backtab && event.key != Qt.Key_Tab) {
                    // Works for both LTR and RTL
                    header.input.insert(header.input.length, event.text.charAt(0))
                    header.input.forceActiveFocus()
                }
                // Relay backspace
                if (!header.input.activeFocus && event.key == Qt.Key_Backspace && header.input.length != 0) {
                    header.input.remove(header.input.length - 1, header.input.length)
                    header.input.forceActiveFocus()
                }
            }
        }
    }
    state: "Normal"
    states: [
        State {
            name: "Normal"
            PropertyChanges {
                target: footer
                //Set the opacity and NOT the visibility, as visibility is recursive
                //and this binding would be executed also on popup show/hide
                //as recommended by the docs: https://doc.qt.io/qt-5/qml-qtquick-item.html#visible-prop
                //plus, it triggers https://bugreports.qt.io/browse/QTBUG-66907
                //in which a mousearea may think it's under the mouse while it isn't
                opacity: 1
            }
            PropertyChanges {
                target: mainArea
                opacity: 1
            }
            PropertyChanges {
                target: verticalSeparator
                visible: true
            }
        },
        State {
            name: "Search"
            PropertyChanges {
                target: footer
                opacity: 0
            }
            PropertyChanges {
                target: mainArea
                opacity: 0
            }
            PropertyChanges {
                target: verticalSeparator
                visible: false
            }
        }
    ] // states
    Item {
        id: keyboardNavigation
        state: "LeftColumn"
        states: [
            State {
                name: "LeftColumn"
            },
            State {
                name: "RightColumn"
            }
        ]
        onStateChanged: {
            if (state == "LeftColumn") {
                root.currentView.forceActiveFocus()
            } else if (root.state != "search") {
                root.currentContentView.forceActiveFocus()
            }
        }
    }
    onCurrentViewChanged: {
        if (keyboardNavigation.state == "LeftColumn") {
            root.currentView.forceActiveFocus()
        }
    }
    onCurrentContentViewChanged: {
        if (keyboardNavigation.state == "RightColumn" && root.currentContentView != searchPage.item) {
            root.currentContentView.forceActiveFocus()
        }
    }
    Item {
        id: navigationMethod
        property bool inSearch: root.state == "Search"
        state: "mouse"
        states: [
            State {
                name: "mouse"
            },
            State {
                name: "keyboard"
            }
        ]
    }
}
