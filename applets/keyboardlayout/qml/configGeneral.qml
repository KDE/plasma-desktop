/*
    SPDX-FileCopyrightText: 2021 Andrey Butirsky <butirsky@gmail.com>
    SPDX-FileCopyrightText: 2022 Nate Graham <nate@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2

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

    Kirigami.Form {
        Kirigami.FormGroup {
            QQC2.ButtonGroup { id: displayButtonGroup}
            Kirigami.FormEntry {
                title: i18nc("@title:group of radio buttons, options are language codes or images", "Display style:")
                contentItem:  QQC2.RadioButton {
                    id: showLabel
                    QQC2.ButtonGroup.group: displayButtonGroup
                    text: root.displayName.length > 0 ? root.displayName: root.layoutShortName
                    checked: root.cfg_displayStyle === 0
                    onToggled: root.cfg_displayStyle = 0;
                }
            }

            Kirigami.FormEntry {
                contentItem: QQC2.RadioButton {
                    id: showFlag
                    QQC2.ButtonGroup.group: displayButtonGroup
                    checked: root.cfg_displayStyle === 1
                    onToggled: root.cfg_displayStyle = 1;
                    contentItem: RowLayout {
                        id: flagMissing
                        spacing: Kirigami.Units.smallSpacing
                        anchors.left: parent.left
                        anchors.leftMargin: showFlag.indicator.width + showFlag.spacing
                        anchors.verticalCenter: parent.verticalCenter

                        // Deliberately using this instead of Image to preserve visual fidelity
                        // with what the widget will show
                        Kirigami.Icon {
                            id: flagImage
                            implicitWidth: Kirigami.Units.iconSizes.medium
                            implicitHeight: Kirigami.Units.iconSizes.medium
                            source: KCMKeyboard.Flags.getIcon(root.layoutShortName)
                            visible: valid
                        }

                        Kirigami.Icon {
                            implicitWidth: Kirigami.Units.iconSizes.smallMedium
                            implicitHeight: Kirigami.Units.iconSizes.smallMedium
                            source: "emblem-warning"
                            visible: !flagImage.visible

                        }
                        QQC2.Label {
                            text: i18nc("@info:placeholder Make this translation as short as possible", "No flag available")
                            visible: !flagImage.visible

                        }
                    }
                }
            }

            Kirigami.FormEntry {
                contentItem: QQC2.RadioButton {
                    id: showLabelOverFlag
                    QQC2.ButtonGroup.group: displayButtonGroup
                    checked: root.cfg_displayStyle === 2
                    onToggled: root.cfg_displayStyle = 2;
                    contentItem: Kirigami.Icon {
                        // Deliberately using this instead of Image to preserve visual fidelity
                        // with what the widget will show
                        id: labelOverFlagImage
                        width: Kirigami.Units.iconSizes.medium
                        height: Kirigami.Units.iconSizes.medium
                        anchors.left: parent.left
                        anchors.leftMargin: showLabelOverFlag.indicator.width + showLabelOverFlag.spacing
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

            Kirigami.FormSeparator { }

            Kirigami.FormAction {
                action: Kirigami.Action {
                    text: i18nc("@action:button opens kcm_keyboard", "Configure Layouts…")
                    icon.name: "preferences-desktop-keyboard"
                    onTriggered: KCMLauncher.openSystemSettings("kcm_keyboard", "--tab=layouts")
                }
            }

        }
    }
}
