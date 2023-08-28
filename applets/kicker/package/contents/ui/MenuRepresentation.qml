/*
    SPDX-FileCopyrightText: 2013-2015 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.ksvg 1.0 as KSvg
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.kirigami 2 as Kirigami
import org.kde.plasma.plasmoid 2.0

FocusScope {
    id: root

    Layout.minimumWidth: sideBar.width
        + (sideBar.visible ? mainRow.anchors.leftMargin : 0) + Math.max(searchField.defaultWidth, runnerColumns.visible ? runnerColumns.implicitWidth : 0)
    Layout.maximumWidth: Math.max(rootList.minimumWidth, Layout.minimumWidth)

    Layout.minimumHeight: Math.max(
        ((rootModel.count - rootModel.separatorCount) * rootList.itemHeight) + (rootModel.separatorCount * rootList.separatorHeight) + searchField.height + 2 * Kirigami.Units.smallSpacing,
        sideBar.margins.top + sideBar.margins.bottom + favoriteApps.contentHeight + favoriteSystemActions.contentHeight + sidebarSeparator.height + 4 * Kirigami.Units.smallSpacing
    )
    Layout.maximumHeight: Layout.minimumHeight

    readonly property alias searchField: searchField // Used to forward key input

    Keys.forwardTo: searchField
    Keys.onEscapePressed: {
        kicker.expanded = false;
    }

    Connections {
        target: kicker
        function onExpandedChanged(expanded) {
            if (!expanded) {
                searchField.text = "";
            }
        }
    }

    KSvg.FrameSvgItem {
        id: sideBar

        visible: width > 0

        anchors.top: parent.top
        anchors.left: parent.left
        width: (globalFavorites && systemFavorites
            && (globalFavorites.count + systemFavorites.count)
            ? Kirigami.Units.iconSizes.medium + margins.left + margins.right : 0)
        height: parent.height
        focus: true

        imagePath: "widgets/frame"
        prefix: "plain"

        SideBarSection {
            id: favoriteApps

            anchors.top: Plasmoid.location === PlasmaCore.Types.TopEdge ? sidebarSeparator.bottom : parent.top
            anchors.topMargin: Plasmoid.location === PlasmaCore.Types.TopEdge ? sideBar.margins.bottom : 0

            height: (sideBar.height - sideBar.margins.top - sideBar.margins.bottom
                - favoriteSystemActions.height - sidebarSeparator.height - (4 * Kirigami.Units.smallSpacing))

            model: globalFavorites

            KeyNavigation.up: Plasmoid.location === PlasmaCore.Types.TopEdge ? favoriteSystemActions : null
            KeyNavigation.down: Plasmoid.location === PlasmaCore.Types.TopEdge ? null : favoriteSystemActions
            KeyNavigation.right: searchField

            Binding {
                target: globalFavorites
                property: "iconSize"
                value: Kirigami.Units.iconSizes.medium
            }
        }

        KSvg.SvgItem {
            id: sidebarSeparator

            anchors.top: Plasmoid.location === PlasmaCore.Types.TopEdge ? favoriteSystemActions.bottom : undefined
            anchors.topMargin: Plasmoid.location === PlasmaCore.Types.TopEdge ? sideBar.margins.top : 0
            anchors.bottom: Plasmoid.location === PlasmaCore.Types.TopEdge ? undefined : favoriteSystemActions.top
            anchors.bottomMargin: Plasmoid.location === PlasmaCore.Types.TopEdge ? 0 : sideBar.margins.bottom
            anchors.horizontalCenter: parent.horizontalCenter

            width: Kirigami.Units.iconSizes.medium
            height: lineSvg.horLineHeight

            visible: (favoriteApps.model && favoriteApps.model.count
                && favoriteSystemActions.model && favoriteSystemActions.model.count)
            svg: lineSvg
            elementId: "horizontal-line"
        }

        SideBarSection {
            id: favoriteSystemActions

            anchors.top: Plasmoid.location === PlasmaCore.Types.TopEdge ? parent.top : undefined
            anchors.bottom: Plasmoid.location === PlasmaCore.Types.TopEdge ? undefined : parent.bottom

            model: systemFavorites

            KeyNavigation.up: Plasmoid.location === PlasmaCore.Types.TopEdge ? null : favoriteApps
            KeyNavigation.down: Plasmoid.location === PlasmaCore.Types.TopEdge ? favoriteApps : null
            KeyNavigation.right: favoriteApps.KeyNavigation.right
        }
    }

    Item {
        id: mainRow

        anchors {
            top: parent.top
            bottom: searchField.top
            left: sideBar.right
            leftMargin: sideBar.visible ? Kirigami.Units.smallSpacing : 0
            right: parent.right
        }

        Row {
            id: runnerColumns

            anchors {
                top: parent.top
                left: parent.left
                bottom: parent.bottom
            }
            visible: searchField.text !== "" && runnerModel.count > 0

            readonly property list<Item> visibleChildren: children
                .filter(child => child instanceof RunnerResultsList && child.visible)

            Repeater {
                id: runnerColumnsRepeater

                model: runnerModel

                delegate: RunnerResultsList {
                    id: runnerMatches
                    height: runnerColumns.height
                    visible: runnerModel.modelForRow(index).count > 0

                    KeyNavigation.up: searchField
                    KeyNavigation.down: searchField
                    // ScrollView accepts Left/Right key events, so can't use KeyNavigation here
                    Keys.onLeftPressed: event => {
                        let leftIndex = index - 1;
                        while (leftIndex >= 0 && !runnerColumnsRepeater.itemAt(leftIndex).count) {
                            leftIndex -= 1;
                        }
                        if (leftIndex >= 0) {
                            currentIndex = -1;
                            runnerColumnsRepeater.itemAt(leftIndex).listView.itemAtIndex(0).forceActiveFocus(Qt.TabFocusReason);
                        } else {
                            searchField.KeyNavigation.left.forceActiveFocus(Qt.TabFocusReason); // Focus on the first sidebar item
                        }
                    }
                    Keys.onRightPressed: event => {
                        let rightIndex = index + 1;
                        while (rightIndex <= runnerColumnsRepeater.count - 1 && !runnerColumnsRepeater.itemAt(rightIndex).count) {
                            rightIndex += 1;
                        }
                        if (rightIndex <= runnerColumnsRepeater.count - 1) {
                            currentIndex = -1;
                            runnerColumnsRepeater.itemAt(rightIndex).listView.itemAtIndex(0).forceActiveFocus(Qt.TabFocusReason);
                        }
                    }
                }
            }
        }

        ItemListView {
            id: rootList
            anchors {
                top: parent.top
                left: parent.left
            }
            minimumWidth: root.Layout.minimumWidth - sideBar.width - mainRow.anchors.leftMargin
            height: ((rootModel.count - rootModel.separatorCount) * itemHeight) + (rootModel.separatorCount * separatorHeight)
            visible: searchField.text.length === 0

            focus: !runnerColumns.visible && currentIndex !== -1 // Avoid stealing focus from runnerColumns and searchField.
            model: rootModel
            iconsEnabled: Plasmoid.configuration.showIconsRootLevel

            KeyNavigation.left: favoriteApps.repeater.count > 0 ? favoriteApps : favoriteSystemActions
        }
    }

    PlasmaExtras.SearchField {
        id: searchField

        anchors {
            bottom: parent.bottom
            left: mainRow.anchors.left
            leftMargin: mainRow.anchors.leftMargin
        }

        readonly property real defaultWidth: Kirigami.Units.gridUnit * 14
        width: (runnerColumnsRepeater.count !== 0 ? runnerColumnsRepeater.itemAt(0).width
                                                : (rootList.visible ? rootList.width : defaultWidth))

        focus: !Kirigami.InputMethod.willShowOnActive && kicker.expanded

        KeyNavigation.up: runnerColumns.visible ? runnerColumns.visibleChildren[0] : rootList
        KeyNavigation.down: runnerColumns.visible ? runnerColumns.visibleChildren[0] : rootList
        KeyNavigation.left: favoriteApps.repeater.count > 0 ? favoriteApps : favoriteSystemActions
        KeyNavigation.right: runnerColumns.visibleChildren.length > 1 ? runnerColumns.visibleChildren[1] : null

        onTextChanged: {
            if (text.length === 0 && rootList.currentIndex === -1) {
                forceActiveFocus(Qt.TabFocusReason); // When rootList has no selected item, up/down key doesn't work.
            }
            runnerModel.query = text;
        }

        onFocusChanged: {
            if (focus) {
                // FIXME: Cleanup arbitration between rootList/runnerCols here and in Keys.
                if (rootList.visible) {
                    rootList.currentIndex = -1;
                }

                if (runnerColumns.visible) {
                    for (let i = 0; i < runnerColumnsRepeater.count; i++) {
                        runnerColumnsRepeater.itemAt(i).currentIndex = -1;
                    }
                }
            }
        }

        Keys.onUpPressed: event => {
            if (runnerColumns.visible) {
                runnerColumns.visibleChildren[0].currentIndex = runnerColumns.visibleChildren[0].count - 1;
            } else {
                // Create child dialogs only after a key is pressed
                rootList.showChildDialogs = false;
                rootList.currentIndex = rootList.count - 1;
            }
            event.accepted = false; // Pass the event to KeyNavigation.up
        }
        Keys.onDownPressed: event => {
            if (runnerColumns.visibleChildren[0]) {
                runnerColumns.visibleChildren[0].currentIndex = 0;
            } else {
                // Create child dialogs only after a key is pressed
                rootList.showChildDialogs = false;
                rootList.currentIndex = 0;
            }
            event.accepted = false; // Pass the event to KeyNavigation.down
        }
        Keys.onRightPressed: event => {
            if (runnerColumns.visibleChildren.length > 1) {
                runnerColumns.visibleChildren[1].currentIndex = 0;
            }
            event.accepted = false; // Pass the event to KeyNavigation.right
        }

        Keys.onReturnPressed: event => {
            if (runnerColumns.visible) {
                const listView = runnerColumns.visibleChildren[0].listView;
                (listView.currentItem || listView.itemAtIndex(0)).Keys.onReturnPressed(event);
            } else {
                event.accepted = false;
            }
        }
        Keys.onEnterPressed: event => Keys.onReturnPressed(event)
    }

    Component.onCompleted: {
        rootModel.refresh();
    }
}
