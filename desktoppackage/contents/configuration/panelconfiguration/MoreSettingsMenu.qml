/*
    SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Layouts 1.0

import org.kde.plasma.components 3.0 as PC3
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.ksvg 1.0 as KSvg
import org.kde.plasma.shell.panel 0.1 as Panel
import org.kde.kquickcontrols 2.0
import org.kde.kirigami 2.20 as Kirigami


PlasmaCore.Dialog {
    id: contextMenu
    visualParent: settingsButton
    location: plasmoid.location
    type: PlasmaCore.Dialog.PopupMenu
    flags: Qt.Popup | Qt.FramelessWindowHint | Qt.WindowDoesNotAcceptFocus
    mainItem: ColumnLayout {
        id: menuColumn

        LayoutMirroring.enabled: Qt.application.layoutDirection === Qt.RightToLeft
        LayoutMirroring.childrenInherit: true
        spacing: Kirigami.Units.gridUnit

        readonly property int headingLevel: 2

        PlasmaExtras.PlasmoidHeading {

            RowLayout {
                anchors.fill: parent
                spacing: Kirigami.Units.gridUnit

                Kirigami.Heading {
                    level: 3
                    text: i18nd("plasma_shell_org.kde.plasma.desktop", "Panel Settings")
                }

                Item { Layout.fillWidth: true }

                PC3.ToolButton {
                    text: i18ndc("plasma_shell_org.kde.plasma.desktop", "@action:button Make the panel as big as it can be", "Maximize")
                    icon.name: panel.formFactor === PlasmaCore.Types.Vertical ? "zoom-fit-height" : "zoom-fit-width"

                    PC3.ToolTip.text: panel.formFactor === PlasmaCore.Types.Vertical
                        ? i18nd("plasma_shell_org.kde.plasma.desktop", "Make this panel as tall as possible")
                        : i18nd("plasma_shell_org.kde.plasma.desktop", "Make this panel as wide as possible")
                    PC3.ToolTip.delay: Kirigami.Units.toolTipDelay
                    PC3.ToolTip.visible: hovered

                    onClicked: panel.maximize();
                }

                PC3.ToolButton {
                    text: i18ndc("plasma_shell_org.kde.plasma.desktop", "@action:button Delete the panel", "Delete")
                    icon.name: "delete"

                    PC3.ToolTip.text: i18nd("plasma_shell_org.kde.plasma.desktop", "Remove this panel; this action is undo-able")
                    PC3.ToolTip.delay: Kirigami.Units.toolTipDelay
                    PC3.ToolTip.visible: hovered

                    onClicked: plasmoid.internalAction("remove").trigger();
                }

            }
        }

        RowLayout {
            spacing: Kirigami.Units.gridUnit
            Layout.leftMargin: Kirigami.Units.gridUnit
            Layout.rightMargin: Kirigami.Units.gridUnit

            Item {
                Layout.preferredWidth: menuColumn.width / 5
                Layout.fillHeight: true
                Layout.rightMargin: Kirigami.Units.gridUnit
                Kirigami.Heading {
                    anchors.right: parent.right
                    anchors.top: parent.top
                    anchors.topMargin: referenceRepresentation.screenHeight - height / 2
                    level: menuColumn.headingLevel
                    text: i18nd("plasma_shell_org.kde.plasma.desktop", "Alignment:")
                }
            }

            PanelRepresentation {
                id: referenceRepresentation
                text: panel.formFactor === PlasmaCore.Types.Vertical ? i18nd("plasma_shell_org.kde.plasma.desktop", "Top") : i18nd("plasma_shell_org.kde.plasma.desktop", "Left")
                tooltip: panel.formFactor === PlasmaCore.Types.Vertical ? i18nd("plasma_shell_org.kde.plasma.desktop", "Aligns a non-maximized panel to the top; no effect when panel is maximized") : i18nd("plasma_shell_org.kde.plasma.desktop", "Aligns a non-maximized panel to the left; no effect when panel is maximized")
                Layout.alignment: Qt.AlignTop
                alignment: Qt.AlignLeft
                checked: panel.alignment === Qt.AlignLeft
                onClicked: panel.alignment = Qt.AlignLeft
            }

            PanelRepresentation {
                text: i18nd("plasma_shell_org.kde.plasma.desktop", "Center")
                tooltip: i18nd("plasma_shell_org.kde.plasma.desktop", "Aligns a non-maximized panel to the center; no effect when panel is maximized")
                Layout.alignment: Qt.AlignTop
                alignment: Qt.AlignCenter
                checked: panel.alignment === Qt.AlignCenter
                onClicked: panel.alignment = Qt.AlignCenter
            }

            PanelRepresentation {
                text: panel.formFactor === PlasmaCore.Types.Vertical ? i18nd("plasma_shell_org.kde.plasma.desktop", "Bottom") : i18nd("plasma_shell_org.kde.plasma.desktop", "Right")
                tooltip: panel.formFactor === PlasmaCore.Types.Vertical ? i18nd("plasma_shell_org.kde.plasma.desktop", "Aligns a non-maximized panel to the bottom; no effect when panel is maximized") : i18nd("plasma_shell_org.kde.plasma.desktop", "Aligns a non-maximized panel to the right; no effect when panel is maximized")
                Layout.alignment: Qt.AlignTop
                alignment: Qt.AlignRight
                checked: panel.alignment === Qt.AlignRight
                onClicked: panel.alignment = Qt.AlignRight
            }


        }
        KSvg.SvgItem {
            Layout.fillWidth: true
            imagePath: "widgets/line"
            elementId: "horizontal-line"
        }
        RowLayout {
            spacing: Kirigami.Units.gridUnit
            Layout.leftMargin: Kirigami.Units.gridUnit
            Layout.rightMargin: Kirigami.Units.gridUnit

            Item {
                Layout.preferredWidth: menuColumn.width / 5
                Layout.rightMargin: Kirigami.Units.gridUnit
                Layout.fillHeight: true
                Kirigami.Heading {
                    anchors.right: parent.right
                    anchors.top: parent.top
                    anchors.topMargin: referenceRepresentation.screenHeight - height / 2
                    level: menuColumn.headingLevel
                    text: i18nd("plasma_shell_org.kde.plasma.desktop", "Visibility:")
                }
            }

            PanelRepresentation {
                text: i18nd("plasma_shell_org.kde.plasma.desktop", "Always Visible")
                Layout.alignment: Qt.AlignTop
                alignment: panel.alignment
                checked: configDialog.visibilityMode === Panel.Global.NormalPanel
                onClicked: configDialog.visibilityMode = Panel.Global.NormalPanel
            }

            PanelRepresentation {
                text: i18nd("plasma_shell_org.kde.plasma.desktop", "Auto-Hide")
                tooltip: i18nd("plasma_shell_org.kde.plasma.desktop", "Panel is hidden, but reveals itself when the cursor touches the panel's screen edge")
                Layout.alignment: Qt.AlignTop
                alignment: panel.alignment
                sunkenPanel: true
                checked: configDialog.visibilityMode === Panel.Global.AutoHide
                onClicked: configDialog.visibilityMode = Panel.Global.AutoHide
            }
        }
        KSvg.SvgItem {
            Layout.fillWidth: true
            imagePath: "widgets/line"
            elementId: "horizontal-line"
        }
        RowLayout {
            spacing: Kirigami.Units.gridUnit
            Layout.leftMargin: Kirigami.Units.gridUnit
            Layout.rightMargin: Kirigami.Units.gridUnit

            Item {
                Layout.preferredWidth: menuColumn.width / 5
                Layout.rightMargin: Kirigami.Units.gridUnit
                Layout.fillHeight: true
                Kirigami.Heading {
                    anchors.right: parent.right
                    anchors.top: parent.top
                    anchors.topMargin: referenceRepresentation.screenHeight - height / 2
                    level: menuColumn.headingLevel
                    text: i18nd("plasma_shell_org.kde.plasma.desktop", "Opacity:")
                }
            }

            PanelRepresentation {
                text: i18nd("plasma_shell_org.kde.plasma.desktop", "Always Opaque")
                Layout.alignment: Qt.AlignTop
                alignment: panel.alignment
                checked: configDialog.opacityMode === Panel.Global.Opaque
                onClicked: configDialog.opacityMode = Panel.Global.Opaque
            }

            PanelRepresentation {
                text: i18nd("plasma_shell_org.kde.plasma.desktop", "Adaptive")
                tooltip: i18nd("plasma_shell_org.kde.plasma.desktop", "Panel is opaque when any windows are touching it, and translucent at other times")
                Layout.alignment: Qt.AlignTop
                alignment: panel.alignment
                adaptivePanel: true
                checked: configDialog.opacityMode === Panel.Global.Adaptive
                onClicked: configDialog.opacityMode = Panel.Global.Adaptive
            }

            PanelRepresentation {
                text: i18nd("plasma_shell_org.kde.plasma.desktop", "Always Translucent")
                Layout.alignment: Qt.AlignTop
                alignment: panel.alignment
                translucentPanel: true
                checked: configDialog.opacityMode === Panel.Global.Translucent
                onClicked: configDialog.opacityMode = Panel.Global.Translucent
            }
        }
        KSvg.SvgItem {
            Layout.fillWidth: true
            imagePath: "widgets/line"
            elementId: "horizontal-line"
        }
        RowLayout {
            spacing: Kirigami.Units.gridUnit
            Layout.leftMargin: Kirigami.Units.gridUnit
            Layout.rightMargin: Kirigami.Units.gridUnit

            Item {
                Layout.preferredWidth: menuColumn.width / 5
                Layout.rightMargin: Kirigami.Units.gridUnit
                Layout.fillHeight: true
                Kirigami.Heading {
                    anchors.right: parent.right
                    anchors.top: parent.top
                    anchors.topMargin: referenceRepresentation.screenHeight - height / 2
                    level: menuColumn.headingLevel
                    text: i18nd("plasma_shell_org.kde.plasma.desktop", "Floating:")
                }
            }

            PanelRepresentation {
                text: i18nd("plasma_shell_org.kde.plasma.desktop", "Floating")
                tooltip: i18nd("plasma_shell_org.kde.plasma.desktop", "Panel visibly floats away from its screen edge")
                Layout.alignment: Qt.AlignTop
                alignment: panel.alignment
                checked: panel.floating
                floatingGap: Kirigami.Units.smallSpacing
                onClicked: panel.floating = true
            }

            PanelRepresentation {
                text: i18nd("plasma_shell_org.kde.plasma.desktop", "Attached")
                tooltip: i18nd("plasma_shell_org.kde.plasma.desktop", "Panel is attached to its screen edge")
                Layout.alignment: Qt.AlignTop
                alignment: panel.alignment
                checked: !panel.floating
                floatingGap: 0
                onClicked: panel.floating = false
            }
        }

        PlasmaExtras.PlasmoidHeading {
            location: PlasmaExtras.PlasmoidHeading.Footer
            Layout.topMargin: Kirigami.Units.smallSpacing
            topPadding: Kirigami.Units.smallSpacing * 2
            bottomPadding: Kirigami.Units.smallSpacing

            Layout.fillWidth: true
            RowLayout {
                anchors.centerIn: parent
                spacing: Kirigami.Units.gridUnit

                Kirigami.Heading {
                    level: menuColumn.headingLevel
                    text: i18nd("plasma_shell_org.kde.plasma.desktop", "Focus Shortcut:")
                    visible: panel.adaptiveOpacityEnabled

                    MouseArea {
                        id: mouseArea
                        anchors.fill: parent
                        hoverEnabled: true
                    }

                    PC3.ToolTip {
                        text: i18nd("plasma_shell_org.kde.plasma.desktop", "Press this keyboard shortcut to move focus to the Panel")
                        visible: mouseArea.containsMouse
                    }
                }

                KeySequenceItem {
                    id: button
                    keySequence: plasmoid.globalShortcut
                    onCaptureFinished: {
                        plasmoid.globalShortcut = button.keySequence
                    }
                }
            }
        }

    }

    function hide() {
        visible = false;
    }

    Component.onCompleted: {
        dialogRoot.closeContextMenu.connect(hide);
    }
}
