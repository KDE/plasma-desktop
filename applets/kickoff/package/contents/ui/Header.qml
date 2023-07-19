/*
    SPDX-FileCopyrightText: 2014 Sebastian Kügler <sebas@kde.org>
    SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>
    SPDX-FileCopyrightText: 2021 Mikel Johnson <mikel5764@gmail.com>
    SPDX-FileCopyrightText: 2021 Noah Davis <noahadvs@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15
import QtQml 2.15
import QtQuick.Layouts 1.15
import QtQuick.Templates 2.15 as T
import Qt5Compat.GraphicalEffects
import org.kde.plasma.components 3.0 as PC3
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.kirigami 2.20 as Kirigami
import org.kde.coreaddons 1.0 as KCoreAddons
import org.kde.kcmutils as KCM
import org.kde.config as KConfig

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

    leftInset: -kickoff.backgroundMetrics.leftPadding
    rightInset: -kickoff.backgroundMetrics.rightPadding
    topInset: -background.margins.top
    bottomInset: 0

    KCoreAddons.KUser {
        id: kuser
    }

    spacing: kickoff.backgroundMetrics.spacing

    RowLayout {
        id: nameAndIcon
        spacing: root.spacing
        anchors.left: parent.left
        height: parent.height
        width: root.preferredNameAndIconWidth

        PC3.RoundButton {
            id: avatar
            visible: KConfig.KAuthorized.authorizeControlModule("kcm_users")
            hoverEnabled: true
            Layout.fillHeight: true
            Layout.minimumWidth: height
            Layout.maximumWidth: height
            // FIXME: Not using text with display because of RoundButton bugs in plasma-framework
            // See https://bugs.kde.org/show_bug.cgi?id=440022
            Accessible.name: i18n("Open user settings")
            leftPadding: 1
            rightPadding: 1
            topPadding: 1
            bottomPadding: 1
            contentItem: Kirigami.Avatar {
                source: kuser.faceIconUrl
                name: kuser.fullName
            }
            Rectangle {
                parent: avatar.background
                anchors.fill: avatar.background
                anchors.margins: -1
                z: 1
                radius: height/2
                color: "transparent"
                border.width: avatar.visualFocus ? 2 : 0
                border.color: Kirigami.Theme.buttonFocusColor
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
            Keys.onDownPressed: if (kickoff.sideBar) {
                kickoff.sideBar.forceActiveFocus(Qt.TabFocusReason)
            } else {
                kickoff.contentArea.forceActiveFocus(Qt.TabFocusReason)
            }

            onClicked: KCM.KCMLauncher.openSystemSettings("kcm_users")
        }

        MouseArea {
            id: nameAndInfoMouseArea
            hoverEnabled: true

            Layout.fillHeight: true
            Layout.fillWidth: true

            Kirigami.Heading {
                id: nameLabel
                anchors.fill: parent
                opacity: parent.containsMouse ? 0 : 1
                color: Kirigami.Theme.textColor
                level: 4
                text: kuser.fullName
                elide: Text.ElideRight
                horizontalAlignment: Text.AlignLeft
                verticalAlignment: Text.AlignVCenter

                Behavior on opacity {
                    NumberAnimation {
                        duration: Kirigami.Units.longDuration
                        easing.type: Easing.InOutQuad
                    }
                }
            }

            Kirigami.Heading {
                id: infoLabel
                anchors.fill: parent
                level: 5
                opacity: parent.containsMouse ? 1 : 0
                color: Kirigami.Theme.textColor
                text: kuser.os !== "" ? `${kuser.loginName}@${kuser.host} (${kuser.os})` : `${kuser.loginName}@${kuser.host}`
                elide: Text.ElideRight
                horizontalAlignment: Text.AlignLeft
                verticalAlignment: Text.AlignVCenter

                Behavior on opacity {
                    NumberAnimation {
                        duration: Kirigami.Units.longDuration
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
        Keys.onDownPressed: kickoff.contentArea.forceActiveFocus(Qt.TabFocusReason)

        PlasmaExtras.SearchField {
            id: searchField
            Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
            Layout.fillWidth: true
            Layout.leftMargin: kickoff.backgroundMetrics.leftPadding
            focus: true

            Binding {
                target: kickoff
                property: "searchField"
                value: searchField
                // there's only one header ever, so don't waste resources
                restoreMode: Binding.RestoreNone
            }
            Connections {
                target: kickoff
                function onExpandedChanged() {
                    if (kickoff.expanded) {
                        searchField.clear()
                    }
                }
            }
            onTextEdited: {
                searchField.forceActiveFocus(Qt.ShortcutFocusReason)
            }
            onAccepted: {
                kickoff.contentArea.currentItem.action.triggered()
                kickoff.contentArea.currentItem.forceActiveFocus(Qt.ShortcutFocusReason)
            }
            Keys.priority: Keys.AfterItem
            Keys.forwardTo: kickoff.contentArea !== null ? kickoff.contentArea.view : []
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
            visible: plasmoid.internalAction("configure").enabled
            icon.name: "configure"
            text: plasmoid.internalAction("configure").text
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
            onClicked: plasmoid.internalAction("configure").trigger()
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
                target: kickoff
                property: "hideOnWindowDeactivate"
                value: !plasmoid.configuration.pin
                // there should be no other bindings, so don't waste resources
                restoreMode: Binding.RestoreNone
            }
            KeyNavigation.backtab: configureButton
            KeyNavigation.tab: if (kickoff.sideBar) {
                return kickoff.sideBar
            } else {
                return nextItemInFocusChain()
            }
            Keys.onLeftPressed: if (!LayoutMirroring.enabled) {
                nextItemInFocusChain(false).forceActiveFocus(Qt.BacktabFocusReason)
            }
            Keys.onRightPressed: if (LayoutMirroring.enabled) {
                nextItemInFocusChain(false).forceActiveFocus(Qt.BacktabFocusReason)
            }
            onToggled: plasmoid.configuration.pin = checked
        }
    }
}
