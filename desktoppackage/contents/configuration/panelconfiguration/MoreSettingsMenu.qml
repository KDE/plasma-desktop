/*
    SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15
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
        width: PlasmaCore.Units.gridUnit * 30
        height: PlasmaCore.Units.gridUnit * 43

        LayoutMirroring.enabled: Qt.application.layoutDirection === Qt.RightToLeft
        LayoutMirroring.childrenInherit: true
        spacing: PlasmaCore.Units.largeSpacing

        readonly property int headingLabel: 2

        PlasmaExtras.PlasmoidHeading {

            RowLayout {
                anchors.fill: parent
                PlasmaExtras.Heading {
                    level: 3
                    text: i18nd("plasma_shell_org.kde.plasma.desktop", " Panel Settings")
                }

                Item { Layout.fillWidth: true }

                PC3.ToolButton {
                    text: i18nd("plasma_shell_org.kde.plasma.desktop", "Remove Panel")
                    icon.name: "delete"
                    onClicked: plasmoid.action("remove").trigger();
                }

            }
        }

        RowLayout {
            spacing: PlasmaCore.Units.largeSpacing
            Layout.leftMargin: PlasmaCore.Units.largeSpacing
            Layout.rightMargin: PlasmaCore.Units.largeSpacing

            Item {
                Layout.preferredWidth: menuColumn.width / 5
                Layout.fillHeight: true
                Layout.rightMargin: PlasmaCore.Units.largeSpacing
                PlasmaExtras.Heading {
                    anchors.right: parent.right
                    anchors.top: parent.top
                    anchors.topMargin: referenceRepresentation.screenHeight - height / 2
                    level: menuColumn.headingLabel
                    Layout.topMargin: PlasmaCore.Units.smallSpacing
                    Layout.bottomMargin: PlasmaCore.Units.largeSpacing
                    text: i18nd("plasma_shell_org.kde.plasma.desktop", "Alignment:")
                }
            }

            PanelRepresentation {
                id: referenceRepresentation
                text: panel.formFactor === PlasmaCore.Types.Vertical ? i18nd("plasma_shell_org.kde.plasma.desktop", "Top") : i18nd("plasma_shell_org.kde.plasma.desktop", "Left")
                tooltip: panel.formFactor === PlasmaCore.Types.Vertical ? i18nd("plasma_shell_org.kde.plasma.desktop", "Aligns the panel to the top if the panel is not maximized.") : i18nd("plasma_shell_org.kde.plasma.desktop", "Aligns the panel
                to the left if the panel is not maximized.")
                Layout.alignment: Qt.AlignTop
                alignment: Qt.AlignLeft
                checked: panel.alignment === Qt.AlignLeft
                onClicked: panel.alignment = Qt.AlignLeft
            }

            PanelRepresentation {
                text: i18nd("plasma_shell_org.kde.plasma.desktop", "Center")
                tooltip: i18nd("plasma_shell_org.kde.plasma.desktop", "Center aligns the panel if the panel is not maximized.")
                Layout.alignment: Qt.AlignTop
                alignment: Qt.AlignCenter
                checked: panel.alignment === Qt.AlignCenter
                onClicked: panel.alignment = Qt.AlignCenter
            }

            PanelRepresentation {
                text: panel.formFactor === PlasmaCore.Types.Vertical ? i18nd("plasma_shell_org.kde.plasma.desktop", "Bottom") : i18nd("plasma_shell_org.kde.plasma.desktop", "Right")
                tooltip: panel.formFactor === PlasmaCore.Types.Vertical ? i18nd("plasma_shell_org.kde.plasma.desktop", "Aligns the panel to the bottom if the panel is not maximized.") : i18nd("plasma_shell_org.kde.plasma.desktop", "Aligns the panel to the right if the panel is not maximized.")
                Layout.alignment: Qt.AlignTop
                alignment: Qt.AlignRight
                checked: panel.alignment === Qt.AlignRight
                onClicked: panel.alignment = Qt.AlignRight
            }


        }
        PlasmaCore.SvgItem {
            Layout.fillWidth: true
            elementId: "horizontal-line"
            svg: PlasmaCore.Svg {
                id: lineSvg
                imagePath: "widgets/line"
            }
        }
        RowLayout {
            spacing: PlasmaCore.Units.largeSpacing
            Layout.leftMargin: PlasmaCore.Units.largeSpacing
            Layout.rightMargin: PlasmaCore.Units.largeSpacing

            Item {
                Layout.preferredWidth: menuColumn.width / 5
                Layout.rightMargin: PlasmaCore.Units.largeSpacing
                Layout.fillHeight: true
                PlasmaExtras.Heading {
                    anchors.right: parent.right
                    anchors.top: parent.top
                    anchors.topMargin: referenceRepresentation.screenHeight - height / 2
                    level: menuColumn.headingLabel
                    Layout.topMargin: PlasmaCore.Units.smallSpacing
                    Layout.bottomMargin: PlasmaCore.Units.largeSpacing
                    text: i18nd("plasma_shell_org.kde.plasma.desktop", "Visibility:")
                }
            }

            PanelRepresentation {
                text: i18nd("plasma_shell_org.kde.plasma.desktop", "Always Visible")
                tooltip: i18nd("plasma_shell_org.kde.plasma.desktop", "Makes the panel remain visible always.")
                Layout.alignment: Qt.AlignTop
                alignment: panel.alignment
                checked: configDialog.visibilityMode === Panel.Global.NormalPanel
                onClicked: configDialog.visibilityMode = Panel.Global.NormalPanel
            }

            PanelRepresentation {
                text: i18nd("plasma_shell_org.kde.plasma.desktop", "Auto Hide")
                tooltip: i18nd("plasma_shell_org.kde.plasma.desktop", "Makes the panel hidden always but reveals it when mouse enters the area where the panel would have been if it were not hidden.")
                Layout.alignment: Qt.AlignTop
                alignment: panel.alignment
                sunkenPanel: true
                checked: configDialog.visibilityMode === Panel.Global.AutoHide
                onClicked: configDialog.visibilityMode = Panel.Global.AutoHide
            }
        }
        RowLayout {
            spacing: PlasmaCore.Units.largeSpacing
            Layout.leftMargin: PlasmaCore.Units.largeSpacing
            Layout.rightMargin: PlasmaCore.Units.largeSpacing

            Item {
                Layout.preferredWidth: menuColumn.width / 5
                Layout.rightMargin: PlasmaCore.Units.largeSpacing
            }

            PanelRepresentation {
                text: i18nd("plasma_shell_org.kde.plasma.desktop", "Windows In Front")
                tooltip: i18nd("plasma_shell_org.kde.plasma.desktop", "Makes the panel remain visible always but maximized windows shall cover it. It is revealed when mouse enters the area where the panel would have been if it were not covered.")
                Layout.alignment: Qt.AlignTop
                alignment: panel.alignment
                windowVisible: true
                windowZ: 1
                checked: configDialog.visibilityMode === Panel.Global.LetWindowsCover
                onClicked: configDialog.visibilityMode = Panel.Global.LetWindowsCover
            }

            PanelRepresentation {
                text: i18nd("plasma_shell_org.kde.plasma.desktop", "Windows Behind")
                tooltip: i18nd("plasma_shell_org.kde.plasma.desktop", "Makes the panel remain visible always but part of the maximized windows shall go below the panel as though the panel did not exist.")
                Layout.alignment: Qt.AlignTop
                alignment: panel.alignment
                windowVisible: true
                checked: configDialog.visibilityMode === Panel.Global.WindowsGoBelow
                onClicked: configDialog.visibilityMode = Panel.Global.WindowsGoBelow
            }
        }
        PlasmaCore.SvgItem {
            Layout.fillWidth: true
            elementId: "horizontal-line"
            svg: lineSvg
        }
        RowLayout {
            spacing: PlasmaCore.Units.largeSpacing
            Layout.leftMargin: PlasmaCore.Units.largeSpacing
            Layout.rightMargin: PlasmaCore.Units.largeSpacing

            Item {
                Layout.preferredWidth: menuColumn.width / 5
                Layout.rightMargin: PlasmaCore.Units.largeSpacing
                Layout.fillHeight: true
                PlasmaExtras.Heading {
                    anchors.right: parent.right
                    anchors.top: parent.top
                    anchors.topMargin: referenceRepresentation.screenHeight - height / 2
                    level: menuColumn.headingLabel
                    Layout.topMargin: PlasmaCore.Units.smallSpacing
                    Layout.bottomMargin: PlasmaCore.Units.largeSpacing
                    text: i18nd("plasma_shell_org.kde.plasma.desktop", "Opacity:")
                }
            }

            PanelRepresentation {
                text: i18nd("plasma_shell_org.kde.plasma.desktop", "Fully Opaque")
                tooltip: i18nd("plasma_shell_org.kde.plasma.desktop", "Makes the panel opaque always.")
                Layout.alignment: Qt.AlignTop
                alignment: panel.alignment
                checked: configDialog.opacityMode === Panel.Global.Opaque
                onClicked: configDialog.opacityMode = Panel.Global.Opaque
            }

            PanelRepresentation {
                text: i18nd("plasma_shell_org.kde.plasma.desktop", "Adaptive")
                tooltip: i18nd("plasma_shell_org.kde.plasma.desktop", "Makes the panel translucent except when some windows touch it.")
                Layout.alignment: Qt.AlignTop
                alignment: panel.alignment
                adaptivePanel: true
                checked: configDialog.opacityMode === Panel.Global.Adaptive
                onClicked: configDialog.opacityMode = Panel.Global.Adaptive
            }

            PanelRepresentation {
                text: i18nd("plasma_shell_org.kde.plasma.desktop", "Translucent")
                tooltip: i18nd("plasma_shell_org.kde.plasma.desktop", "Makes the panel translucent always.")
                Layout.alignment: Qt.AlignTop
                alignment: panel.alignment
                translucentPanel: true
                checked: configDialog.opacityMode === Panel.Global.Translucent
                onClicked: configDialog.opacityMode = Panel.Global.Translucent
            }
        }
        PlasmaCore.SvgItem {
            Layout.fillWidth: true
            elementId: "horizontal-line"
            svg: lineSvg
        }
        RowLayout {
            spacing: PlasmaCore.Units.largeSpacing
            Layout.leftMargin: PlasmaCore.Units.largeSpacing
            Layout.rightMargin: PlasmaCore.Units.largeSpacing

            Item {
                Layout.preferredWidth: menuColumn.width / 5
                Layout.rightMargin: PlasmaCore.Units.largeSpacing
                Layout.fillHeight: true
                PlasmaExtras.Heading {
                    anchors.right: parent.right
                    anchors.top: parent.top
                    anchors.topMargin: referenceRepresentation.screenHeight - height / 2
                    level: menuColumn.headingLabel
                    Layout.topMargin: PlasmaCore.Units.smallSpacing
                    Layout.bottomMargin: PlasmaCore.Units.largeSpacing
                    text: i18nd("plasma_shell_org.kde.plasma.desktop", "Floating:")
                }
            }

            PanelRepresentation {
                text: i18nd("plasma_shell_org.kde.plasma.desktop", "Floating")
                tooltip: i18nd("plasma_shell_org.kde.plasma.desktop", "Makes the panel float from the edge of the screen.")
                Layout.alignment: Qt.AlignTop
                alignment: panel.alignment
                checked: panel.floating
                floatingGap: PlasmaCore.Units.smallSpacing
                onClicked: panel.floating = true
            }

            PanelRepresentation {
                text: i18nd("plasma_shell_org.kde.plasma.desktop", "Attached")
                tooltip: i18nd("plasma_shell_org.kde.plasma.desktop", "Makes the panel remain attached to the edge of the screen.")
                Layout.alignment: Qt.AlignTop
                alignment: panel.alignment
                checked: !panel.floating
                floatingGap: 0
                onClicked: panel.floating = false
            }
        }

        PlasmaExtras.PlasmoidHeading {
            location: PlasmaExtras.PlasmoidHeading.Footer
            Layout.topMargin: PlasmaCore.Units.smallSpacing
            topPadding: PlasmaCore.Units.smallSpacing * 2
            bottomPadding: PlasmaCore.Units.smallSpacing

            Layout.fillWidth: true
            RowLayout {
                anchors.centerIn: parent
                spacing: PlasmaCore.Units.largeSpacing

                PlasmaExtras.Heading {
                    level: menuColumn.headingLabel
                    text: i18nd("plasma_shell_org.kde.plasma.desktop", "Focus Shortcut")
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
