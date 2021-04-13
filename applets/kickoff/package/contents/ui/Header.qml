/*
    SPDX-FileCopyrightText: 2014 Sebastian Kügler <sebas@kde.org>
    SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>
    SPDX-FileCopyrightText: 2021 Mikel Johnson <mikel5764@gmail.com>
    SPDX-FileCopyrightText: 2021 Noah Davis <noahadvs@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Templates 2.15 as T
import QtGraphicalEffects 1.15
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 3.0 as PC3
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.kirigami 2.13 as Kirigami
import org.kde.kcoreaddons 1.0 as KCoreAddons
import org.kde.kquickcontrolsaddons 2.0 as KQuickAddons

PlasmaExtras.PlasmoidHeading {
    id: root

    property alias searchText: searchField.text
    property Item configureButton: configureButton
    property Item avatar: avatar
    property real preferredNameAndIconWidth: 0

    contentHeight: Math.max(searchField.implicitHeight, configureButton.implicitHeight)

    leftPadding: 0
    rightPadding: 0
    topPadding: 0
    bottomPadding: background.margins.bottom

    leftInset: -KickoffSingleton.leftPadding
    rightInset: -KickoffSingleton.rightPadding
    topInset: -background.margins.top
    bottomInset: 0

    KCoreAddons.KUser {
        id: kuser
    }

    spacing: KickoffSingleton.spacing

    RowLayout {
        id: nameAndIcon
        spacing: root.spacing
        anchors.left: parent.left
        height: parent.height
        width: root.preferredNameAndIconWidth

        PC3.RoundButton {
            id: avatar
            visible: KQuickAddons.KCMShell.authorize("kcm_users.desktop").length > 0
            hoverEnabled: true
            Layout.fillHeight: true
            Layout.minimumWidth: height
            Layout.maximumWidth: height
            // FIXME: Not using text with display because of RoundButton bugs in plasma-framework
            // See https://bugs.kde.org/show_bug.cgi?id=440022
            Accessible.name: i18n("Go to user settings")
            leftPadding: PlasmaCore.Units.devicePixelRatio
            rightPadding: PlasmaCore.Units.devicePixelRatio
            topPadding: PlasmaCore.Units.devicePixelRatio
            bottomPadding: PlasmaCore.Units.devicePixelRatio
            contentItem: Loader {
                sourceComponent: kuser.faceIconUrl ? imageComponent : icon
                Component {
                    id: imageComponent
                    Image {
                        id: imageItem
                        anchors.fill: avatar.contentItem
                        source: kuser.faceIconUrl
                        smooth: true
                        sourceSize.width: avatar.contentItem.width
                        sourceSize.height: avatar.contentItem.height
                        fillMode: Image.PreserveAspectCrop
                    }
                }
                Component {
                    id: iconComponent
                    PlasmaCore.IconItem {
                        id: iconItem
                        anchors.fill: avatar.contentItem
                        source: "user"
                    }
                }
                layer.enabled: kuser.faceIconUrl
                layer.effect: OpacityMask {
                    anchors.fill: avatar.contentItem
                    source: avatar.contentItem
                    maskSource: Rectangle {
                        visible: false
                        radius: height/2
                        width: avatar.contentItem.width
                        height: avatar.contentItem.height
                    }
                }
            }
            Rectangle {
                parent: avatar.background
                anchors.fill: avatar.background
                anchors.margins: -PlasmaCore.Units.devicePixelRatio
                z: 1
                radius: height/2
                color: "transparent"
                border.width: avatar.visualFocus ? PlasmaCore.Units.devicePixelRatio * 2 : 0
                border.color: PlasmaCore.Theme.buttonFocusColor
            }
            // Only used to keep the exact circular shape consistent with the image.
            // Without this, it looks significantly worse.
            background.layer.enabled: kuser.faceIconUrl
            background.layer.effect: OpacityMask {
                anchors.fill: avatar.background
                source: avatar.background
                maskSource: Rectangle {
                    visible: false
                    radius: height/2
                    width: avatar.background.width
                    height: avatar.background.height
                }
            }
            HoverHandler {
                id: hoverHandler
                cursorShape: Qt.PointingHandCursor
            }
            PC3.ToolTip.text: Accessible.name
            PC3.ToolTip.visible: hovered
            PC3.ToolTip.delay: Kirigami.Units.toolTipDelay
            onClicked: KQuickAddons.KCMShell.openSystemSettings("kcm_users")
        }

        MouseArea {
            id: nameAndInfoMouseArea
            hoverEnabled: true

            Layout.fillHeight: true
            Layout.fillWidth: true

            PlasmaExtras.Heading {
                id: nameLabel
                anchors.fill: parent
                opacity: parent.containsMouse ? 0 : 1
                color: PlasmaCore.Theme.textColor
                level: 2
                text: kuser.fullName
                elide: Text.ElideRight
                horizontalAlignment: Text.AlignLeft
                verticalAlignment: Text.AlignVCenter

                Behavior on opacity {
                    NumberAnimation {
                        duration: PlasmaCore.Units.longDuration
                        easing.type: Easing.InOutQuad
                    }
                }
            }

            PlasmaExtras.Heading {
                id: infoLabel
                anchors.fill: parent
                level: 5
                opacity: parent.containsMouse ? 1 : 0
                color: PlasmaCore.Theme.textColor
                text: kuser.os !== "" ? `${kuser.loginName}@${kuser.host} (${kuser.os})` : `${kuser.loginName}@${kuser.host}`
                elide: Text.ElideRight
                horizontalAlignment: Text.AlignLeft
                verticalAlignment: Text.AlignVCenter

                Behavior on opacity {
                    NumberAnimation {
                        duration: PlasmaCore.Units.longDuration
                        easing.type: Easing.InOutQuad
                    }
                }
            }

            PC3.ToolTip.text: infoLabel.text
            PC3.ToolTip.delay: Kirigami.Units.toolTipDelay
            PC3.ToolTip.visible: infoLabel.truncated && containsMouse
        }
    }
    RowLayout {
        id: rowLayout
        spacing: root.spacing
        height: parent.height
        anchors {
            left: nameAndIcon.right
            right: parent.right
        }
        PlasmaCore.SvgItem {
            id: separator
            Layout.fillHeight: true
            implicitWidth: naturalSize.width
            implicitHeight: 0
            elementId: "vertical-line"
            svg: KickoffSingleton.lineSvg
        }

        PC3.TextField {
            id: searchField
            Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
            Layout.fillWidth: true
            focus: true
            placeholderText: i18n("Search…")
            clearButtonShown: true
            Accessible.editable: true
            Accessible.searchEdit: true
            Binding {
                target: KickoffSingleton
                property: "searchField"
                value: searchField
            }
            Connections {
                target: plasmoid
                function onExpandedChanged() {
                    if(!plasmoid.expanded) {
                        searchField.clear()
                    }
                }
            }
            Shortcut {
                sequence: StandardKey.Find
                onActivated: {
                    searchField.forceActiveFocus(Qt.ShortcutFocusReason)
                    searchField.selectAll()
                }
            }
            onTextEdited: {
                searchField.forceActiveFocus(Qt.ShortcutFocusReason)
            }
            onAccepted: {
                KickoffSingleton.contentArea.currentItem.action.triggered()
                KickoffSingleton.contentArea.currentItem.forceActiveFocus(Qt.ShortcutFocusReason)
            }
            Keys.priority: Keys.AfterItem
            Keys.forwardTo: KickoffSingleton.contentArea.view
        }

        PC3.ToolButton {
            id: configureButton
            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
            visible: plasmoid.action("configure").enabled
            icon.name: "configure"
            text: plasmoid.action("configure").text
            display: PC3.ToolButton.IconOnly

            PC3.ToolTip.text: text
            PC3.ToolTip.delay: Kirigami.Units.toolTipDelay
            PC3.ToolTip.visible: hovered
            onClicked: plasmoid.action("configure").trigger()
        }
        PC3.ToolButton {
            checkable: true
            checked: plasmoid.configuration.pin
            icon.name: "window-pin"
            text: i18n("Keep Open")
            display: PC3.ToolButton.IconOnly
            PC3.ToolTip.text: text
            PC3.ToolTip.delay: Kirigami.Units.toolTipDelay
            PC3.ToolTip.visible: hovered
            Binding {
                target: plasmoid
                property: "hideOnWindowDeactivate"
                value: !plasmoid.configuration.pin
            }
            KeyNavigation.backtab: configureButton
            KeyNavigation.tab: if (KickoffSingleton.sideBar) {
                return KickoffSingleton.sideBar
            } else {
                return nextItemInFocusChain()
            }
            onToggled: plasmoid.configuration.pin = checked
        }
    }
}
