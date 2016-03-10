/***************************************************************************
 *   Copyright (C) 2014-2015 by Eike Hein <hein@kde.org>                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

import QtQuick 2.0
import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents

import org.kde.private.desktopcontainment.folder 0.1 as Folder

Item {
    id: folderViewLayerComponent

    property variant sharedActions: ["newMenu", "paste", "undo", "refresh", "emptyTrash"]
    property Component folderViewDialogComponent: Qt.createComponent("FolderViewDialog.qml", Qt.Asynchronous, root)

    property Item view: folderView
    property Item label: null
    property int labelHeight: theme.mSize(theme.defaultFont).height
        + (root.isPopup ? (units.smallSpacing * 2) : 0)

    property alias model: folderView.model
    property alias overflowing: folderView.overflowing
    property alias flow: folderView.flow

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

    PlasmaCore.Svg {
        id: actionOverlays

        imagePath: "widgets/action-overlays"
        multipleImages: true
        size: "16x16"
    }

    // FIXME TODO: See https://codereview.qt-project.org/#/c/113758/
    Folder.TextFix {
        id: textFix
    }

    Folder.MenuHelper {
        id: menuHelper
    }

    Folder.ViewPropertiesMenu {
        id: viewPropertiesMenu

        showLayoutActions: !isPopup
        showLockAction: isContainment
        showIconSizeActions: !root.useListViewMode

        onArrangementChanged: {
            plasmoid.configuration.arrangement = arrangement;
        }

        onAlignmentChanged: {
            plasmoid.configuration.alignment = alignment;
        }

        onLockedChanged: {
            plasmoid.configuration.locked = locked;
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
            locked = plasmoid.configuration.locked;
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

        onExpandedChanged: {
            if (root.isPopup && !plasmoid.expanded) {
                if (folderView.url != plasmoid.configuration.url) {
                    folderView.url = plasmoid.configuration.url;
                }

                folderView.currentIndex = -1;
                folderView.model.clearSelection();
            }
        }

        onExternalData: {
            plasmoid.configuration.url = data
        }
    }

    Connections {
        target: plasmoid.configuration

        onArrangementChanged: {
            viewPropertiesMenu.arrangement = plasmoid.configuration.arrangement;
        }

        onAlignmentChanged: {
            viewPropertiesMenu.alignment = plasmoid.configuration.alignment;
        }

        onLockedChanged: {
            viewPropertiesMenu.locked = plasmoid.configuration.locked;
        }

        onSortModeChanged: {
            folderView.sortMode = plasmoid.configuration.sortMode;
            viewPropertiesMenu.sortMode = plasmoid.configuration.sortMode;
        }

        onSortDescChanged: {
            viewPropertiesMenu.sortDesc = plasmoid.configuration.sortDesc;
        }

        onSortDirsFirstChanged: {
            viewPropertiesMenu.sortDirsFirst = plasmoid.configuration.sortDirsFirst;
        }

        onIconSizeChanged: {
            viewPropertiesMenu.iconSize = plasmoid.configuration.iconSize;
        }

        onPositionsChanged: {
            folderView.positions = plasmoid.configuration.positions;
        }
    }

    FolderView {
        id: folderView

        anchors.left: parent.left
        anchors.top: parent.top
        anchors.topMargin: folderViewLayerComponent.label != null ? folderViewLayerComponent.label.height : 0
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        isRootView: true

        url: plasmoid.configuration.url
        locked: (plasmoid.configuration.locked || !isContainment)
        filterMode: plasmoid.configuration.filterMode
        filterPattern: plasmoid.configuration.filterPattern
        filterMimeTypes: plasmoid.configuration.filterMimeTypes

        flow: (plasmoid.configuration.arrangement == 0) ? GridView.FlowLeftToRight : GridView.FlowTopToBottom
        layoutDirection: (plasmoid.configuration.alignment == 0) ? Qt.LeftToRight : Qt.RightToLeft

        onSortModeChanged: {
            plasmoid.configuration.sortMode = sortMode;
        }

        onPositionsChanged: {
            plasmoid.configuration.positions = folderView.positions;
        }

        Component.onCompleted: {
            folderView.sortMode = plasmoid.configuration.sortMode;
            folderView.positions = plasmoid.configuration.positions;
        }
    }

    Component {
        id: labelComponent

        Item {
            id: label

            width: parent.width
            height: visible ? labelHeight : 0

            visible: (plasmoid.configuration.labelMode != 0)

            property Item windowPin: null

            onVisibleChanged: {
                if (root.isPopup && !visible) {
                    plasmoid.hideOnWindowDeactivate = true;
                }
            }

            Connections {
                target: root

                onIsPopupChanged: {
                    if (windowPin == null && root.isPopup) {
                        windowPin = windowPinComponent.createObject(label);
                    } else if (upButton != null) {
                        windowPin.destroy();
                    }
                }
            }

            PlasmaComponents.Label {
                id: text

                width: parent.width - (windowPin != null ? windowPin.width - units.smallSpacing : 0)
                height: parent.height

                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignTop
                elide: Text.ElideMiddle
                text: labelGenerator.displayLabel

                Binding {
                    target: plasmoid
                    property: "title"
                    value: labelGenerator.displayLabel
                }

                Folder.LabelGenerator {
                    id: labelGenerator

                    url: folderView.model.resolvedUrl
                    rtl: (Qt.application.layoutDirection == Qt.RightToLeft)
                    labelMode: plasmoid.configuration.labelMode
                    labelText: plasmoid.configuration.labelText
                }
            }

            MouseArea {
                anchors.fill: text

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

                    visible: root.isPopup

                    width: root.isPopup ? Math.round(units.gridUnit * 1.25) : 0
                    height: width
                    checkable: true
                    iconSource: "window-pin"
                    onCheckedChanged: plasmoid.hideOnWindowDeactivate = !checked
                }
            }

            Component.onCompleted: {
                if (root.isPopup) {
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
            plasmoid.setAction(actionName, modelAction.text, menuHelper.iconName(modelAction));

            if (actionName == "newMenu") {
                menuHelper.setMenu(plasmoid.action(actionName), folderView.model.newMenu);
                plasmoid.setActionSeparator("separator1");

                plasmoid.setAction("viewProperties", i18n("Icons"), "preferences-desktop-icons");
                menuHelper.setMenu(plasmoid.action("viewProperties"), viewPropertiesMenu.menu);
            } else {
                plasmoid.action(actionName).triggered.connect(modelAction.trigger);
            }
        }

        plasmoid.setActionSeparator("separator2");

        plasmoid.contextualActionsAboutToShow.connect(updateContextualActions);
    }
}
