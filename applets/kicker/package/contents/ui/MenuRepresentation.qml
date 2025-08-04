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


    function reset() {
        kicker.hideOnWindowDeactivate = true;

        rootList.currentIndex = -1;
        rootList.mouseMoved = false;

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
                ? Kirigami.Units.iconSizes.medium + margins.left + margins.right : 0)
            height: parent.height

            imagePath: "widgets/frame"
            prefix: "plain"

            activeFocusOnTab: true
            onActiveFocusChanged: if (activeFocus) {
                (onTopPanel ? favoriteSystemActions : favoriteApps).forceActiveFocus(Qt.TabFocusReason)
            }
            KeyNavigation.right: rootList
            Keys.onPressed: event => {
                let backArrowKey = (event.key === Qt.Key_Left && Application.layoutDirection === Qt.LeftToRight) ||
                    (event.key === Qt.Key_Right && Application.layoutDirection === Qt.RightToLeft)
                let forwardArrowKey = (event.key === Qt.Key_Right && Application.layoutDirection === Qt.LeftToRight) ||
                    (event.key === Qt.Key_Left && Application.layoutDirection === Qt.RightToLeft)

                if (backArrowKey & runnerColumns.visibleChildren.length > 1) {
                    const targetList = runnerColumns.visibleChildren[runnerColumns.visibleChildren.length-2]
                    targetList.currentIndex = 0
                    targetList.forceActiveFocus(Qt.BacktabFocusReason)
                    root.ensureVisible(targetList)
                    event.accepted = true
                }
                if (forwardArrowKey) {
                    if (runnerColumns.visible) {
                        runnerColumns.visibleChildren[0].currentIndex = 0
                        runnerColumns.visibleChildren[0].forceActiveFocus(Qt.TabFocusReason)
                        root.ensureVisible(runnerColumns.visibleChildren[0])
                    } else {
                        rootList.showChildDialogs = false;
                        rootList.currentIndex = 0
                        rootList.forceActiveFocus(Qt.TabFocusReason)
                        rootList.showChildDialogs = true;
                    }
                }
            }

            ColumnLayout {
                anchors.top: parent.top
                anchors.horizontalCenter: parent.horizontalCenter
                height: Math.max(implicitHeight, parent.height)

                LayoutItemProxy {
                    target: sideBar.onTopPanel ? favoriteSystemActions : favoriteApps
                }
                LayoutItemProxy {
                    target: sidebarSeparator
                }
                LayoutItemProxy {
                    target: sideBar.onTopPanel ? favoriteApps : favoriteSystemActions
                }

                SideBarSection {
                    id: favoriteApps

                    Layout.fillHeight: true

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

                SideBarSection {
                    id: favoriteSystemActions

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
            maximumWidth: Math.round(minimumWidth * 1.5)
            height: ((rootModel.count - rootModel.separatorCount) * itemHeight) + (rootModel.separatorCount * separatorHeight)

            visible: searchField.text === ""

            iconsEnabled: Plasmoid.configuration.showIconsRootLevel

            mouseMoved: false // don't hover-activate until mouse is moved to not interfere with keyboard use

            mainSearchField: searchField

            model: rootModel

            onNavigateLeftRequested: {
                currentIndex = -1
                sideBar.forceActiveFocus(Qt.TabFocusReason)
                root.ensureVisible(sideBar)
            }
            onKeyNavigationAtListEnd: {
                searchField.focus = true;
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

            Repeater {
                id: runnerColumnsRepeater

                model: runnerModel

                delegate: RunnerResultsList {
                    id: runnerMatches

                    visible: runnerModel.modelForRow(index).count > 0

                    mainSearchField: searchField

                    onKeyNavigationAtListEnd: {
                        searchField.focus = true;
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

                    onNavigateLeftRequested: {
                        let target = null;
                        const targets = [];

                        for (let i = index - 1; i >= 0; --i) {
                            targets.push(i);
                        }

                        for (const i of targets) {
                            if (runnerModel.modelForRow(i).count) {
                                target = runnerColumnsRepeater.itemAt(i);
                                break;
                            }
                        }

                        if (target) {
                            currentIndex = -1;
                            target.currentIndex = 0;
                            target.focus = true;
                            root.ensureVisible(target)
                        } else {
                            currentIndex = -1;
                            sideBar.forceActiveFocus(Qt.TabFocusReason)
                            root.ensureVisible(sideBar)
                        }
                    }

                    onNavigateRightRequested: {
                        let target = null;
                        const targets = [];

                        for (let i = index + 1; i < runnerModel.count; ++i) {
                            targets.push(i);
                        }

                        for (const i of targets) {
                            if (runnerModel.modelForRow(i).count) {
                                target = runnerColumnsRepeater.itemAt(i);
                                break;
                            }
                        }

                        if (target) {
                            currentIndex = -1;
                            target.currentIndex = 0;
                            target.focus = true;
                            root.ensureVisible(target)
                        } else {
                            currentIndex = -1;
                            sideBar.forceActiveFocus(Qt.TabFocusReason)
                            root.ensureVisible(sideBar)
                        }
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
                name: "top"
                when: Plasmoid.location === PlasmaCore.Types.TopEdge

                AnchorChanges {
                    target: searchField
                    anchors.top: undefined
                    anchors.bottom: mainRow.bottom
                    anchors.left: parent.left
                    anchors.right: undefined
                }

                PropertyChanges {
                    target: searchField
                    anchors.leftMargin: sideBar.width + spacing
                    anchors.rightMargin: undefined
                }
            },
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
            let backArrowKey = (event.key === Qt.Key_Left && Application.layoutDirection === Qt.LeftToRight) ||
                (event.key === Qt.Key_Right && Application.layoutDirection === Qt.RightToLeft)
            let forwardArrowKey = (event.key === Qt.Key_Right && Application.layoutDirection === Qt.LeftToRight) ||
                (event.key === Qt.Key_Left && Application.layoutDirection === Qt.RightToLeft)
            if (event.key === Qt.Key_Up) {
                if (rootList.visible) {
                    rootList.showChildDialogs = false;
                    rootList.currentIndex = rootList.model.count - 1;
                    rootList.forceActiveFocus();
                    rootList.showChildDialogs = true;
                }

                if (runnerColumns.visible) {
                    const targetList =  runnerColumns.visibleChildren[0];
                    targetList.currentIndex = targetList.count-1;
                    targetList.currentItem.forceActiveFocus();
                    root.ensureVisible(targetList)
                }
                event.accepted = true;
            } else if (event.key === Qt.Key_Down) {
                if (rootList.visible) {
                    rootList.showChildDialogs = false;
                    rootList.currentIndex = Math.min(rootList.currentIndex + 1, rootList.count);
                    rootList.forceActiveFocus();
                    rootList.showChildDialogs = true;
                }

                if (runnerColumns.visible) {
                    const targetList =  runnerColumns.visibleChildren[0];
                    targetList.currentIndex = Math.min(targetList.currentIndex + 1, targetList.count);
                    targetList.currentItem.forceActiveFocus();
                    root.ensureVisible(targetList)
                }
                event.accepted = true;
            } else if (backArrowKey) {
                if (runnerColumns.visible) {
                    runnerColumns.visibleChildren[0].currentIndex = -1;
                }
                sideBar.forceActiveFocus(Qt.TabFocusReason)
                root.ensureVisible(sideBar)
                event.accepted = true;
            } else if (forwardArrowKey && runnerColumns.visibleChildren.length > 2) {
                runnerColumns.visibleChildren[0].currentIndex = -1;
                runnerColumns.visibleChildren[1].currentIndex = 0;
                runnerColumns.visibleChildren[1].forceActiveFocus(Qt.TabFocusReason);
                root.ensureVisible(runnerColumns.visibleChildren[1])
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
}
