/*
    SPDX-FileCopyrightText: 2024 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2014 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Effects
import QtQuick.Layouts

import org.kde.plasma.core as PlasmaCore
import org.kde.plasma.components as PC
import org.kde.kirigami 2.20 as Kirigami

import org.kde.kcmutils as KCM

Item {
    property real centerX: Math.round(editModeUi.x + editModeUi.width/2)
    property real centerY: Math.round(editModeUi.y + editModeUi.height/2)
    property real roundedRootWidth: Math.round(root.width)
    property real roundedRootHeight: Math.round(root.height)

    property bool open: false
    Component.onCompleted: {
        open = Qt.binding(() => {return containment.plasmoid.corona.editMode})
    }

    // Those 2 elements have the same parameters as the overview effect
    MultiEffect {
        source: containment
        anchors.fill: parent
        blurEnabled: true
        blurMax: 64
        blur: 1.0
    }
    Rectangle {
        anchors.fill: parent
        color: Kirigami.Theme.backgroundColor
        opacity: 0.7
    }

    Item {
        id: editModeUi
        visible: open || xAnim.running
        x: Math.round(open ? editModeRect.x + editModeRect.width/2 - zoomedWidth/2 : 0)
        y: Math.round(open ? editModeRect.y + editModeRect.height/2 - zoomedHeight/2 + toolBar.height/2 : 0)
        width: open ? zoomedWidth : roundedRootWidth
        height: open ? zoomedHeight : roundedRootHeight
        property real zoomedWidth: Math.round(root.width * containmentParent.scaleFactor)
        property real zoomedHeight: Math.round(root.height * containmentParent.scaleFactor)

        Kirigami.ShadowedRectangle {
            color: Kirigami.Theme.backgroundColor
            width: Math.round(parent.width)
            height: Math.round(parent.height + toolBar.height + Kirigami.Units.largeSpacing)
            y: - toolBar.height - Kirigami.Units.largeSpacing

            radius: open ? Kirigami.Units.cornerRadius : 0
            Behavior on radius {
                NumberAnimation {
                    duration: Kirigami.Units.longDuration
                    easing.type: Easing.InOutQuad
                }
            }

            shadow {
                size: Kirigami.Units.gridUnit * 2
                color: Qt.rgba(0, 0, 0, 0.3)
                yOffset: 3
            }
            RowLayout {
                id: toolBar
                LayoutMirroring.enabled: Qt.application.layoutDirection === Qt.RightToLeft
                LayoutMirroring.childrenInherit: true
                anchors {
                    left: parent.left
                    top: parent.top
                    right: parent.right
                    margins: Kirigami.Units.smallSpacing
                }
                Flow {
                    Layout.fillWidth: true
                    Layout.minimumHeight: implicitHeight
                    PC.ToolButton {
                        id: addWidgetButton
                        property QtObject qAction: containment?.plasmoid.internalAction("add widgets") || null
                        text: qAction?.text
                        icon.name: "list-add"
                        onClicked: qAction.trigger()
                    }

                    PC.ToolButton {
                        id: addPanelButton
                        height: addWidgetButton.height
                        property QtObject qAction: containment?.plasmoid.corona.action("add panel") || null
                        text: qAction?.text
                        icon.name: "list-add"
                        Accessible.role: Accessible.ButtonMenu
                        onClicked: containment.plasmoid.corona.showAddPanelContextMenu(mapToGlobal(0, height))
                    }

                    PC.ToolButton {
                        id: configureButton
                        property QtObject qAction: containment?.plasmoid.internalAction("configure") || null
                        text: qAction?.text
                        icon.name: "preferences-desktop-wallpaper"
                        onClicked: qAction.trigger()
                    }

                    PC.ToolButton {
                        id: themeButton
                        text: i18nd("plasma_shell_org.kde.plasma.desktop", "Global Themes")
                        icon.name: "preferences-desktop-theme-global"
                        onClicked: KCM.KCMLauncher.openSystemSettings("kcm_lookandfeel")
                    }

                    PC.ToolButton {
                        id: displaySettingsButton
                        text: i18nd("plasma_shell_org.kde.plasma.desktop", "Display Configuration")
                        icon.name: "preferences-desktop-display"
                        onClicked: KCM.KCMLauncher.openSystemSettings("kcm_kscreen")
                    }

                    PC.ToolButton {
                        id: manageContainmentsButton
                        property QtObject qAction: containment?.plasmoid.corona.action("manage-containments") || null
                        text: qAction?.text
                        visible: qAction?.visible || false
                        icon.name: "preferences-system-windows-effect-fadedesktop"
                        onClicked: qAction.trigger()
                    }
                }

                PC.ToolButton {
                    Layout.alignment: Qt.AlignTop

                    visible: Kirigami.Settings.hasTransientTouchInput || Kirigami.Settings.tabletMode

                    icon.name: "overflow-menu"
                    text: i18ndc("plasma_shell_org.kde.plasma.desktop", "@action:button", "More")

                    onClicked: {
                        containment.openContextMenu(mapToGlobal(0, height));
                    }
                }
                PC.ToolButton {
                    Layout.alignment: Qt.AlignTop
                    icon.name: "window-close"
                    text: i18nd("plasma_shell_org.kde.plasma.desktop", "Exit Edit Mode")
                    onClicked: containment.plasmoid.corona.editMode = false
                }
            }
        }

        Behavior on x {
            NumberAnimation {
                id: xAnim
                duration: Kirigami.Units.longDuration
                easing.type: Easing.InOutQuad
            }
        }
        Behavior on y {
            NumberAnimation {
                duration: Kirigami.Units.longDuration
                easing.type: Easing.InOutQuad
            }
        }
        Behavior on width {
            NumberAnimation {
                duration: Kirigami.Units.longDuration
                easing.type: Easing.InOutQuad
            }
        }
        Behavior on height {
            NumberAnimation {
                duration: Kirigami.Units.longDuration
                easing.type: Easing.InOutQuad
            }
        }

        MultiEffect {
            anchors.fill: parent
            source: containment
            layer.enabled: true
            layer.smooth: true
            layer.effect: Kirigami.ShadowedTexture {
                width: roundedRootWidth
                height: roundedRootHeight
                color: "transparent"

                radius: open ? Kirigami.Units.cornerRadius : 0
                Behavior on radius {
                    NumberAnimation {
                        duration: Kirigami.Units.longDuration
                        easing.type: Easing.InOutQuad
                    }
                }
            }
        }
    }
}
