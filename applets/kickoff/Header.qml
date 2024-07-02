/*
    SPDX-FileCopyrightText: 2014 Sebastian Kügler <sebas@kde.org>
    SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>
    SPDX-FileCopyrightText: 2021 Mikel Johnson <mikel5764@gmail.com>
    SPDX-FileCopyrightText: 2021 Noah Davis <noahadvs@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Templates as T
import Qt5Compat.GraphicalEffects
import org.kde.plasma.components as PC3
import org.kde.plasma.extras as PlasmaExtras
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.components as KirigamiComponents
import org.kde.coreaddons as KCoreAddons
import org.kde.kcmutils as KCM
import org.kde.config as KConfig
import org.kde.plasma.plasmoid

PlasmaExtras.PlasmoidHeading {
    id: root

    property alias searchText: searchField.text
    property Item configureButton: configureButton
    property Item pinButton: pinButton
    property Item avatar: avatar
    property real preferredNameAndIconWidth: 0

    contentHeight: layoutContainer.height
        + kickoff.backgroundMetrics.topPadding
        + kickoff.backgroundMetrics.bottomPadding

    KCoreAddons.KUser {
        id: kuser
    }

    spacing: kickoff.backgroundMetrics.spacing

    function tabSetFocus(event, invertedTarget, normalTarget) {
        // Set input focus depending on whether layout order matches focus chain order
        // normalTarget is optional
        const reason = event.key == Qt.Key_Tab ? Qt.TabFocusReason : Qt.BacktabFocusReason
        if (kickoff.paneSwap) {
            invertedTarget.forceActiveFocus(reason)
        } else if (normalTarget !== undefined) {
            normalTarget.forceActiveFocus(reason)
        } else {
            event.accepted = false
        }
    }

    contentItem: Item {
        Item {
            id: layoutContainer

            height: Math.max(searchField.implicitHeight, configureButton.implicitHeight)
            anchors {
                verticalCenter: parent.verticalCenter
                left: parent.left
                leftMargin: kickoff.backgroundMetrics.leftPadding
                right: parent.right
                rightMargin: kickoff.backgroundMetrics.rightPadding
            }

            RowLayout {
                id: nameAndIcon
                spacing: root.spacing
                anchors.left: parent.left
                LayoutMirroring.enabled: kickoff.sideBarOnRight
                height: parent.height
                width: root.preferredNameAndIconWidth - layoutContainer.anchors.leftMargin

                KirigamiComponents.AvatarButton {
                    id: avatar
                    visible: KConfig.KAuthorized.authorizeControlModule("kcm_users")

                    Layout.fillHeight: true
                    Layout.minimumWidth: height
                    Layout.maximumWidth: height

                    text: i18n("Open user settings")
                    name: kuser.fullName

                    // The icon property emits two signals in a row during which it
                    // changes to an empty URL and probably back to the same
                    // static file path, so we need QtQuick.Image not to cache it.
                    cache: false
                    source: kuser.faceIconUrl

                    Keys.onTabPressed: event => {
                        tabSetFocus(event, kickoff.firstCentralPane);
                    }
                    Keys.onBacktabPressed: event => {
                        tabSetFocus(event, nextItemInFocusChain());
                    }
                    Keys.onLeftPressed: event => {
                        if (kickoff.sideBarOnRight) {
                            searchField.forceActiveFocus(Qt.application.layoutDirection == Qt.RightToLeft ? Qt.TabFocusReason : Qt.BacktabFocusReason)
                        }
                    }
                    Keys.onRightPressed: event => {
                        if (!kickoff.sideBarOnRight) {
                            searchField.forceActiveFocus(Qt.application.layoutDirection == Qt.RightToLeft ? Qt.BacktabFocusReason : Qt.TabFocusReason)
                        }
                    }
                    Keys.onDownPressed: event => {
                        if (kickoff.sideBar) {
                            kickoff.sideBar.forceActiveFocus(Qt.TabFocusReason)
                        } else {
                            kickoff.contentArea.forceActiveFocus(Qt.TabFocusReason)
                        }
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
                        textFormat: Text.PlainText
                        elide: Text.ElideRight
                        horizontalAlignment: kickoff.paneSwap ? Text.AlignRight : Text.AlignLeft
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
                        textFormat: Text.PlainText
                        elide: Text.ElideRight
                        horizontalAlignment: kickoff.paneSwap ? Text.AlignRight : Text.AlignLeft
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
                LayoutMirroring.enabled: kickoff.sideBarOnRight
                Keys.onDownPressed: event => {
                    kickoff.contentArea.forceActiveFocus(Qt.TabFocusReason);
                }

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
                            if (!kickoff.expanded) {
                                searchField.clear()
                            }
                        }
                    }
                    onTextEdited: {
                        searchField.forceActiveFocus(Qt.ShortcutFocusReason)
                    }
                    Keys.priority: Keys.AfterItem
                    Keys.forwardTo: kickoff.contentArea !== null ? kickoff.contentArea.view : []
                    Keys.onTabPressed: event => {
                        tabSetFocus(event, nextItemInFocusChain(false));
                    }
                    Keys.onBacktabPressed: event => {
                        tabSetFocus(event, nextItemInFocusChain());
                    }
                    Keys.onLeftPressed: event => {
                        if (activeFocus) {
                            nextItemInFocusChain(kickoff.sideBarOnRight).forceActiveFocus(
                                Qt.application.layoutDirection === Qt.RightToLeft ? Qt.TabFocusReason : Qt.BacktabFocusReason)
                        }
                    }
                    Keys.onRightPressed: event => {
                        if (activeFocus) {
                            nextItemInFocusChain(!kickoff.sideBarOnRight).forceActiveFocus(
                                Qt.application.layoutDirection === Qt.RightToLeft ? Qt.BacktabFocusReason : Qt.TabFocusReason)
                        }
                    }
                }

                PC3.ToolButton {
                    id: configureButton
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    visible: Plasmoid.internalAction("configure").enabled
                    icon.name: "configure"
                    text: Plasmoid.internalAction("configure").text
                    display: PC3.ToolButton.IconOnly

                    PC3.ToolTip.text: text
                    PC3.ToolTip.delay: Kirigami.Units.toolTipDelay
                    PC3.ToolTip.visible: hovered
                    Keys.onTabPressed: event => {
                        tabSetFocus(event, nextItemInFocusChain(false));
                    }
                    Keys.onBacktabPressed: event => {
                        tabSetFocus(event, nextItemInFocusChain());
                    }
                    Keys.onLeftPressed: event => {
                        nextItemInFocusChain(kickoff.sideBarOnRight).forceActiveFocus(
                            Qt.application.layoutDirection == Qt.RightToLeft ? Qt.TabFocusReason : Qt.BacktabFocusReason)
                    }
                    Keys.onRightPressed: event => {
                        nextItemInFocusChain(!kickoff.sideBarOnRight).forceActiveFocus(
                            Qt.application.layoutDirection == Qt.RightToLeft ? Qt.BacktabFocusReason : Qt.TabFocusReason)
                    }
                    onClicked: plasmoid.internalAction("configure").trigger()
                }
                PC3.ToolButton {
                    id: pinButton
                    checkable: true
                    checked: Plasmoid.configuration.pin
                    icon.name: "window-pin"
                    text: i18n("Keep Open")
                    display: PC3.ToolButton.IconOnly
                    PC3.ToolTip.text: text
                    PC3.ToolTip.delay: Kirigami.Units.toolTipDelay
                    PC3.ToolTip.visible: hovered
                    Binding {
                        target: kickoff
                        property: "hideOnWindowDeactivate"
                        value: !Plasmoid.configuration.pin
                        // there should be no other bindings, so don't waste resources
                        restoreMode: Binding.RestoreNone
                    }
                    Keys.onTabPressed: event => {
                        tabSetFocus(event, nextItemInFocusChain(false), kickoff.firstCentralPane || nextItemInFocusChain());
                    }
                    Keys.onBacktabPressed: event => {
                        tabSetFocus(event, nameAndIcon.nextItemInFocusChain(false), nextItemInFocusChain(false));
                    }
                    Keys.onLeftPressed: event => {
                        if (!kickoff.sideBarOnRight) {
                            nextItemInFocusChain(false).forceActiveFocus(Qt.application.layoutDirection == Qt.RightToLeft ? Qt.TabFocusReason : Qt.BacktabFocusReason)
                        }
                    }
                    Keys.onRightPressed: event => {
                        if (kickoff.sideBarOnRight) {
                            nextItemInFocusChain(false).forceActiveFocus(Qt.application.layoutDirection == Qt.RightToLeft ? Qt.BacktabFocusReason : Qt.TabFocusReason)
                        }
                    }
                    onToggled: Plasmoid.configuration.pin = checked
                }
            }
        }
    }
}
