/*
    SPDX-FileCopyrightText: 2014-2015 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQml

import org.kde.plasma.plasmoid
import org.kde.plasma.core as PlasmaCore
import org.kde.plasma.components as PlasmaComponents
import org.kde.config // for KAuthorized
import org.kde.kirigami as Kirigami

import org.kde.private.desktopcontainment.folder as Folder

FocusScope {
    id: folderViewLayerComponent

    property var sharedActions: ["newMenu", "paste", "undo", "emptyTrash"]
    property Component folderViewDialogComponent: Qt.createComponent("FolderViewDialog.qml", Qt.Asynchronous, root)

    property Item view: folderView
    property Item label: null
    property int labelHeight: Kirigami.Units.iconSizes.sizeForLabels + (Kirigami.Units.smallSpacing * 2)

    property alias model: folderView.model
    property alias overflowing: folderView.overflowing
    property alias flow: folderView.flow

    readonly property bool lockedByKiosk: !KAuthorized.authorize("editable_desktop_icons")

    focus: true

    function updateContextualActions() {
        folderView.model.updateActions();

        for (let i = 0, len = sharedActions.length; i < len; i++) {
            const actionName = sharedActions[i];
            const appletAction = Plasmoid.internalAction(actionName);
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
        if (folderView.url !== Plasmoid.configuration.url) {
            folderView.url = Qt.binding(() => Plasmoid.configuration.url);
            folderView.history = [];
            folderView.updateHistory();
        }
    }

    Binding {
        target: Plasmoid
        property: "title"
        value: labelGenerator.displayLabel
        restoreMode: Binding.RestoreBinding
    }

    Folder.LabelGenerator {
        id: labelGenerator

        folderModel: folderView.model
        rtl: (Qt.application.layoutDirection === Qt.RightToLeft)
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
        textFormat: Text.PlainText

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
    }

    FolderView {
        id: folderView

        anchors.left: parent.left
        anchors.top: parent.top
        anchors.topMargin: folderViewLayerComponent.label !== null ? folderViewLayerComponent.label.height : 0
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        focus: true
        isRootView: isContainment
        positionerApplet: Plasmoid

        url: Plasmoid.configuration.url
        locked: (Plasmoid.configuration.locked || !isContainment || lockedByKiosk)
        filterMode: Plasmoid.configuration.filterMode
        filterPattern: Plasmoid.configuration.filterPattern
        filterMimeTypes: Plasmoid.configuration.filterMimeTypes
        showHiddenFiles: Plasmoid.configuration.showHiddenFiles

        flow: (Plasmoid.configuration.arrangement === 0) ? GridView.FlowLeftToRight : GridView.FlowTopToBottom
        layoutDirection: (Plasmoid.configuration.alignment === 0) ? Qt.LeftToRight : Qt.RightToLeft

        onSortModeChanged: {
            Plasmoid.configuration.sortMode = sortMode;
        }

        Component.onCompleted: {
            folderView.sortMode = Plasmoid.configuration.sortMode;
        }
    }

    Component {
        id: labelComponent

        Item {
            id: label

            // If we bind height to visible, it will be invisible initially (since "visible"
            // propagates recursively) and that confuses the Label, hence the temp property.
            readonly property bool active: (Plasmoid.configuration.labelMode !== 0)

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
                    margins: Kirigami.Units.smallSpacing
                }
                height: parent.height

                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignTop
                elide: Text.ElideMiddle
                text: labelGenerator.displayLabel
                textFormat: Text.PlainText
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

                    visible: root.isPopup && folderView.url !== Plasmoid.configuration.url

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
            Plasmoid.contextualActions.push(modelAction)
            if (actionName === "emptyTrash") {
                Plasmoid.contextualActions.push(viewPropertiesAction)
            }
        }

        Plasmoid.contextualActions.push(actionSeparator);

        Plasmoid.contextualActionsAboutToShow.connect(updateContextualActions);
        Plasmoid.contextualActionsAboutToShow.connect(folderView.model.clearSelection);
    }
}
