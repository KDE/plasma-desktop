/*
    SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.0
import QtQuick.Layouts 1.0
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.plasma.core 2.0 as PlasmaCore


PlasmaCore.Dialog {
    id: contextMenu
    visualParent: settingsButton
    location: plasmoid.location
    type: PlasmaCore.Dialog.PopupMenu
    flags: Qt.Popup | Qt.FramelessWindowHint | Qt.WindowDoesNotAcceptFocus
    mainItem: ColumnLayout {
        id: menuColumn
        Layout.minimumWidth: menuColumn.implicitWidth
        Layout.minimumHeight: menuColumn.implicitHeight
        spacing: PlasmaCore.Units.smallSpacing

        LayoutMirroring.enabled: Qt.application.layoutDirection === Qt.RightToLeft
        LayoutMirroring.childrenInherit: true

        PlasmaComponents.ToolButton {
            Layout.fillWidth: true
            // we want destructive actions to be far from the initial cursor
            // position, so show this on the top unless it's on a top panel
            visible: location !== PlasmaCore.Types.TopEdge
            text: i18nd("plasma_shell_org.kde.plasma.desktop", "Remove Panel")
            iconSource: "delete"
            onClicked: plasmoid.action("remove").trigger();
        }

        PlasmaExtras.Heading {
            level: 3
            text: i18nd("plasma_shell_org.kde.plasma.desktop", "Panel Alignment")
        }
        PlasmaComponents.ButtonColumn {
            spacing: 0
            Layout.fillWidth: true
            PlasmaComponents.ToolButton {
                anchors {
                    left: parent.left
                    right: parent.right
                }
                text: panel.formFactor === PlasmaCore.Types.Vertical ? i18nd("plasma_shell_org.kde.plasma.desktop", "Top") : i18nd("plasma_shell_org.kde.plasma.desktop", "Left")
                checkable: true
                checked: panel.alignment === Qt.AlignLeft
                onClicked: panel.alignment = Qt.AlignLeft
                flat: false
            }
            PlasmaComponents.ToolButton {
                anchors {
                    left: parent.left
                    right: parent.right
                }
                text: i18nd("plasma_shell_org.kde.plasma.desktop", "Center")
                checkable: true
                checked: panel.alignment === Qt.AlignCenter
                onClicked: panel.alignment = Qt.AlignCenter
                flat: false
            }
            PlasmaComponents.ToolButton {
                anchors {
                    left: parent.left
                    right: parent.right
                }
                text: panel.formFactor === PlasmaCore.Types.Vertical ? i18nd("plasma_shell_org.kde.plasma.desktop", "Bottom") : i18nd("plasma_shell_org.kde.plasma.desktop", "Right")
                checkable: true
                checked: panel.alignment === Qt.AlignRight
                onClicked: panel.alignment = Qt.AlignRight
                flat: false
            }
        }

        PlasmaExtras.Heading {
            level: 3
            text: i18nd("plasma_shell_org.kde.plasma.desktop", "Visibility")
        }
        PlasmaComponents.ButtonColumn {
            spacing: 0
            Layout.fillWidth: true
            Layout.minimumWidth: implicitWidth
            PlasmaComponents.ToolButton {
                width: Math.max(implicitWidth, parent.width)
                text: i18nd("plasma_shell_org.kde.plasma.desktop", "Always Visible")
                checkable: true
                checked: configDialog.visibilityMode === 0
                onClicked: configDialog.visibilityMode = 0
                flat: false
            }
            PlasmaComponents.ToolButton {
                width: Math.max(implicitWidth, parent.width)
                text: i18nd("plasma_shell_org.kde.plasma.desktop", "Auto Hide")
                checkable: true
                checked: configDialog.visibilityMode === 1
                onClicked: configDialog.visibilityMode = 1
                flat: false
            }
            PlasmaComponents.ToolButton {
                width: Math.max(implicitWidth, parent.width)
                text: i18nd("plasma_shell_org.kde.plasma.desktop", "Windows Can Cover")
                checkable: true
                checked: configDialog.visibilityMode === 2
                onClicked: configDialog.visibilityMode = 2
                flat: false
            }
            PlasmaComponents.ToolButton {
                width: Math.max(implicitWidth, parent.width)
                text: i18nd("plasma_shell_org.kde.plasma.desktop", "Windows Go Below")
                checkable: true
                checked: configDialog.visibilityMode === 3
                onClicked: configDialog.visibilityMode = 3
                flat: false
            }
        }
        PlasmaExtras.Heading {
            level: 3
            text: i18nd("plasma_shell_org.kde.plasma.desktop", "Opacity")
            visible: panel.adaptiveOpacityEnabled
        }
        PlasmaComponents.ButtonColumn {
            spacing: 0
            visible: panel.adaptiveOpacityEnabled
            Layout.fillWidth: true
            Layout.minimumWidth: implicitWidth
            PlasmaComponents.ToolButton {
                width: Math.max(implicitWidth, parent.width)
                text: i18nd("plasma_shell_org.kde.plasma.desktop", "Adaptive")
                checkable: true
                checked: configDialog.opacityMode === 0
                onClicked: configDialog.opacityMode = 0
                flat: false
            }
            PlasmaComponents.ToolButton {
                width: Math.max(implicitWidth, parent.width)
                text: i18nd("plasma_shell_org.kde.plasma.desktop", "Opaque")
                checkable: true
                checked: configDialog.opacityMode === 1
                onClicked: configDialog.opacityMode = 1
                flat: false
            }
            PlasmaComponents.ToolButton {
                width: Math.max(implicitWidth, parent.width)
                text: i18nd("plasma_shell_org.kde.plasma.desktop", "Translucent")
                checkable: true
                checked: configDialog.opacityMode === 2
                onClicked: configDialog.opacityMode = 2
                flat: false
            }
        }
        PlasmaComponents.ToolButton {
            Layout.fillWidth: true
            text: i18nd("plasma_shell_org.kde.plasma.desktop", "Maximize Panel")
            iconSource: panel.formFactor === PlasmaCore.Types.Vertical ? "zoom-fit-height" : "zoom-fit-width"
            onClicked: panel.maximize();
        }
        PlasmaComponents.ToolButton {
            Layout.fillWidth: true
            // we want destructive actions to be far from the initial cursor
            // position, so show this on the bottom for top panels
            visible: location === PlasmaCore.Types.TopEdge
            text: i18nd("plasma_shell_org.kde.plasma.desktop", "Remove Panel")
            iconSource: "delete"
            onClicked: plasmoid.action("remove").trigger();
        }
    }

    function hide() {
        visible = false;
    }

    Component.onCompleted: {
        dialogRoot.closeContextMenu.connect(hide);
    }
}
