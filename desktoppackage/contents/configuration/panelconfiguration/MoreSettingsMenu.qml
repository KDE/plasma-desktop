/*
    SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.0
import QtQuick.Layouts 1.0
import QtQuick.Controls 2.12 as QQC2
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

        PlasmaExtras.PlasmoidHeading {
            contentItem: RowLayout {
                PlasmaExtras.Heading {
                    text: i18nd("plasma_shell_org.kde.plasma.desktop", "More Settings")

                    Layout.fillWidth: true
                }
                PlasmaComponents.ToolButton {
                    iconSource: panel.formFactor === PlasmaCore.Types.Vertical ? "zoom-fit-height" : "zoom-fit-width"
                    onClicked: panel.maximize();

                    QQC2.ToolTip {
                        text: i18nd("plasma_shell_org.kde.plasma.desktop", "Maximize Panel")
                        visible: parent.hovered
                    }
                }
                PlasmaComponents.ToolButton {
                    iconSource: "delete"
                    onClicked: plasmoid.action("remove").trigger();

                    QQC2.ToolTip {
                        visible: parent.hovered
                        text: i18nd("plasma_shell_org.kde.plasma.desktop", "Remove Panel")
                    }
                }
            }
        }

        ColumnLayout {
            PlasmaExtras.Heading {
                level: 3
                text: i18nd("plasma_shell_org.kde.plasma.desktop", "Panel Alignment")
            }
            QQC2.RadioButton {
                text: panel.formFactor === PlasmaCore.Types.Vertical ? i18nd("plasma_shell_org.kde.plasma.desktop", "Top") : i18nd("plasma_shell_org.kde.plasma.desktop", "Left")
                checked: panel.alignment === Qt.AlignLeft
                onClicked: panel.alignment = Qt.AlignLeft
            }
            QQC2.RadioButton {
                text: i18nd("plasma_shell_org.kde.plasma.desktop", "Center")
                checked: panel.alignment === Qt.AlignCenter
                onClicked: panel.alignment = Qt.AlignCenter
            }
            QQC2.RadioButton {
                text: panel.formFactor === PlasmaCore.Types.Vertical ? i18nd("plasma_shell_org.kde.plasma.desktop", "Bottom") : i18nd("plasma_shell_org.kde.plasma.desktop", "Right")
                checked: panel.alignment === Qt.AlignRight
                onClicked: panel.alignment = Qt.AlignRight
            }
        }

        ColumnLayout {
            Layout.topMargin: PlasmaCore.Units.smallSpacing
            Layout.bottomMargin: PlasmaCore.Units.smallSpacing

            PlasmaExtras.Heading {
                level: 3
                text: i18nd("plasma_shell_org.kde.plasma.desktop", "Visibility")
            }
            QQC2.RadioButton {
                text: i18nd("plasma_shell_org.kde.plasma.desktop", "Always Visible")
                checked: configDialog.visibilityMode === 0
                onClicked: configDialog.visibilityMode = 0
            }
            QQC2.RadioButton {
                text: i18nd("plasma_shell_org.kde.plasma.desktop", "Auto Hide")
                checked: configDialog.visibilityMode === 1
                onClicked: configDialog.visibilityMode = 1
            }
            QQC2.RadioButton {
                text: i18nd("plasma_shell_org.kde.plasma.desktop", "Windows Can Cover")
                checked: configDialog.visibilityMode === 2
                onClicked: configDialog.visibilityMode = 2
            }
            QQC2.RadioButton {
                text: i18nd("plasma_shell_org.kde.plasma.desktop", "Windows Go Below")
                checked: configDialog.visibilityMode === 3
                onClicked: configDialog.visibilityMode = 3
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
