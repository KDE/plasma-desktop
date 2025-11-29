/*
    SPDX-FileCopyrightText: 2013-2015 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.ksvg as KSvg
import org.kde.plasma.components as PlasmaComponents3
import org.kde.plasma.core as PlasmaCore
import org.kde.plasma.extras as PlasmaExtras
import org.kde.plasma.plasmoid

PlasmaComponents3.ScrollView {
    id: root

    focus: true

    Layout.minimumWidth: Math.min(mainRow.width, Screen.width - Kirigami.Units.largeSpacing * 4)
    Layout.maximumWidth: Layout.minimumWidth

    contentWidth: mainRow.width

    Layout.minimumHeight: Math.max(((rootModel.count - rootModel.separatorCount) * rootList.itemHeight)
        + (rootModel.separatorCount * rootList.separatorHeight)
        + searchField.height + (2 * Kirigami.Units.smallSpacing), sideBar.margins.top + sideBar.margins.bottom
        + favoriteApps.contentHeight + favoriteSystemActions.contentHeight + sidebarSeparator.height
        + (4 * Kirigami.Units.smallSpacing))
    Layout.maximumHeight: Layout.minimumHeight

    function ensureVisible(item: Item) : void {
        var actualItemX = item.x
        var viewXPosition = (item.width <= contentItem.width)
        ? Math.round(actualItemX + item.width / 2 - contentItem.width / 2)
        : actualItemX
        if (actualItemX < contentItem.contentX) {
            contentItem.contentX = Math.max(0, viewXPosition)
        } else if ((actualItemX + item.width) > (contentItem.contentX + contentItem.width)) {
            contentItem.contentX = Math.min(contentItem.contentWidth - contentItem.width, viewXPosition)
        }
        contentItem.returnToBounds()
    }

    function focusRunnerColumn(column, focusTopElement = true) : void {
        if (runnerColumns.visibleChildren.length <= column + 1) { // visibleChildren includes repeater
            return
        }
        const targetList = runnerColumns.visibleChildren[column]
        targetList.currentIndex = focusTopElement ? 0 : (targetList.count - 1)
        targetList.forceActiveFocus(Qt.TabFocusReason)
        root.ensureVisible(targetList)
    }

    function focusRootList(focusTopElement = true) : void {
        rootList.currentIndex = focusTopElement ? 0 : (rootList.model.count - 1)
        rootList.forceActiveFocus(Qt.TabFocusReason)
    }

    function focusSideBar() : void {
        sideBar.forceActiveFocus(Qt.TabFocusReason)
        root.ensureVisible(sideBar)
    }


    function reset() {
        kicker.hideOnWindowDeactivate = true;

        rootList.currentIndex = -1;
        hoverBlock.reset();

        searchField.text = "";
        searchField.focus = true;
    }

    Row {
        id: mainRow

        height: parent.height
        width: sideBar.width + (sideBar.width ? mainRow.spacing : 0) +
            Math.max((runnerColumns.visible ? runnerColumns.width : rootList.width), searchField.defaultWidth)

        spacing: Kirigami.Units.smallSpacing

        LayoutMirroring.enabled: ((Plasmoid.location === PlasmaCore.Types.RightEdge)
            || (Qt.application.layoutDirection === Qt.RightToLeft && Plasmoid.location !== PlasmaCore.Types.LeftEdge))

        KSvg.FrameSvgItem {
            id: sideBar

            property bool onTopPanel: Plasmoid.location === PlasmaCore.Types.TopEdge

            visible: width > 0

            width: (globalFavorites && systemFavorites
                && (globalFavorites.count + systemFavorites.count)
                ? Math.max(favoriteApps.implicitWidth, favoriteSystemActions.implicitWidth) + margins.left + margins.right : 0)
            height: parent.height

            imagePath: "widgets/frame"
            prefix: "plain"

            activeFocusOnTab: true
            onActiveFocusChanged: if (activeFocus) {
                let target = (onTopPanel && favoriteSystemActions.model.count) ? favoriteSystemActions : favoriteApps
                target.forceActiveFocus(Qt.TabFocusReason)
            }
            KeyNavigation.right: rootList
            Keys.onPressed: event => {
                let backArrowKey = (event.key === Qt.Key_Left && !mainRow.LayoutMirroring.enabled) ||
                    (event.key === Qt.Key_Right && mainRow.LayoutMirroring.enabled)
                let forwardArrowKey = (event.key === Qt.Key_Right && !mainRow.LayoutMirroring.enabled) ||
                    (event.key === Qt.Key_Left && mainRow.LayoutMirroring.enabled)

                if (backArrowKey & runnerColumns.visibleChildren.length > 1) {
                    root.focusRunnerColumn(runnerColumns.visibleChildren.length - 2, true)
                    event.accepted = true
                }
                if (forwardArrowKey) {
                    if (runnerColumns.visibleChildren.length > 1) {
                        root.focusRunnerColumn(0, true)
                    } else if (rootList.visible) {
                        root.focusRootList(true)
                    } else {
                        searchField.forceActiveFocus(Qt.TabFocusReason)
                    }
                    event.accepted = true
                }
            }

            ColumnLayout {
                anchors.top: parent.top
                anchors.horizontalCenter: parent.horizontalCenter
                height: Math.max(implicitHeight, parent.height)
                width: parent.width

                LayoutItemProxy {
                    target: sideBar.onTopPanel ? favoriteSystemActions : favoriteApps
                    Layout.topMargin: sideBar.margins.top
                }
                KSvg.SvgItem {
                    id: sidebarSeparator

                    Layout.fillWidth: true
                    Layout.leftMargin: Kirigami.Units.smallSpacing
                    Layout.rightMargin: Layout.leftMargin
                    Layout.alignment: Qt.AlignHCenter

                    visible: (favoriteApps.model && favoriteApps.model.count
                        && favoriteSystemActions.model && favoriteSystemActions.model.count)

                    imagePath: "widgets/line"
                    elementId: "horizontal-line"
                }
                LayoutItemProxy {
                    target: sideBar.onTopPanel ? favoriteApps : favoriteSystemActions
                    Layout.topMargin: sideBar.margins.bottom
                }

                SideBarSection {
                    id: favoriteApps

                    Layout.fillHeight: true
                    Layout.alignment: Qt.AlignHCenter

                    KeyNavigation.up: favoriteSystemActions.bottomSideBarItem
                    KeyNavigation.down: favoriteSystemActions

                    model: globalFavorites

                    Binding {
                        target: globalFavorites
                        property: "iconSize"
                        value: Kirigami.Units.iconSizes.medium
                        restoreMode: Binding.RestoreBinding
                    }
                }

                SideBarSection {
                    id: favoriteSystemActions

                    Layout.alignment: Qt.AlignHCenter

                    model: systemFavorites
                    KeyNavigation.up: favoriteApps.bottomSideBarItem
                    KeyNavigation.down: favoriteApps
                }
            }
        }

        ItemListView {
            id: rootList

            anchors.top: parent.top

            minimumWidth: searchField.defaultWidth
            height: ((rootModel.count - rootModel.separatorCount) * itemHeight) + (rootModel.separatorCount * separatorHeight)

            visible: searchField.text === ""

            iconsEnabled: Plasmoid.configuration.showIconsRootLevel

            hoverEnabled: !hoverBlock.enabled

            mainSearchField: searchField

            model: rootModel

            LayoutMirroring.enabled: mainRow.LayoutMirroring.enabled

            showSeparators: true // keep even if sorted, the one between recents and categories works

            onKeyNavigationAtListEnd: {
                searchField.focus = true;
            }

            onNavigateLeftRequested: {
                if (!sideBar.visible) {
                    return
                }
                currentIndex = -1
                root.focusSideBar()
            }

            states: State {
                name: "top"
                when: Plasmoid.location === PlasmaCore.Types.TopEdge

                AnchorChanges {
                    target: rootList
                    anchors.top: parent.top
                }
            }

            Component.onCompleted: {
                rootList.exited.connect(root.reset);
            }
        }

        Row {
            id: runnerColumns

            height: parent.height

            visible: searchField.text !== "" && runnerModel.count > 0

            spacing: Kirigami.Units.smallSpacing

            LayoutMirroring.enabled: mainRow.LayoutMirroring.enabled

            Repeater {
                id: runnerColumnsRepeater

                model: runnerModel

                delegate: RunnerResultsList {
                    id: runnerMatches

                    visible: runnerModel.modelForRow(index).count > 0

                    mainSearchField: searchField

                    LayoutMirroring.enabled: runnerColumns.LayoutMirroring.enabled

                    function navigateToAdjacentColumn(forward = true) {
                        let currentColumnIndex = runnerColumns.visibleChildren.indexOf(runnerMatches)
                        let targetColumnIndex = currentColumnIndex + (forward ? 1 : -1)
                        if (currentColumnIndex < 0 || runnerColumns.visibleChildren.length <= 2) {
                            return
                        }
                        currentIndex = -1
                        if (targetColumnIndex >= 0 && targetColumnIndex < (runnerColumns.visibleChildren.length - 1)) {
                            root.focusRunnerColumn(targetColumnIndex)
                        } else if (sideBar.visible) {
                            root.focusSideBar()
                        } else {
                            root.focusRunnerColumn(forward ? 0 : runnerColumns.visibleChildren.length - 2)
                        }
                    }

                    onContainsMouseChanged: {
                        if (containsMouse) {
                            focus = true;
                        }
                    }

                    onFocusChanged: {
                        if (!focus) {
                            currentIndex = -1;
                        }
                    }

                    onNavigateLeftRequested: navigateToAdjacentColumn(false)
                    onNavigateRightRequested: navigateToAdjacentColumn(true)
                }
            }
        }

        Item {
            height: parent.height
            width: searchField.defaultWidth

            PlasmaExtras.PlaceholderMessage {
                id: noMatchesPlaceholder

                property bool searchRunning: false
                property string lastQuery: "" // copy to avoid timing conflicts with visible binding

                anchors.centerIn: parent
                visible: lastQuery !== "" && runnerColumns.width < 1 && (!searchRunning || visible)

                iconName: "edit-none"
                text: i18nc("@info:status", "No matches")

                Connections {
                    target: runnerModel

                    function onQueryFinished() {
                        noMatchesPlaceholder.searchRunning = false
                    }
                }

                Connections {
                    target: searchField

                    function onTextChanged() {
                        noMatchesPlaceholder.searchRunning = searchField.text !== ""
                        noMatchesPlaceholder.lastQuery = searchField.text
                    }
                }
            }
        }
    }

    PlasmaExtras.SearchField {
        id: searchField

        readonly property int spacing: sideBar.width ? mainRow.spacing : 0

        anchors.bottom: mainRow.bottom
        anchors.left: parent.left
        anchors.leftMargin: sideBar.width + spacing

        readonly property real defaultWidth: Kirigami.Units.gridUnit * 14

        width: runnerColumns.visibleChildren.length > 1
            ? runnerColumns.visibleChildren[0].width
            : (rootList.visible ? rootList.width : defaultWidth)

        focus: !Kirigami.InputMethod.willShowOnActive

        onTextChanged: {
            runnerModel.query = text;
        }

        onFocusChanged: {
            if (focus) {
                // FIXME: Cleanup arbitration between rootList/runnerCols here and in Keys.
                if (rootList.visible) {
                    rootList.currentIndex = -1;
                }

                if (runnerColumns.visible) {
                    runnerColumnsRepeater.itemAt(0).currentIndex = -1;
                }
            }
        }

        states: [
            State {
                name: "right"
                when: (Plasmoid.location === PlasmaCore.Types.RightEdge && Qt.application.layoutDirection === Qt.LeftToRight)
                    || (Plasmoid.location === PlasmaCore.Types.LeftEdge && Qt.application.layoutDirection === Qt.RightToLeft)

                AnchorChanges {
                    target: searchField
                    anchors.top: undefined
                    anchors.bottom: mainRow.bottom
                    anchors.left: undefined
                    anchors.right: parent.right
                }

                PropertyChanges {
                    target: searchField
                    anchors.leftMargin: undefined
                    anchors.rightMargin: sideBar.width + spacing
                }
            }
        ]

        Keys.priority: Keys.AfterItem // arrow keys should move cursor first
        Keys.onPressed: event => {
            let backArrowKey = (event.key === Qt.Key_Left && !mainRow.LayoutMirroring.enabled) ||
                (event.key === Qt.Key_Right && mainRow.LayoutMirroring.enabled)
            let forwardArrowKey = (event.key === Qt.Key_Right && !mainRow.LayoutMirroring.enabled) ||
                (event.key === Qt.Key_Left && mainRow.LayoutMirroring.enabled)
            if (event.key === Qt.Key_Up || event.key === Qt.Key_Down) {
                if (rootList.visible) {
                    root.focusRootList(event.key === Qt.Key_Down);
                } else if (runnerColumns.visible) {
                    let index = runnerColumns.visibleChildren[0].currentIndex
                    root.focusRunnerColumn(0, event.key === Qt.Key_Down)
                    // First column, first entry is initially selected even when focus is on the search
                    // field, as Return will activate it. Down should immediately go to the second entry.
                    if (index === 0 && event.key === Qt.Key_Down) {
                        runnerColumns.visibleChildren[0].currentIndex = Math.min(index + 1, runnerColumns.visibleChildren[0].count - 1)
                    }
                }
                event.accepted = true;
            } else if (backArrowKey) {
                if (!sideBar.visible && !runnerColumns.visible) {
                    return;
                }
                if (runnerColumns.visibleChildren[0]?.length > 1) {
                    runnerColumns.visibleChildren[0].currentIndex = -1;
                }
                if (sideBar.visible) {
                    root.focusSideBar(true)
                    event.accepted = true;
                } else {
                    root.focusRunnerColumn(runnerColumns.visibleChildren.length - 2, true)
                }

            } else if (forwardArrowKey && runnerColumns.visibleChildren.length > 2) {
                runnerColumns.visibleChildren[0].currentIndex = -1;
                root.focusRunnerColumn(1, true)
                event.accepted = true;
            } else if (event.key === Qt.Key_Enter || event.key === Qt.Key_Return) {
                if (runnerColumns.visible) {
                    for (let i = 0; i < runnerModel.count; ++i) {
                        if (runnerModel.modelForRow(i).count) {
                            runnerModel.modelForRow(i).trigger(0, "", null);
                            kicker.expanded = false;
                            break;
                        }
                    }
                }
                event.accepted = true;
            }
        }

        function appendText(newText) {
            focus = true;
            text = text + newText;
        }
    }

    Keys.onPressed: event => {
        if (event.key === Qt.Key_Escape) {
            kicker.expanded = false;
        }
    }

    Component.onCompleted: {
        kicker.modelRefreshed.connect(() => {
            const searchTerm = searchField.text
            root.reset()
            runnerModel.clear() // runnerModel is on a timer, clearing and setting query is too fast to work
            searchField.text = searchTerm
        });
        kicker.reset.connect(reset)
        windowSystem.hidden.connect(reset);

        rootModel.refresh();
    }

    MouseArea {
        id: hoverBlock  // don't hover-activate until mouse is moved to not interfere with keyboard use
        anchors.fill: parent
        hoverEnabled: true
        propagateComposedEvents: true // clicking should still work if hovering is blocked

        property bool mouseMoved: false

        function reset() {
            mouseMoved = false
            enabled = true
        }

        onPositionChanged: if (!mouseMoved) {
            mouseMoved = true
        } else {
            enabled = false // this immediately triggers other hover events when bound to their hoverEnabled
        }

    }
}
