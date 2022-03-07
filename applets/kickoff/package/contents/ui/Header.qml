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
import org.kde.plasma.plasmoid 2.0

PlasmaExtras.PlasmoidHeading {
    id: root

    property alias searchText: searchField.text
    property Item configureButton: configureButton
    property Item avatar: avatar
    property real preferredNameAndIconWidth: 0

    contentHeight: Math.max(searchField.implicitHeight, configureButton.implicitHeight)

    leftPadding: 0
    rightPadding: 0
    topPadding: Math.round((background.margins.top - background.inset.top) / 2.0)
    bottomPadding: background.margins.bottom + Math.round((background.margins.bottom - background.inset.bottom) / 2.0)

    leftInset: -Plasmoid.rootItem.backgroundMetrics.leftPadding
    rightInset: -Plasmoid.rootItem.backgroundMetrics.rightPadding
    topInset: -background.margins.top
    bottomInset: 0

    KCoreAddons.KUser {
        id: kuser
    }

    spacing: Plasmoid.rootItem.backgroundMetrics.spacing

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
            Accessible.name: i18n("Open user settings")
            leftPadding: PlasmaCore.Units.devicePixelRatio
            rightPadding: PlasmaCore.Units.devicePixelRatio
            topPadding: PlasmaCore.Units.devicePixelRatio
            bottomPadding: PlasmaCore.Units.devicePixelRatio
            contentItem: Kirigami.Avatar {
                source: kuser.faceIconUrl
                name: kuser.fullName
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
            HoverHandler {
                id: hoverHandler
                cursorShape: Qt.PointingHandCursor
            }
            PC3.ToolTip.text: Accessible.name
            PC3.ToolTip.visible: hovered
            PC3.ToolTip.delay: Kirigami.Units.toolTipDelay

            Keys.onLeftPressed: if (LayoutMirroring.enabled) {
                searchField.forceActiveFocus(Qt.TabFocusReason)
            }
            Keys.onRightPressed: if (!LayoutMirroring.enabled) {
                searchField.forceActiveFocus(Qt.TabFocusReason)
            }
            Keys.onDownPressed: if (Plasmoid.rootItem.sideBar) {
                Plasmoid.rootItem.sideBar.forceActiveFocus(Qt.TabFocusReason)
            } else {
                Plasmoid.rootItem.contentArea.forceActiveFocus(Qt.TabFocusReason)
            }

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
                level: 4
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
        Keys.onDownPressed: Plasmoid.rootItem.contentArea.forceActiveFocus(Qt.TabFocusReason)

        PlasmaExtras.SearchField {
            id: searchField
            Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
            Layout.fillWidth: true
            Layout.leftMargin: Plasmoid.rootItem.backgroundMetrics.leftPadding
            focus: true

            Binding {
                target: Plasmoid.rootItem
                property: "searchField"
                value: searchField
            }
            Connections {
                target: Plasmoid.self
                function onExpandedChanged() {
                    if(!Plasmoid.expanded) {
                        searchField.clear()
                    }
                }
            }
            onTextEdited: {
                searchField.forceActiveFocus(Qt.ShortcutFocusReason)
            }
            onAccepted: {
                Plasmoid.rootItem.contentArea.currentItem.action.triggered()
                Plasmoid.rootItem.contentArea.currentItem.forceActiveFocus(Qt.ShortcutFocusReason)
            }
            Keys.priority: Keys.AfterItem
            Keys.forwardTo: Plasmoid.rootItem.contentArea.view
            Keys.onLeftPressed: if (activeFocus) {
                if (LayoutMirroring.enabled) {
                    nextItemInFocusChain().forceActiveFocus(Qt.TabFocusReason)
                } else {
                    nextItemInFocusChain(false).forceActiveFocus(Qt.BacktabFocusReason)
                }
            }
            Keys.onRightPressed: if (activeFocus) {
                if (!LayoutMirroring.enabled) {
                    nextItemInFocusChain().forceActiveFocus(Qt.TabFocusReason)
                } else {
                    nextItemInFocusChain(false).forceActiveFocus(Qt.BacktabFocusReason)
                }
            }
        }

        PC3.ToolButton {
            id: configureButton
            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
            visible: Plasmoid.action("configure").enabled
            icon.name: "configure"
            text: Plasmoid.action("configure").text
            display: PC3.ToolButton.IconOnly

            PC3.ToolTip.text: text
            PC3.ToolTip.delay: Kirigami.Units.toolTipDelay
            PC3.ToolTip.visible: hovered
            Keys.onLeftPressed: if (LayoutMirroring.enabled) {
                nextItemInFocusChain().forceActiveFocus(Qt.TabFocusReason)
            } else {
                nextItemInFocusChain(false).forceActiveFocus(Qt.BacktabFocusReason)
            }
            Keys.onRightPressed: if (!LayoutMirroring.enabled) {
                nextItemInFocusChain().forceActiveFocus(Qt.TabFocusReason)
            } else {
                nextItemInFocusChain(false).forceActiveFocus(Qt.BacktabFocusReason)
            }
            onClicked: Plasmoid.action("configure").trigger()
        }
        PC3.ToolButton {
            checkable: true
            checked: Plasmoid.configuration.pin
            icon.name: "window-pin"
            text: i18n("Keep Open")
            display: PC3.ToolButton.IconOnly
            PC3.ToolTip.text: text
            PC3.ToolTip.delay: Kirigami.Units.toolTipDelay
            PC3.ToolTip.visible: hovered
            Binding {
                target: Plasmoid.self
                property: "hideOnWindowDeactivate"
                value: !Plasmoid.configuration.pin
            }
            KeyNavigation.backtab: configureButton
            KeyNavigation.tab: if (Plasmoid.rootItem.sideBar) {
                return Plasmoid.rootItem.sideBar
            } else {
                return nextItemInFocusChain()
            }
            Keys.onLeftPressed: if (!LayoutMirroring.enabled) {
                nextItemInFocusChain(false).forceActiveFocus(Qt.BacktabFocusReason)
            }
            Keys.onRightPressed: if (LayoutMirroring.enabled) {
                nextItemInFocusChain(false).forceActiveFocus(Qt.BacktabFocusReason)
            }
            onToggled: Plasmoid.configuration.pin = checked
        }
    }
}
