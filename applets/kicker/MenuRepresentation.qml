/*
    SPDX-FileCopyrightText: 2013-2015 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.ksvg as KSvg
import org.kde.plasma.components as PlasmaComponents3
import org.kde.plasma.core as PlasmaCore
import org.kde.plasma.extras as PlasmaExtras
import org.kde.plasma.plasmoid
import org.kde.plasma.private.kicker as Kicker

PlasmaComponents3.ScrollView {
    id: root

    required property Kicker.KAStatsFavoritesModel globalFavorites
    required property Kicker.SimpleFavoritesModel systemFavorites
    required property Kicker.RootModel rootModel
    required property Kicker.RunnerModel runnerModel

    signal interactionConcluded

    focus: true

    // we never want a vertical scrollbar, the components handle those.
    PlasmaComponents3.ScrollBar.vertical.policy: PlasmaComponents3.ScrollBar.AlwaysOff
    // needs to be set explicitly as the default can cause loops
    PlasmaComponents3.ScrollBar.horizontal.visible: Screen.width > 0 && contentWidth > (Screen.width - Kirigami.Units.largeSpacing * 4)

    // ScrollBar's padding is only to make space for the scrollbar, so we don't need it here
    Layout.minimumWidth: Math.min(Math.max(mainRow.Layout.minimumWidth, mainRow.implicitWidth), Screen.width - Kirigami.Units.largeSpacing * 4)
    Layout.maximumWidth: Layout.minimumWidth

    contentWidth: mainRow.implicitWidth

    Layout.minimumHeight: Math.min(Math.max(sideBar.implicitHeight, rootList.implicitHeight + rootList.Layout.bottomMargin), Math.round(Screen.height * 0.9)) + topPadding + bottomPadding
    Layout.maximumHeight: Layout.minimumHeight

    function ensureVisible(item: Item) : void {
        const actualItemX = item.x
        const viewXPosition = (item.width <= contentItem.width)
            ? Math.round(actualItemX + item.width / 2 - contentItem.width / 2)
            : actualItemX
        const flickable = contentItem as Flickable
        if (actualItemX < flickable.contentX) {
            flickable.contentX = Math.max(0, viewXPosition)
        } else if ((actualItemX + item.width) > (flickable.contentX + flickable.width)) {
            flickable.contentX = Math.min(flickable.contentWidth - flickable.width, viewXPosition)
        }
        flickable.returnToBounds()
    }

    function focusRunnerColumn(column, focusTopElement = true) : void {
        if (runnerColumns.visibleChildren.length <= column + 1) { // visibleChildren includes repeater
            return
        }
        const targetList = runnerColumns.visibleChildren[column]
        targetList.currentIndex = focusTopElement ? 0 : (targetList.count - 1)
        targetList.giveFocus(Qt.TabFocusReason)
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
        rootList.currentIndex = -1;
        hoverBlock.reset();

        searchField.text = "";
        searchField.focus = true;
    }

    DropArea {
        id: unfavoriteDropArea
        anchors.fill: parent
        keys: ["favoritedrag"]
        onDropped: drop => {
            let draggedItem = drag.source as SideBarItem
            if (draggedItem && draggedItem.favoritesModel.isFavorite(draggedItem.favoriteId)) {
                draggedItem.showUnfavoritePlaceholder = true
                draggedItem.favoritesModel.removeFavorite(draggedItem.favoriteId)
                drop.accept(Qt.MoveAction)
            }
        }
    }

    RowLayout {
        id: mainRow

        anchors.fill: parent

        spacing: 0

        readonly property int minimumMainWidth: Math.max(searchField.defaultWidth, runnerColumns.searchResultsPresent ? 0 : Math.min(rootList.implicitWidth, rootList.Layout.maximumWidth))
        Layout.minimumWidth: (sideBar.visible ? sideBar.implicitWidth + sideBar.Layout.rightMargin : 0) + minimumMainWidth
        LayoutMirroring.enabled: ((Plasmoid.location === PlasmaCore.Types.RightEdge)
            || (Application.layoutDirection === Qt.RightToLeft && Plasmoid.location !== PlasmaCore.Types.LeftEdge))

        KSvg.FrameSvgItem {
            id: sideBar

            property bool onTopPanel: Plasmoid.location === PlasmaCore.Types.TopEdge

            visible: (root.globalFavorites.count + root.systemFavorites.count) > 0

            Layout.fillHeight: true
            Layout.rightMargin: Kirigami.Units.smallSpacing

            implicitWidth: Math.max(favoriteApps.implicitWidth, favoriteSystemActions.implicitWidth) + margins.left + margins.right + sideBarScrollView.leftPadding + sideBarScrollView.rightPadding
            implicitHeight: sideBarLayout.implicitHeight + margins.top + margins.bottom

            imagePath: "widgets/frame"
            prefix: "plain"

            function handleLeftRightArrow(event: KeyEvent) : void {
                let backArrowKey = (event.key === Qt.Key_Left && !mainRow.LayoutMirroring.enabled) ||
                    (event.key === Qt.Key_Right && mainRow.LayoutMirroring.enabled)
                let forwardArrowKey = (event.key === Qt.Key_Right && !mainRow.LayoutMirroring.enabled) ||
                    (event.key === Qt.Key_Left && mainRow.LayoutMirroring.enabled)
                if (backArrowKey & runnerColumns.visibleChildren.length > 1) {
                    root.focusRunnerColumn(runnerColumns.visibleChildren.length - 2, true)
                } else if (forwardArrowKey) {
                    if (runnerColumns.visibleChildren.length > 1) {
                        root.focusRunnerColumn(0, true)
                    } else if (rootList.visible) {
                        root.focusRootList(true)
                    } else {
                        searchField.forceActiveFocus(Qt.TabFocusReason)
                    }
                } else {
                    event.accepted = false
                }
            }

            activeFocusOnTab: true
            onActiveFocusChanged: if (activeFocus) {
                let target = (onTopPanel && favoriteSystemActions.model.count) ? favoriteSystemActions : favoriteApps
                target.forceActiveFocus(Qt.TabFocusReason)
            }
            KeyNavigation.right: rootList

            PlasmaComponents3.ScrollView {
                id: sideBarScrollView

                anchors.fill: parent

                PlasmaComponents3.ScrollBar.horizontal.policy: PlasmaComponents3.ScrollBar.AlwaysOff
                PlasmaComponents3.ScrollBar.vertical.visible: Screen.height > 0 && sideBarScrollView.contentHeight > Math.round(Screen.height * 0.9)
                contentWidth: availableWidth

                function ensureVisible(item: Item) {
                    let flickable = (contentItem as Flickable)
                    let actualItemY = flickable.mapFromItem(item, 0, 0).y
                    let viewYPosition = (item.height <= contentItem.height)
                        ? Math.round(actualItemY + item.height / 2 - contentItem.height / 2)
                        : actualItemY
                    if (actualItemY < flickable.contentY) {
                        flickable.contentY = Math.max(0, viewYPosition)
                    } else if ((actualItemY + item.height) > (flickable.contentY + flickable.height)) {
                        flickable.contentY = Math.min(flickable.contentHeight - flickable.height, viewYPosition)
                    }
                    flickable.returnToBounds()
                }

                ColumnLayout {
                    id: sideBarLayout
                    height: Math.max(implicitHeight, sideBarScrollView.height - anchors.topMargin - anchors.bottomMargin)
                    anchors.top: parent.top
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.topMargin: sideBar.margins.top
                    anchors.leftMargin: sideBar.margins.left
                    anchors.rightMargin: sideBar.margins.right

                    Accessible.role: Accessible.List
                    Accessible.name: i18nc("@title:group accessible name for favorite group in sidebar", "Favorites")

                    Keys.onLeftPressed: event => sideBar.handleLeftRightArrow(event)
                    Keys.onRightPressed: event => sideBar.handleLeftRightArrow(event)

                    LayoutItemProxy {
                        target: sideBar.onTopPanel ? favoriteSystemActions : favoriteApps
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
                        Layout.bottomMargin: sideBar.margins.bottom
                    }

                    SideBarSection {
                        id: favoriteApps

                        Layout.fillHeight: true
                        Layout.alignment: Qt.AlignHCenter

                        KeyNavigation.up: favoriteSystemActions.bottomSideBarItem
                        KeyNavigation.down: favoriteSystemActions

                        model: root.globalFavorites
                        favoriteType: SideBarSection.FavoriteType.KAStats
                        onItemFocused: item => sideBarScrollView.ensureVisible(item)
                        onInteractionConcluded: root.interactionConcluded()

                        Binding {
                            target: root.globalFavorites
                            property: "iconSize"
                            value: Kirigami.Units.iconSizes.medium
                            restoreMode: Binding.RestoreBinding
                        }
                    }

                    SideBarSection {
                        id: favoriteSystemActions

                        Layout.alignment: Qt.AlignHCenter

                        model: root.systemFavorites
                        favoriteType: SideBarSection.FavoriteType.Simple
                        onItemFocused: item => sideBarScrollView.ensureVisible(item)
                        onInteractionConcluded: root.interactionConcluded()
                        KeyNavigation.up: favoriteApps.bottomSideBarItem
                        KeyNavigation.down: favoriteApps
                    }
                }
            }
        }

        ItemListView {
            id: rootList

            Layout.alignment: Qt.AlignTop
            Layout.bottomMargin: searchField.implicitHeight + Kirigami.Units.smallSpacing
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.maximumHeight: implicitHeight


            visible: searchField.text === ""

            iconsEnabled: Plasmoid.configuration.showIconsRootLevel

            hoverEnabled: !hoverBlock.enabled

            mainSearchField: searchField

            model: root.rootModel

            LayoutMirroring.enabled: mainRow.LayoutMirroring.enabled

            onExited: rootList.currentIndex = -1
            onInteractionConcluded: root.interactionConcluded()
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
        }

        RowLayout {
            id: runnerColumns

            readonly property bool searchResultsPresent: runnerColumns.visibleChildren[0] instanceof RunnerResultsList

            Layout.fillHeight: true

            visible: searchField.text !== "" && root.runnerModel.count > 0 && (!initialDelayTimer.active || !root.runnerModel.querying || searchResultsPresent)

            spacing: 0

            LayoutMirroring.enabled: mainRow.LayoutMirroring.enabled

            Timer {
                property bool active: false
                id: initialDelayTimer
                interval: 250 // match KRunner's delay for multi-runner queries
                onRunningChanged: if (running) { active = true }
                onTriggered: active = false
            }

            Repeater {
                id: runnerColumnsRepeater

                model: root.runnerModel

                delegate: RunnerResultsList {
                    id: runnerMatches

                    Layout.fillHeight: true
                    Layout.fillWidth: true

                    visible: model.count > 0 || (model.querying && visible)
                    hoverEnabled: !hoverBlock.enabled

                    model: root.runnerModel.modelForRow(index)
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

                    onListActiveFocusChanged: {
                        if (!listActiveFocus) {
                            currentIndex = -1;
                        }
                    }

                    onInteractionConcluded: root.interactionConcluded()
                    onNavigateLeftRequested: navigateToAdjacentColumn(false)
                    onNavigateRightRequested: navigateToAdjacentColumn(true)
                }
            }

        }
        PlasmaExtras.PlaceholderMessage {
            id: noMatchesPlaceholder

            Layout.minimumWidth: mainRow.minimumMainWidth
            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter

            visible: root.runnerModel.query !== "" && !runnerColumns.searchResultsPresent && ((!root.runnerModel.querying && !initialDelayTimer.active) || visible)
            iconName: "edit-none"
            text: i18nc("@info:status", "No matches")

            Binding {
                searchField.width: noMatchesPlaceholder.width
                when: noMatchesPlaceholder.visible
            }
        }
    }

    PlasmaExtras.SearchField {
        id: searchField

        readonly property int spacing: sideBar.width ? sideBar.Layout.rightMargin : 0

        anchors.bottom: mainRow.bottom
        anchors.left: parent.left
        anchors.leftMargin: sideBar.visible ? sideBar.width + spacing : 0

        readonly property real defaultWidth: Kirigami.Units.gridUnit * 14

        width: runnerColumns.visible && runnerColumns.searchResultsPresent
            ? runnerColumns.visibleChildren[0].width - (runnerColumns.visibleChildren.length > 2 ? Kirigami.Units.smallSpacing : 0)
            : (rootList.visible ? rootList.width : mainRow.minimumMainWidth)

        focus: !Kirigami.InputMethod.willShowOnActive

        onTextChanged: {
            initialDelayTimer.restart()
            root.runnerModel.query = text;
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
                when: (Plasmoid.location === PlasmaCore.Types.RightEdge && Application.layoutDirection === Qt.LeftToRight)
                    || (Plasmoid.location === PlasmaCore.Types.LeftEdge && Application.layoutDirection === Qt.RightToLeft)

                AnchorChanges {
                    target: searchField
                    anchors.top: undefined
                    anchors.bottom: mainRow.bottom
                    anchors.left: undefined
                    anchors.right: parent.right
                }

                PropertyChanges {
                    searchField.anchors.leftMargin: undefined
                    searchField.anchors.rightMargin: sideBar.visible ? sideBar.width + spacing : 0
                }
            }
        ]

        function handleUpDownArrow(event: KeyEvent) : void {
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
            } else {
                event.accepted = false
            }
        }

        function handleLeftRightArrow(event: KeyEvent) : void {
            let backArrowKey = (event.key === Qt.Key_Left && !mainRow.LayoutMirroring.enabled) ||
                (event.key === Qt.Key_Right && mainRow.LayoutMirroring.enabled)
            let forwardArrowKey = (event.key === Qt.Key_Right && !mainRow.LayoutMirroring.enabled) ||
                (event.key === Qt.Key_Left && mainRow.LayoutMirroring.enabled)

            if (backArrowKey) {
                if (!sideBar.visible && !runnerColumns.visible) {
                    return;
                }
                if (runnerColumns.visibleChildren[0]?.length > 1) {
                    runnerColumns.visibleChildren[0].currentIndex = -1;
                }
                if (sideBar.visible) {
                    root.focusSideBar(true)
                } else {
                    root.focusRunnerColumn(runnerColumns.visibleChildren.length - 2, true)
                }
            } else if (forwardArrowKey && runnerColumns.visibleChildren.length > 2) {
                runnerColumns.visibleChildren[0].currentIndex = -1;
                root.focusRunnerColumn(1, true)
            } else {
                event.accepted = false
            }
        }

        function launchBestMatch() : void  {
            if (launchMatchTimer.running) {
                launchMatchTimer.stop()
                launchMatchTimer.triggered()
                return
            }
            if (!root.runnerModel.querying || runnerColumns.visibleChildren[0]?.currentItem?.text.toLowerCase().includes(root.runnerModel.query.toLowerCase())) {
                launchMatchTimer.triggered()
                return
            }
            launchMatchTimer.start()
        }

        Timer {
            id: launchMatchTimer
            interval: 750
            onTriggered: runnerColumns.visibleChildren[0]?.currentItem?.action.trigger()
        }

        Connections {
            target: runnerColumns,visibleChildren[0] instanceof RunnerResultsList ? runnerColumns.visibleChildren[0] : null
            enabled: launchMatchTimer.running
            function onCurrentItemChanged() : void {
                if (runnerColumns.visibleChildren[0]?.currentItem?.text.toLowerCase().includes(root.runnerModel.query.toLowerCase())) {
                    launchMatchTimer.stop()
                    launchMatchTimer.triggered()
                }
            }
        }

        Connections {
            target: root.runnerModel
            enabled: launchMatchTimer.running
            function onQueryFinished() : void {
                launchMatchTimer.stop()
                Qt.callLater(launchMatchTimer.triggered) // callLater to give bindings time to update
            }
        }

        Keys.priority: Keys.AfterItem // arrow keys should move cursor first
        Keys.onUpPressed: event => handleUpDownArrow(event)
        Keys.onDownPressed: event => handleUpDownArrow(event)
        Keys.onLeftPressed: event => handleLeftRightArrow(event)
        Keys.onRightPressed: event => handleLeftRightArrow(event)
        Keys.onEnterPressed: launchBestMatch()
        Keys.onReturnPressed: launchBestMatch()
        Keys.onEscapePressed: root.interactionConcluded()

        function appendText(newText) {
            focus = true;
            text = text + newText;
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

        rootModel.refresh();
    }

    HoverBlocker {
        id: hoverBlock

        anchors.fill: parent
        onWidthChanged: hoverBlock.reset()

        Connections {
            target: root.runnerModel

            function onQueryChanged() : void {
                hoverBlock.reset()
            }
        }
    }
}
