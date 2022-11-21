/*
    SPDX-FileCopyrightText: 2014-2015 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15
import QtQml 2.15

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.kconfig 1.0 // for KAuthorized

import org.kde.private.desktopcontainment.folder 0.1 as Folder

FocusScope {
    id: folderViewLayerComponent

    property var sharedActions: ["newMenu", "paste", "undo", "refresh", "emptyTrash"]
    property Component folderViewDialogComponent: Qt.createComponent("FolderViewDialog.qml", Qt.Asynchronous, root)

    property Item view: folderView
    property Item label: null
    property int labelHeight: PlasmaCore.Theme.mSize(PlasmaCore.Theme.defaultFont).height
        + (root.isPopup ? (PlasmaCore.Units.smallSpacing * 2) : 0)

    property alias model: folderView.model
    property alias overflowing: folderView.overflowing
    property alias flow: folderView.flow

    property string resolution: Math.round(plasmoid.screenGeometry.width) + "x" + Math.round(plasmoid.screenGeometry.height)

    readonly property bool lockedByKiosk: !KAuthorized.authorize("editable_desktop_icons")

    focus: true

    function updateContextualActions() {
        folderView.model.updateActions();

        var actionName = "";
        var appletAction = null;
        var modelAction = null;

        for (var i = 0; i < sharedActions.length; i++) {
            actionName = sharedActions[i];
            appletAction = plasmoid.action(actionName);
            modelAction = folderView.model.action(actionName);

            appletAction.text = modelAction.text;
            appletAction.enabled = modelAction.enabled;
            appletAction.visible = modelAction.visible;
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

    PlasmaCore.Svg {
        id: actionOverlays

        imagePath: "widgets/action-overlays"
        multipleImages: true
        size: "16x16"
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
        target: plasmoid

        function onExpandedChanged() {
            if (root.isPopup) {
                if (plasmoid.expanded) {
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

        function onExternalData(mimetype, data) {
            plasmoid.configuration.url = data
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

            readonly property bool showPin: root.isPopup && plasmoid.compactRepresentationItem && plasmoid.compactRepresentationItem.visible

            width: parent.width
            height: active ? labelHeight : 0

            visible: active

            property Item windowPin: null
            property Item homeButton: null

            onVisibleChanged: {
                if (root.isPopup && !visible) {
                    plasmoid.hideOnWindowDeactivate = true;
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
                    margins: PlasmaCore.Units.smallSpacing
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
                    var action = plasmoid.action("run associated application");

                    if (action) {
                        action.trigger();
                    }
                }
            }

            Component {
                id: windowPinComponent

                PlasmaComponents.ToolButton {
                    id: windowPin

                    anchors.right: parent.right

                    visible: label.showPin

                    width: root.isPopup ? Math.round(PlasmaCore.Units.gridUnit * 1.25) : 0
                    height: width
                    checkable: true
                    iconSource: "window-pin"
                    onCheckedChanged: plasmoid.hideOnWindowDeactivate = !checked
                }
            }

            Component {
                id: homeButtonComponent

                PlasmaComponents.ToolButton {
                    id: homeButton

                    anchors.left: parent.left

                    visible: root.isPopup && folderView.url !== plasmoid.configuration.url

                    width: root.isPopup ? Math.round(PlasmaCore.Units.gridUnit * 1.25) : 0
                    height: width
                    iconSource: "go-home"

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

    Component.onCompleted: {
        if (!isContainment) {
            label = labelComponent.createObject(folderViewLayerComponent);
        }

        var actionName = "";
        var modelAction = null;

        for (var i = 0; i < sharedActions.length; i++) {
            actionName = sharedActions[i];
            modelAction = folderView.model.action(actionName);
            plasmoid.setAction(actionName, modelAction.text, Folder.MenuHelper.iconName(modelAction));

            var plasmoidAction = plasmoid.action(actionName);
            plasmoidAction.shortcut = modelAction.shortcut;
            plasmoidAction.shortcutContext = Qt.WidgetShortcut;

            if (actionName === "newMenu") {
                Folder.MenuHelper.setMenu(plasmoidAction, folderView.model.newMenu);
                plasmoid.setActionSeparator("separator1");

                plasmoid.setAction("viewProperties", i18n("Icons"), "view-list-icons");
                Folder.MenuHelper.setMenu(plasmoid.action("viewProperties"), viewPropertiesMenu.menu);
            } else {
                plasmoidAction.triggered.connect(modelAction.trigger);
            }
        }

        plasmoid.setActionSeparator("separator2");

        plasmoid.contextualActionsAboutToShow.connect(updateContextualActions);
        plasmoid.contextualActionsAboutToShow.connect(folderView.model.clearSelection);
    }
}
