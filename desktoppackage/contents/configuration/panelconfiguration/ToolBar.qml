/*
    SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Controls 2.5 as QQC2
import QtQuick.Layouts 1.0
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.components 3.0 as PlasmaComponents3
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.configuration 2.0
import org.kde.kirigami 2.0 as Kirigami

Item {
    id: root
    state: parent.state
    implicitWidth: Math.max(buttonsLayout_1.width, buttonsLayout_2.width, row.width) + PlasmaCore.Units.smallSpacing * 2
    implicitHeight: row.height + 20

    readonly property string addWidgetsButtonText: i18nd("plasma_shell_org.kde.plasma.desktop", "Add Widgets…")
    readonly property string addSpacerButtonText: i18nd("plasma_shell_org.kde.plasma.desktop", "Add Spacer")
    readonly property string settingsButtonText: i18nd("plasma_shell_org.kde.plasma.desktop", "More Options…")

    QQC2.Action {
        shortcut: "Escape"
        onTriggered: {
            // avoid leaving the panel in an inconsistent state when escaping while dragging it
            // "checked" means "pressed" in this case, we abuse that property to make the button look pressed
            if (edgeHandle.checked) {
                return
            }

            if (contextMenuLoader.opened) {
                contextMenuLoader.close()
            } else {
                configDialog.close()
            }
        }
    }

    GridLayout {
        id: buttonsLayout_1
        rows: 1
        columns: 1
        flow: plasmoid.formFactor === PlasmaCore.Types.Horizontal ? GridLayout.TopToBottom : GridLayout.LeftToRight

        anchors.margins: rowSpacing
        anchors.topMargin: plasmoid.formFactor === PlasmaCore.Types.Vertical ? rowSpacing + closeButton.height : rowSpacing

        property bool showText: plasmoid.formFactor === PlasmaCore.Types.Vertical || (row.x + row.width < root.width - placeHolder.width - PlasmaCore.Units.iconSizes.small*4 - PlasmaCore.Units.largeSpacing*5)

        rowSpacing: PlasmaCore.Units.smallSpacing
        columnSpacing: PlasmaCore.Units.smallSpacing

        PlasmaComponents.Button {
            text: buttonsLayout_2.showText ? root.addWidgetsButtonText : ""
            tooltip: buttonsLayout_2.showText ? "" : root.addWidgetsButtonText
            iconSource: "list-add"
            Layout.fillWidth: true
            onClicked: {
                configDialog.close();
                configDialog.showAddWidgetDialog();
            }
        }

        PlasmaComponents.Button {
            iconSource: "distribute-horizontal-x"
            text: buttonsLayout_2.showText ? root.addSpacerButtonText : ""
            tooltip: buttonsLayout_2.showText ? "" : root.addSpacerButtonText
            Layout.fillWidth: true
            onClicked: {
                configDialog.addPanelSpacer();
            }
        }
    }

    GridLayout {
        id: row
        columns: dialogRoot.vertical ? 1 : 4
        rows: dialogRoot.vertical ? 4 : 1
        anchors.centerIn: parent

        rowSpacing: PlasmaCore.Units.smallSpacing
        columnSpacing: PlasmaCore.Units.smallSpacing

        EdgeHandle {
            id: edgeHandle
            Layout.alignment: Qt.AlignHCenter
        }
        Item {
            Layout.preferredWidth: PlasmaCore.Units.gridUnit
            Layout.preferredHeight: PlasmaCore.Units.gridUnit
        }
        PlasmaComponents3.Label {
            Layout.fillWidth: true
            wrapMode: Text.Wrap

            text: panel.location === PlasmaCore.Types.LeftEdge || panel.location === PlasmaCore.Types.RightEdge ? i18nd("plasma_shell_org.kde.plasma.desktop", "Panel width:") : i18nd("plasma_shell_org.kde.plasma.desktop", "Panel height:")
        }
        PlasmaComponents3.SpinBox {
            Layout.fillWidth: true

            editable: true
            focus: true

            from: 20 // below this size, the panel is mostly unusable
            to: PlasmaCore.Types.LeftEdge || panel.location === PlasmaCore.Types.RightEdge ? panel.screenToFollow.geometry.width / 2 : panel.screenToFollow.geometry.height / 2

            stepSize: 2

            value: panel.thickness
            onValueModified: {
                panel.thickness = value
                // Adjust the position of the config bar too
                switch (panel.location) {
                    case PlasmaCore.Types.TopEdge:
                        configDialog.y = Qt.binding(function() { return panel.y + panel.thickness });
                        break;
                    case PlasmaCore.Types.LeftEdge:
                        configDialog.x = Qt.binding(function() { return panel.x + panel.thickness });
                        break;
                    case PlasmaCore.Types.RightEdge:
                        configDialog.x = Qt.binding(function() { return panel.x - configDialog.width });
                        break;
                    case PlasmaCore.Types.BottomEdge:
                    default:
                        configDialog.y = Qt.binding(function() { return panel.y - configDialog.height });
                        break;
                }
            }
            DragHandler {
                property int magnitude: PlasmaCore.Units.gridUnit
                target: null
                xAxis.enabled: panel.location == PlasmaCore.Types.LeftEdge || panel.location == PlasmaCore.Types.RightEdge
                yAxis.enabled: panel.location == PlasmaCore.Types.TopEdge || panel.location == PlasmaCore.Types.BottomEdge
                grabPermissions: PointerHandler.CanTakeOverFromAnything

                onTranslationChanged: {
                    var distance
                    switch (panel.location) {
                    case PlasmaCore.Types.TopEdge:
                        distance = translation.y
                        break;
                    case PlasmaCore.Types.LeftEdge:
                        distance = translation.x
                        break;
                    case PlasmaCore.Types.RightEdge:
                        distance = -translation.x
                        break;
                    case PlasmaCore.Types.BottomEdge:
                    default:
                        distance = -translation.y
                        break;
                    }
                    parent.value = parent.value + (Math.floor(distance / magnitude) * parent.stepSize)
                    parent.valueModified()
                }
            }
        }
    }

    PlasmaComponents.Label {
        id: placeHolder
        visible: false
        text: addWidgetsButtonText + addSpacerButtonText + settingsButtonText
    }

    Connections {
        target: configDialog
        function onVisibleChanged() {
            if (!configDialog.visible) {
                settingsButton.checked = false
            }
        }
    }

    GridLayout {
        id: buttonsLayout_2
        rows: 1
        columns: 1
        flow: plasmoid.formFactor === PlasmaCore.Types.Horizontal ? GridLayout.TopToBottom : GridLayout.LeftToRight

        anchors.margins: rowSpacing

        property bool showText: plasmoid.formFactor === PlasmaCore.Types.Vertical || (row.x + row.width < root.width - placeHolder.width - PlasmaCore.Units.iconSizes.small*4 - PlasmaCore.Units.largeSpacing*5)

        rowSpacing: PlasmaCore.Units.smallSpacing
        columnSpacing: PlasmaCore.Units.smallSpacing

        PlasmaComponents.Button {
            id: settingsButton
            iconSource: "configure"
            text: buttonsLayout_2.showText ? root.settingsButtonText : ""
            tooltip: buttonsLayout_2.showText ? "" : root.settingsButtonText
            Layout.fillWidth: true
            checkable: true
            onCheckedChanged: {
                if (checked) {
                    contextMenuLoader.open()
                } else {
                    contextMenuLoader.close()
                }
            }
        }

        PlasmaComponents.ToolButton {
            id: closeButton
            parent: plasmoid.formFactor === PlasmaCore.Types.Horizontal ? buttonsLayout_2 : root
            anchors.right: plasmoid.formFactor === PlasmaCore.Types.Horizontal ? undefined : parent.right
            iconSource: "window-close"
            tooltip: i18nd("plasma_shell_org.kde.plasma.desktop", "Close")
            onClicked: {
                configDialog.close()
            }
        }

        Loader {
            id: contextMenuLoader
            property bool opened: item && item.visible
            onOpenedChanged: settingsButton.checked = opened
            source: "MoreSettingsMenu.qml"
            active: false

            function open() {
                active = true
                item.visible = true
            }
            function close() {
                if (item) {
                    item.visible = false
                }
            }
        }

    }
//BEGIN States
    states: [
        State {
            name: "TopEdge"
            PropertyChanges {
                target: root
                height: root.implicitHeight
            }
            AnchorChanges {
                target: root
                anchors {
                    top: undefined
                    bottom: root.parent.bottom
                    left: root.parent.left
                    right: root.parent.right
                }
            }
            AnchorChanges {
                target: buttonsLayout_1
                anchors {
                    verticalCenter: root.verticalCenter
                    top: undefined
                    bottom: undefined
                    left: root.left
                    right: undefined
                }
            }
            AnchorChanges {
                target: buttonsLayout_2
                anchors {
                    verticalCenter: root.verticalCenter
                    top: undefined
                    bottom: undefined
                    left: undefined
                    right: root.right
                }
            }
            PropertyChanges {
                target: buttonsLayout_1
                width: buttonsLayout_1.implicitWidth
            }
            PropertyChanges {
                target: buttonsLayout_2
                width: buttonsLayout_2.implicitWidth
            }
        },
        State {
            name: "BottomEdge"
            PropertyChanges {
                target: root
                height: root.implicitHeight
            }
            AnchorChanges {
                target: root
                anchors {
                    top: root.parent.top
                    bottom: undefined
                    left: root.parent.left
                    right: root.parent.right
                }
            }
            AnchorChanges {
                target: buttonsLayout_1
                anchors {
                    verticalCenter: root.verticalCenter
                    top: undefined
                    bottom: undefined
                    left: root.left
                    right: undefined
                }
            }
            AnchorChanges {
                target: buttonsLayout_2
                anchors {
                    verticalCenter: root.verticalCenter
                    top: undefined
                    bottom: undefined
                    left: undefined
                    right: root.right
                }
            }
            PropertyChanges {
                target: buttonsLayout_1
                width: buttonsLayout_1.implicitWidth
            }
            PropertyChanges {
                target: buttonsLayout_2
                width: buttonsLayout_2.implicitWidth
            }
        },
        State {
            name: "LeftEdge"
            PropertyChanges {
                target: root
                width: root.implicitWidth
            }
            AnchorChanges {
                target: root
                anchors {
                    top: root.parent.top
                    bottom: root.parent.bottom
                    left: undefined
                    right: root.parent.right
                }
            }
            AnchorChanges {
                target: buttonsLayout_1
                anchors {
                    verticalCenter: undefined
                    top: root.top
                    bottom: undefined
                    left: root.left
                    right: root.right
                }
            }
            AnchorChanges {
                target: buttonsLayout_2
                anchors {
                    verticalCenter: undefined
                    top: undefined
                    bottom: root.bottom
                    left: root.left
                    right: root.right
                }
            }
        },
        State {
            name: "RightEdge"
            PropertyChanges {
                target: root
                width: root.implicitWidth
            }
            AnchorChanges {
                target: root
                anchors {
                    top: root.parent.top
                    bottom: root.parent.bottom
                    left: root.parent.left
                    right: undefined
                }
            }
            AnchorChanges {
                target: buttonsLayout_1
                anchors {
                    verticalCenter: undefined
                    top: root.top
                    bottom: undefined
                    left: root.left
                    right: root.right
                }
            }
            AnchorChanges {
                target: buttonsLayout_2
                anchors {
                    verticalCenter: undefined
                    top: undefined
                    bottom: root.bottom
                    left: root.left
                    right: root.right
                }
            }
        }
    ]
//END States
}
