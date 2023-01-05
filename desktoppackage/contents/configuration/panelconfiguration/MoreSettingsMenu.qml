/*
    SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.0
import QtQuick.Layouts 1.0
import QtQuick.Controls 2.15 as QQC2

import org.kde.plasma.components 3.0 as PC3
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.shell.panel 0.1 as Panel
import org.kde.kquickcontrols 2.0


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
        readonly property int radioButtonGroupSpacing: PlasmaCore.Units.smallSpacing * 4

        LayoutMirroring.enabled: Qt.application.layoutDirection === Qt.RightToLeft
        LayoutMirroring.childrenInherit: true

        PC3.ToolButton {
            Layout.fillWidth: true
            // we want destructive actions to be far from the initial cursor
            // position, so show this on the top unless it's on a top panel
            visible: location !== PlasmaCore.Types.TopEdge
            text: i18nd("plasma_shell_org.kde.plasma.desktop", "Remove Panel")
            icon.name: "delete"
            onClicked: plasmoid.action("remove").trigger();
        }

        PlasmaExtras.Heading {
            Layout.topMargin: menuColumn.radioButtonGroupSpacing
            level: 3
            text: i18nd("plasma_shell_org.kde.plasma.desktop", "Panel Alignment")
        }

        QQC2.ButtonGroup {
            buttons: alignmentButtons.children
        }

        Column {
            id: alignmentButtons
            spacing: PlasmaCore.Units.smallSpacing * 2
            Layout.fillWidth: true
            PC3.RadioButton {
                anchors {
                    left: parent.left
                    right: parent.right
                }
                text: panel.formFactor === PlasmaCore.Types.Vertical ? i18nd("plasma_shell_org.kde.plasma.desktop", "Top") : i18nd("plasma_shell_org.kde.plasma.desktop", "Left")
                checkable: true
                checked: panel.alignment === Qt.AlignLeft
                onClicked: panel.alignment = Qt.AlignLeft
            }
            PC3.RadioButton {
                anchors {
                    left: parent.left
                    right: parent.right
                }
                text: i18nd("plasma_shell_org.kde.plasma.desktop", "Center")
                checkable: true
                checked: panel.alignment === Qt.AlignCenter
                onClicked: panel.alignment = Qt.AlignCenter
            }
            PC3.RadioButton {
                anchors {
                    left: parent.left
                    right: parent.right
                }
                text: panel.formFactor === PlasmaCore.Types.Vertical ? i18nd("plasma_shell_org.kde.plasma.desktop", "Bottom") : i18nd("plasma_shell_org.kde.plasma.desktop", "Right")
                checkable: true
                checked: panel.alignment === Qt.AlignRight
                onClicked: panel.alignment = Qt.AlignRight
            }
        }

        PlasmaExtras.Heading {
            Layout.topMargin: menuColumn.radioButtonGroupSpacing
            level: 3
            text: i18nd("plasma_shell_org.kde.plasma.desktop", "Visibility")
        }

        QQC2.ButtonGroup {
            buttons: visibilityButtons.children
        }

        Column {
            id: visibilityButtons
            spacing: PlasmaCore.Units.smallSpacing * 2
            Layout.fillWidth: true
            Layout.minimumWidth: implicitWidth
            PC3.RadioButton {
                width: Math.max(implicitWidth, parent.width)
                text: i18nd("plasma_shell_org.kde.plasma.desktop", "Always Visible")
                checkable: true
                checked: configDialog.visibilityMode === Panel.Global.NormalPanel
                onClicked: configDialog.visibilityMode = Panel.Global.NormalPanel
            }
            PC3.RadioButton {
                width: Math.max(implicitWidth, parent.width)
                text: i18nd("plasma_shell_org.kde.plasma.desktop", "Auto Hide")
                checkable: true
                checked: configDialog.visibilityMode === Panel.Global.AutoHide
                onClicked: configDialog.visibilityMode = Panel.Global.AutoHide
            }
            PC3.RadioButton {
                width: Math.max(implicitWidth, parent.width)
                text: i18nd("plasma_shell_org.kde.plasma.desktop", "Windows Can Cover")
                checkable: true
                checked: configDialog.visibilityMode === Panel.Global.LetWindowsCover
                onClicked: configDialog.visibilityMode = Panel.Global.LetWindowsCover
            }
            PC3.RadioButton {
                width: Math.max(implicitWidth, parent.width)
                text: i18nd("plasma_shell_org.kde.plasma.desktop", "Windows Go Below")
                checkable: true
                checked: configDialog.visibilityMode === Panel.Global.WindowsGoBelow
                onClicked: configDialog.visibilityMode = Panel.Global.WindowsGoBelow
            }
        }
        PlasmaExtras.Heading {
            Layout.topMargin: menuColumn.radioButtonGroupSpacing
            level: 3
            text: i18nd("plasma_shell_org.kde.plasma.desktop", "Opacity")
            visible: panel.adaptiveOpacityEnabled
        }

        QQC2.ButtonGroup {
            buttons: opacityButtons.children
        }

        Column {
            id: opacityButtons
            spacing: PlasmaCore.Units.smallSpacing * 2
            visible: panel.adaptiveOpacityEnabled
            Layout.fillWidth: true
            Layout.minimumWidth: implicitWidth
            Layout.bottomMargin: menuColumn.radioButtonGroupSpacing
            PC3.RadioButton {
                width: Math.max(implicitWidth, parent.width)
                text: i18nd("plasma_shell_org.kde.plasma.desktop", "Adaptive")
                checkable: true
                checked: configDialog.opacityMode === Panel.Global.Adaptive
                onClicked: configDialog.opacityMode = Panel.Global.Adaptive
            }
            PC3.RadioButton {
                width: Math.max(implicitWidth, parent.width)
                text: i18nd("plasma_shell_org.kde.plasma.desktop", "Opaque")
                checkable: true
                checked: configDialog.opacityMode === Panel.Global.Opaque
                onClicked: configDialog.opacityMode = Panel.Global.Opaque
            }
            PC3.RadioButton {
                width: Math.max(implicitWidth, parent.width)
                text: i18nd("plasma_shell_org.kde.plasma.desktop", "Translucent")
                checkable: true
                checked: configDialog.opacityMode === Panel.Global.Translucent
                onClicked: configDialog.opacityMode = Panel.Global.Translucent
            }
        }
        PC3.ToolButton {
            Layout.fillWidth: true
            text: i18nd("plasma_shell_org.kde.plasma.desktop", "Maximize Panel")
            icon.name: panel.formFactor === PlasmaCore.Types.Vertical ? "zoom-fit-height" : "zoom-fit-width"
            onClicked: panel.maximize();
        }
        PC3.CheckBox {
            Layout.fillWidth: true
            Layout.topMargin: mainItem.radioButtonGroupSpacing
            Layout.bottomMargin: mainItem.radioButtonGroupSpacing
            text: i18nd("plasma_shell_org.kde.plasma.desktop", "Floating Panel")
            icon.name: "zoom-select"
            onToggled: panel.floating = !panel.floating
            checked: panel.floating
        }
        PC3.ToolButton {
            Layout.fillWidth: true
            // we want destructive actions to be far from the initial cursor
            // position, so show this on the bottom for top panels
            visible: location === PlasmaCore.Types.TopEdge
            text: i18nd("plasma_shell_org.kde.plasma.desktop", "Remove Panel")
            icon.name: "delete"
            onClicked: plasmoid.action("remove").trigger();
        }
        PlasmaExtras.Heading {
            level: 3
            text: i18nd("plasma_shell_org.kde.plasma.desktop", "Shortcut")
            visible: panel.adaptiveOpacityEnabled
        }
        KeySequenceItem {
            id: button
            keySequence: plasmoid.globalShortcut
            onCaptureFinished: {
                plasmoid.globalShortcut = button.keySequence
            }
        }
        PC3.Label {
            Layout.fillWidth: true
            Layout.maximumWidth: PlasmaCore.Units.gridUnit * 6
            text: i18n("Press this keyboard shortcut to move focus to the Panel")
            font: PlasmaCore.Theme.smallestFont
            wrapMode: Text.Wrap
        }
    }

    function hide() {
        visible = false;
    }

    Component.onCompleted: {
        dialogRoot.closeContextMenu.connect(hide);
    }
}
