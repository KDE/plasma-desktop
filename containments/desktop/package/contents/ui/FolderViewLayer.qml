/*
    SPDX-FileCopyrightText: 2014-2015 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15
import QtQml 2.15

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 3.0 as PlasmaComponents
import org.kde.config // for KAuthorized
import org.kde.kirigami 2.20 as Kirigami

import org.kde.private.desktopcontainment.folder 0.1 as Folder

FocusScope {
    id: folderViewLayerComponent

    property var sharedActions: ["newMenu", "paste", "undo", "refresh", "emptyTrash"]
    property Component folderViewDialogComponent: Qt.createComponent("FolderViewDialog.qml", Qt.Asynchronous, root)

    property Item view: folderView
    property Item label: null
    property int labelHeight: Kirigami.Units.iconSizes.sizeForLabels
        + (root.isPopup ? (Kirigami.Units.smallSpacing * 2) : 0)

    property alias model: folderView.model
    property alias overflowing: folderView.overflowing
    property alias flow: folderView.flow

    property string resolution: Math.round(plasmoid.screenGeometry.width) + "x" + Math.round(plasmoid.screenGeometry.height)

    readonly property bool lockedByKiosk: !KAuthorized.authorize("editable_desktop_icons")

    focus: true

    function updateContextualActions() {
        folderView.model.updateActions();

        for (let i = 0, len = sharedActions.length; i < len; i++) {
            const actionName = sharedActions[i];
            const appletAction = plasmoid.internalAction(actionName);
            if (appletAction) {
                modelAction = folderView.model.action(actionName);
                appletAction.text = modelAction.text;
                appletAction.enabled = modelAction.enabled;
                appletAction.visible = modelAction.visible;
            }
        }
    }

    function cancelRename() {
        folderView.cancelRename();
    }

    function goHome() {
        if (folderView.url !== plasmoid.configuration.url) {
            folderView.url = Qt.binding(() => plasmoid.configuration.url);
            folderView.history = [];
            folderView.updateHistory();
        }
    }

    Binding {
        target: plasmoid
        property: "title"
        value: labelGenerator.displayLabel
        restoreMode: Binding.RestoreBinding
    }

    Folder.LabelGenerator {
        id: labelGenerator

        folderModel: folderView.model
        rtl: (Qt.application.layoutDirection === Qt.RightToLeft)
        labelMode: plasmoid.configuration.labelMode || (isContainment ? 0 : 1)
        labelText: plasmoid.configuration.labelText
    }

    Folder.ViewPropertiesMenu {
        id: viewPropertiesMenu

        showLayoutActions: !isPopup
        showLockAction: isContainment
        showIconSizeActions: !root.useListViewMode

        lockedEnabled: !lockedByKiosk

        onArrangementChanged: {
            plasmoid.configuration.arrangement = arrangement;
        }

        onAlignmentChanged: {
            plasmoid.configuration.alignment = alignment;
        }

        onPreviewsChanged: {
            plasmoid.configuration.previews = previews;
        }

        onLockedChanged: {
            if (!lockedByKiosk) {
                plasmoid.configuration.locked = locked;
            }
        }

        onSortModeChanged: {
            plasmoid.configuration.sortMode = sortMode;
        }

        onSortDescChanged: {
            plasmoid.configuration.sortDesc = sortDesc;
        }

        onSortDirsFirstChanged: {
            plasmoid.configuration.sortDirsFirst = sortDirsFirst;
        }

        onIconSizeChanged: {
            plasmoid.configuration.iconSize = iconSize;
        }

        Component.onCompleted: {
            arrangement = plasmoid.configuration.arrangement;
            alignment = plasmoid.configuration.alignment;
            previews = plasmoid.configuration.previews;
            locked = plasmoid.configuration.locked || lockedByKiosk;
            sortMode = plasmoid.configuration.sortMode;
            sortDesc = plasmoid.configuration.sortDesc;
            sortDirsFirst = plasmoid.configuration.sortDirsFirst;
            iconSize = plasmoid.configuration.iconSize;
        }
    }

    PlasmaComponents.Label {
        anchors.fill: parent

        text: folderView.errorString

        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        wrapMode: Text.Wrap
    }

    Connections {
        target: root

        function onExpandedChanged() {
            if (root.isPopup) {
                if (root.expanded) {
                    folderView.currentIndex = -1;
                    folderView.forceActiveFocus();
                    folderView.positionViewAtBeginning();
                } else {
                    goHome();

                    folderView.currentIndex = -1;
                    folderView.model.clearSelection();
                    folderView.cancelRename();
                }
            }
        }
    }

    function getPositions() {
        let allPositions;
        try {
            allPositions = JSON.parse(plasmoid.configuration.positions);
        } catch (err) {
            allPositions = {};
            allPositions[resolution] = plasmoid.configuration.positions;
        }
        return allPositions[resolution] || "";
    }

    function savePositions(positions) {
        let allPositions;
        try {
            allPositions = JSON.parse(plasmoid.configuration.positions);
        } catch (err) {
            allPositions = {};
        }
        allPositions[resolution] = positions;
        plasmoid.configuration.positions = JSON.stringify(allPositions, Object.keys(allPositions).sort());
    }

    Connections {
        target: plasmoid.configuration

        function onArrangementChanged() {
            viewPropertiesMenu.arrangement = plasmoid.configuration.arrangement;
        }

        function onAlignmentChanged() {
            viewPropertiesMenu.alignment = plasmoid.configuration.alignment;
        }

        function onLockedChanged() {
            viewPropertiesMenu.locked = plasmoid.configuration.locked;
        }

        function onSortModeChanged() {
            folderView.sortMode = plasmoid.configuration.sortMode;
            viewPropertiesMenu.sortMode = plasmoid.configuration.sortMode;
        }

        function onSortDescChanged() {
            viewPropertiesMenu.sortDesc = plasmoid.configuration.sortDesc;
        }

        function onSortDirsFirstChanged() {
            viewPropertiesMenu.sortDirsFirst = plasmoid.configuration.sortDirsFirst;
        }

        function onIconSizeChanged() {
            viewPropertiesMenu.iconSize = plasmoid.configuration.iconSize;
        }

        function onPositionsChanged() {
            folderView.positions = getPositions();
        }
    }

    FolderView {
        id: folderView

        anchors.left: parent.left
        anchors.top: parent.top
        anchors.topMargin: folderViewLayerComponent.label !== null ? folderViewLayerComponent.label.height : 0
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        focus: true
        isRootView: true

        url: plasmoid.configuration.url
        locked: (plasmoid.configuration.locked || !isContainment || lockedByKiosk)
        filterMode: plasmoid.configuration.filterMode
        filterPattern: plasmoid.configuration.filterPattern
        filterMimeTypes: plasmoid.configuration.filterMimeTypes
        showHiddenFiles: plasmoid.configuration.showHiddenFiles

        flow: (plasmoid.configuration.arrangement === 0) ? GridView.FlowLeftToRight : GridView.FlowTopToBottom
        layoutDirection: (plasmoid.configuration.alignment === 0) ? Qt.LeftToRight : Qt.RightToLeft

        onSortModeChanged: {
            plasmoid.configuration.sortMode = sortMode;
        }

        onPositionsChanged: {
            savePositions(folderView.positions);
        }

        onPerStripeChanged: {
            folderView.positions = getPositions();
        }

        Component.onCompleted: {
            folderView.sortMode = plasmoid.configuration.sortMode;
            folderView.positions = getPositions();
        }
    }

    Component {
        id: labelComponent

        Item {
            id: label

            // If we bind height to visible, it will be invisible initially (since "visible"
            // propagates recursively) and that confuses the Label, hence the temp property.
            readonly property bool active: (plasmoid.configuration.labelMode !== 0)

            readonly property bool showPin: root.isPopup && root.compactRepresentationItem && root.compactRepresentationItem.visible

            width: parent.width
            height: active ? labelHeight : 0

            visible: active

            property Item windowPin: null
            property Item homeButton: null

            onVisibleChanged: {
                if (root.isPopup && !visible) {
                    root.hideOnWindowDeactivate = true;
                }
            }

            onShowPinChanged: {
                if (!windowPin && showPin) {
                    windowPin = windowPinComponent.createObject(label);
                } else if (windowPin) {
                    windowPin.destroy();
                    windowPin = null;
                }
            }

            Connections {
                target: folderView

                function onUrlChanged() {
                    if (!label.homeButton && folderView.url !== plasmoid.configuration.url) {
                        label.homeButton = homeButtonComponent.createObject(label);
                    } else if (label.homeButton && folderView.url === plasmoid.configuration.url) {
                        label.homeButton.destroy();
                    }
                }
            }

            PlasmaComponents.Label {
                id: text

                anchors {
                    left: label.homeButton ? label.homeButton.right : parent.left
                    right: label.windowPin ? label.windowPin.left : parent.right
                    margins: Kirigami.Units.smallSpacing
                }
                height: parent.height

                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignTop
                elide: Text.ElideMiddle
                text: labelGenerator.displayLabel
                font.underline: labelMouseArea.containsMouse
            }

            MouseArea {
                id: labelMouseArea
                anchors {
                    top: text.top
                    horizontalCenter: text.horizontalCenter
                }
                width: text.contentWidth
                height: text.contentHeight
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor

                onClicked: {
                    Folder.AppLauncher.openUrl(folderView.url)
                }
            }

            Component {
                id: windowPinComponent

                PlasmaComponents.ToolButton {
                    id: windowPin

                    anchors.right: parent.right

                    visible: label.showPin

                    width: root.isPopup ? Math.round(Kirigami.Units.gridUnit * 1.25) : 0
                    height: width
                    checkable: true
                    icon.name: "window-pin"
                    onCheckedChanged: root.hideOnWindowDeactivate = !checked
                }
            }

            Component {
                id: homeButtonComponent

                PlasmaComponents.ToolButton {
                    id: homeButton

                    anchors.left: parent.left

                    visible: root.isPopup && folderView.url !== plasmoid.configuration.url

                    width: root.isPopup ? Math.round(Kirigami.Units.gridUnit * 1.25) : 0
                    height: width
                    icon.name: "go-home"

                    onClicked: goHome()
                }
            }

            Component.onCompleted: {
                if (root.showPin) {
                    windowPin = windowPinComponent.createObject(label);
                }
            }
        }
    }


    PlasmaCore.Action {
        id: viewPropertiesAction
        text: i18n("Icons")
        icon.name: "view-list-icons"
        menu: viewPropertiesMenu.menu
    }
    PlasmaCore.Action {
        id: actionSeparator
        isSeparator: true
    }

    Component.onCompleted: {
        if (!isContainment) {
            label = labelComponent.createObject(folderViewLayerComponent);
        }

        for (let i = 0, len = sharedActions.length; i < len; i++) {
            const actionName = sharedActions[i];
            const modelAction = folderView.model.action(actionName);
            plasmoid.contextualActions.push(modelAction)
            if (actionName === "newMenu") {
                plasmoid.contextualActions.push(viewPropertiesAction)
            }
        }

        plasmoid.contextualActions.push(actionSeparator);

        plasmoid.contextualActionsAboutToShow.connect(updateContextualActions);
        plasmoid.contextualActionsAboutToShow.connect(folderView.model.clearSelection);
    }
}
