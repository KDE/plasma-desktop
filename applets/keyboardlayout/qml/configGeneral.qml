/*
    SPDX-FileCopyrightText: 2021 Andrey Butirsky <butirsky@gmail.com>
    SPDX-FileCopyrightText: 2022 Nate Graham <nate@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import org.kde.kirigami as Kirigami
import org.kde.plasma.workspace.keyboardlayout
import org.kde.plasma.workspace.components as WorkspaceComponents
import org.kde.plasma.private.kcm_keyboard as KCMKeyboard
import org.kde.kcmutils

SimpleKCM {
    id: root

    property int cfg_displayStyle

    readonly property string layoutShortName: keyboardLayout.layoutsList.length ? keyboardLayout.layoutsList[keyboardLayout.layout].shortName
                                                                            : ""
    readonly property string displayName: keyboardLayout.layoutsList.length ? keyboardLayout.layoutsList[keyboardLayout.layout].displayName
                                                                            : ""
    KeyboardLayout { id: keyboardLayout }

    Kirigami.FormLayout {
        RadioButton {
            id: showLabel
            Kirigami.FormData.label: i18n("Display style:") // qmllint disable unqualified
            text: root.displayName.length > 0 ? root.displayName: root.layoutShortName
            checked: root.cfg_displayStyle === 0
            onToggled: root.cfg_displayStyle = 0;
        }

        RadioButton {
            id: showFlag
            checked: root.cfg_displayStyle === 1
            onToggled: root.cfg_displayStyle = 1;
            contentItem: Item {
                implicitWidth: childrenRect.width + showFlag.indicator.width
                implicitHeight: childrenRect.height

                // Deliberately using this instead of Image to preserve visual fidelity
                // with what the widget will show
                Kirigami.Icon {
                    id: flagImage
                    width: Kirigami.Units.iconSizes.medium
                    height: Kirigami.Units.iconSizes.medium
                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                    source: KCMKeyboard.Flags.getIcon(root.layoutShortName)
                    visible: valid
                }
                RowLayout {
                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                    spacing: Kirigami.Units.smallSpacing
                    width: visible ? implicitWidth : 0
                    height: visible ? implicitHeight : 0
                    visible: !flagImage.visible

                    Kirigami.Icon {
                        implicitWidth: Kirigami.Units.iconSizes.smallMedium
                        implicitHeight: Kirigami.Units.iconSizes.smallMedium
                        source: "emblem-warning"
                    }
                    Label {
                        text: i18nc("@info:placeholder Make this translation as short as possible", "No flag available") // qmllint disable unqualified
                    }
                }
            }
        }

        RadioButton {
            id: showLabelOverFlag
            checked: root.cfg_displayStyle === 2
            onToggled: root.cfg_displayStyle = 2;
            contentItem: Item {
                implicitWidth: childrenRect.width + showLabelOverFlag.indicator.width
                implicitHeight: childrenRect.height

                // Deliberately using this instead of Image to preserve visual fidelity
                // with what the widget will show
                Kirigami.Icon {
                    width: Kirigami.Units.iconSizes.medium
                    height: Kirigami.Units.iconSizes.medium
                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                    source: KCMKeyboard.Flags.getIcon(root.layoutShortName)
                    visible: valid

                    WorkspaceComponents.BadgeOverlay {
                        anchors.bottom: parent.bottom
                        anchors.right: parent.right

                        text: showLabel.text
                        icon: parent
                    }
                }
            }
        }

        Kirigami.Separator {
            Kirigami.FormData.isSection: true
        }

        Button {
            Kirigami.FormData.label: i18n("Layouts:") // qmllint disable unqualified
            text: i18n("Configure…") // qmllint disable unqualified
            icon.name: "configure"
            onClicked: KCMLauncher.openSystemSettings("kcm_keyboard", "--tab=layouts")
        }
    }
}
