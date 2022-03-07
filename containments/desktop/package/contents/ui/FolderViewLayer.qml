/*
    SPDX-FileCopyrightText: 2014-2015 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.0
import QtQml 2.15

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.kconfig 1.0 // for KAuthorized

import org.kde.private.desktopcontainment.folder 0.1 as Folder

FocusScope {
    id: folderViewLayerComponent

    property variant sharedActions: ["newMenu", "paste", "undo", "refresh", "emptyTrash"]
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
            appletAction = Plasmoid.action(actionName);
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
        if (folderView.url !== Plasmoid.configuration.url) {
            folderView.url = Qt.binding(function() {
                return Plasmoid.configuration.url;
            });
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
        target: Plasmoid.self
        property: "title"
        value: labelGenerator.displayLabel
        restoreMode: Binding.RestoreBinding
    }

    Folder.LabelGenerator {
        id: labelGenerator

        folderModel: folderView.model
        rtl: (Qt.application.layoutDirection == Qt.RightToLeft)
        labelMode: Plasmoid.configuration.labelMode || (isContainment ? 0 : 1)
        labelText: Plasmoid.configuration.labelText
    }

    Folder.ViewPropertiesMenu {
        id: viewPropertiesMenu

        showLayoutActions: !isPopup
        showLockAction: isContainment
        showIconSizeActions: !root.useListViewMode

        lockedEnabled: !lockedByKiosk

        onArrangementChanged: {
            Plasmoid.configuration.arrangement = arrangement;
        }

        onAlignmentChanged: {
            Plasmoid.configuration.alignment = alignment;
        }

        onPreviewsChanged: {
            Plasmoid.configuration.previews = previews;
        }

        onLockedChanged: {
            if (!lockedByKiosk) {
                Plasmoid.configuration.locked = locked;
            }
        }

        onSortModeChanged: {
            Plasmoid.configuration.sortMode = sortMode;
        }

        onSortDescChanged: {
            Plasmoid.configuration.sortDesc = sortDesc;
        }

        onSortDirsFirstChanged: {
            Plasmoid.configuration.sortDirsFirst = sortDirsFirst;
        }

        onIconSizeChanged: {
            Plasmoid.configuration.iconSize = iconSize;
        }

        Component.onCompleted: {
            arrangement = Plasmoid.configuration.arrangement;
            alignment = Plasmoid.configuration.alignment;
            previews = Plasmoid.configuration.previews;
            locked = Plasmoid.configuration.locked || lockedByKiosk;
            sortMode = Plasmoid.configuration.sortMode;
            sortDesc = Plasmoid.configuration.sortDesc;
            sortDirsFirst = Plasmoid.configuration.sortDirsFirst;
            iconSize = Plasmoid.configuration.iconSize;
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
        target: Plasmoid.self

        function onExpandedChanged() {
            if (root.isPopup) {
                if (Plasmoid.expanded) {
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
            Plasmoid.configuration.url = data
        }
    }

    function getPositions() {
        try {
            var allPositions = JSON.parse(plasmoid.configuration.positions);
        } catch (err) {
            var allPositions = {};
            allPositions[resolution] = plasmoid.configuration.positions;
        }
        return allPositions[resolution] || "";
    }

    function savePositions(positions) {
        try {
            var allPositions = JSON.parse(plasmoid.configuration.positions);
        } catch (err) {
            var allPositions = {};
        }
        allPositions[resolution] = positions;
        plasmoid.configuration.positions = JSON.stringify(allPositions, Object.keys(allPositions).sort());
    }

    Connections {
        target: Plasmoid.configuration

        function onArrangementChanged() {
            viewPropertiesMenu.arrangement = Plasmoid.configuration.arrangement;
        }

        function onAlignmentChanged() {
            viewPropertiesMenu.alignment = Plasmoid.configuration.alignment;
        }

        function onLockedChanged() {
            viewPropertiesMenu.locked = Plasmoid.configuration.locked;
        }

        function onSortModeChanged() {
            folderView.sortMode = Plasmoid.configuration.sortMode;
            viewPropertiesMenu.sortMode = Plasmoid.configuration.sortMode;
        }

        function onSortDescChanged() {
            viewPropertiesMenu.sortDesc = Plasmoid.configuration.sortDesc;
        }

        function onSortDirsFirstChanged() {
            viewPropertiesMenu.sortDirsFirst = Plasmoid.configuration.sortDirsFirst;
        }

        function onIconSizeChanged() {
            viewPropertiesMenu.iconSize = Plasmoid.configuration.iconSize;
        }

        function onPositionsChanged() {
            folderView.positions = getPositions();
        }
    }

    FolderView {
        id: folderView

        anchors.left: parent.left
        anchors.top: parent.top
        anchors.topMargin: folderViewLayerComponent.label != null ? folderViewLayerComponent.label.height : 0
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        focus: true
        isRootView: true

        url: Plasmoid.configuration.url
        locked: (Plasmoid.configuration.locked || !isContainment || lockedByKiosk)
        filterMode: Plasmoid.configuration.filterMode
        filterPattern: Plasmoid.configuration.filterPattern
        filterMimeTypes: Plasmoid.configuration.filterMimeTypes

        flow: (Plasmoid.configuration.arrangement === 0) ? GridView.FlowLeftToRight : GridView.FlowTopToBottom
        layoutDirection: (Plasmoid.configuration.alignment === 0) ? Qt.LeftToRight : Qt.RightToLeft

        onSortModeChanged: {
            Plasmoid.configuration.sortMode = sortMode;
        }

        onPositionsChanged: {
            savePositions(folderView.positions);
        }

        onPerStripeChanged: {
            folderView.positions = getPositions();
        }

        Component.onCompleted: {
            folderView.sortMode = Plasmoid.configuration.sortMode;
            folderView.positions = getPositions();
        }
    }

    Component {
        id: labelComponent

        Item {
            id: label

            // If we bind height to visible, it will be invisible initially (since "visible"
            // propagates recursively) and that confuses the Label, hence the temp property.
            readonly property bool active: (Plasmoid.configuration.labelMode !== 0)

            readonly property bool showPin: root.isPopup && Plasmoid.compactRepresentationItem && Plasmoid.compactRepresentationItem.visible

            width: parent.width
            height: active ? labelHeight : 0

            visible: active

            property Item windowPin: null
            property Item homeButton: null

            onVisibleChanged: {
                if (root.isPopup && !visible) {
                    Plasmoid.hideOnWindowDeactivate = true;
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
                    if (!label.homeButton && folderView.url !== Plasmoid.configuration.url) {
                        label.homeButton = homeButtonComponent.createObject(label);
                    } else if (label.homeButton && folderView.url === Plasmoid.configuration.url) {
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
                    var action = Plasmoid.action("run associated application");

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
                    onCheckedChanged: Plasmoid.hideOnWindowDeactivate = !checked
                }
            }

            Component {
                id: homeButtonComponent

                PlasmaComponents.ToolButton {
                    id: homeButton

                    anchors.left: parent.left

                    visible: root.isPopup && folderView.url !== Plasmoid.configuration.url

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
            Plasmoid.setAction(actionName, modelAction.text, Folder.MenuHelper.iconName(modelAction));

            var plasmoidAction = Plasmoid.action(actionName);
            plasmoidAction.shortcut = modelAction.shortcut;
            plasmoidAction.shortcutContext = Qt.WidgetShortcut;

            if (actionName === "newMenu") {
                Folder.MenuHelper.setMenu(plasmoidAction, folderView.model.newMenu);
                Plasmoid.setActionSeparator("separator1");

                Plasmoid.setAction("viewProperties", i18n("Icons"), "preferences-desktop-icons");
                Folder.MenuHelper.setMenu(Plasmoid.action("viewProperties"), viewPropertiesMenu.menu);
            } else {
                plasmoidAction.triggered.connect(modelAction.trigger);
            }
        }

        Plasmoid.setActionSeparator("separator2");

        Plasmoid.contextualActionsAboutToShow.connect(updateContextualActions);
        Plasmoid.contextualActionsAboutToShow.connect(folderView.model.clearSelection);
    }
}
