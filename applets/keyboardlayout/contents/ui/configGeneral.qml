/*
    SPDX-FileCopyrightText: 2021 Andrey Butirsky <butirsky@gmail.com>
    SPDX-FileCopyrightText: 2022 Nate Graham <nate@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15

import org.kde.kirigami 2.20 as Kirigami
import org.kde.plasma.workspace.keyboardlayout 1.0
import org.kde.plasma.workspace.components 2.0 as WorkspaceComponents
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
            Kirigami.FormData.label: i18n("Display style:")
            text: root.displayName.length > 0 ? root.displayName: root.layoutShortName
            checked: cfg_displayStyle === 0
            onToggled: cfg_displayStyle = 0;
        }

        RadioButton {
            id: showFlag
            checked: cfg_displayStyle === 1
            onToggled: cfg_displayStyle = 1;
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
                        text: i18nc("@info:placeholder Make this translation as short as possible", "No flag available")
                    }
                }
            }
        }

        RadioButton {
            id: showLabelOverFlag
            checked: cfg_displayStyle === 2
            onToggled: cfg_displayStyle = 2;
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
            Kirigami.FormData.label: i18n("Layouts:")
            text: i18n("Configure…")
            icon.name: "configure"
            onClicked: KCMLauncher.openSystemSettings("kcm_keyboard", "--tab=layouts")
        }

        Component.onCompleted: {
            // hide Keyboard Shortcuts tab
            var appletConfiguration = app
            while (appletConfiguration.parent) {
                appletConfiguration = appletConfiguration.parent
            }
            if (appletConfiguration && typeof appletConfiguration.globalConfigModel !== "undefined") {
                appletConfiguration.globalConfigModel.removeCategoryAt(0)
            }
        }
    }
}
