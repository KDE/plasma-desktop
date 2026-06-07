/*
    SPDX-FileCopyrightText: 2014 Aleix Pol Gonzalez <aleixpol@blue-systems.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2

import org.kde.plasma.components as PlasmaComponents
import org.kde.coreaddons as KCoreAddons
import org.kde.kirigami as Kirigami

import org.kde.breeze.components
import "timer.js" as AutoTriggerTimer

import org.kde.plasma.private.sessions

Item {
    id: root
    Kirigami.Theme.inherit: false
    Kirigami.Theme.colorSet: Kirigami.Theme.Complementary
    height: screenGeometry.height
    width: screenGeometry.width

    signal logoutRequested()
    signal haltRequested()
    signal haltUpdateRequested()
    signal suspendRequested(int spdMethod)
    signal rebootRequested()
    signal rebootRequested2(int opt)
    signal rebootUpdateRequested()
    signal cancelRequested()
    signal lockScreenRequested()
    signal cancelSoftwareUpdateRequested()

    function sleepRequested() {
        root.suspendRequested(2);
    }

    function hibernateRequested() {
        root.suspendRequested(4);
    }

    property real timeout: 30
    property real remainingTime: root.timeout

    property var currentAction: {
        switch (sdtype) {
        case ShutdownType.ShutdownTypeReboot:
            return () => softwareUpdatePending ? rebootUpdateRequested() : rebootRequested();
        case ShutdownType.ShutdownTypeHalt:
            return () => softwareUpdatePending ? haltUpdateRequested() : haltRequested();
        default:
            return () => logoutRequested();
        }
    }

    readonly property bool showAllOptions: sdtype === ShutdownType.ShutdownTypeDefault

    KCoreAddons.KUser {
        id: kuser
    }

    // For showing an "other users are logged in" hint
    SessionsModel {
        id: otherSessionsModel

        // In case the default values ever change
        includeUnusedSessions: false
        includeOwnSession: false
    }

    QQC2.Action {
        onTriggered: root.cancelRequested()
        shortcut: "Escape"
    }

    onRemainingTimeChanged: {
        if (remainingTime <= 0) {
            (currentAction)();
        }
    }

    Timer {
        id: countDownTimer
        running: !showAllOptions
        repeat: true
        interval: 1000
        onTriggered: remainingTime--
        Component.onCompleted: {
            AutoTriggerTimer.addCancelAutoTriggerCallback(function() {
                countDownTimer.running = false;
            });
        }
    }

    Rectangle {
        id: backgroundRect
        anchors.fill: parent
        // Intentionally hardcoded because any other color looks terrible here.
        // Any bug reports about illegible text or icons should be considered
        // a color scheme error and sent back to the user or their distro.
        color: "black"
        opacity: 0.85
    }
    MouseArea {
        anchors.fill: parent
        onClicked: root.cancelRequested()
    }
    UserDelegate {
        width: Kirigami.Units.gridUnit * 8
        height: Kirigami.Units.gridUnit * 9
        anchors {
            horizontalCenter: parent.horizontalCenter
            bottom: parent.verticalCenter
        }
        constrainText: false
        avatarPath: kuser.faceIconUrl
        iconSource: "user-identity"
        isCurrent: true
        name: kuser.fullName
    }
    ColumnLayout {
        id: column

        anchors {
            top: parent.verticalCenter
            topMargin: Kirigami.Units.gridUnit * 2
            horizontalCenter: parent.horizontalCenter
        }
        spacing: Kirigami.Units.largeSpacing

        height: Math.max(implicitHeight, Kirigami.Units.gridUnit * 10)
        width: Math.max(implicitWidth, Kirigami.Units.gridUnit * 16)

        PlasmaComponents.Label {
            font.pointSize: Kirigami.Theme.defaultFont.pointSize + 1
            Layout.alignment: Qt.AlignHCenter
            //opacity, as visible would re-layout
            opacity: countDownTimer.running ? 1 : 0
            Behavior on opacity {
                OpacityAnimator {
                    duration: Kirigami.Units.longDuration
                    easing.type: Easing.InOutQuad
                }
            }
            text: {
                switch (sdtype) {
                    case ShutdownType.ShutdownTypeReboot:
                        return softwareUpdatePending ? i18ndp("plasma_shell_org.kde.plasma.desktop", "Installing software updates and restarting in 1 second", "Installing software updates and restarting in %1 seconds", root.remainingTime)
                        : i18ndp("plasma_shell_org.kde.plasma.desktop", "Restarting in 1 second", "Restarting in %1 seconds", root.remainingTime);
                    case ShutdownType.ShutdownTypeNone:
                        return i18ndp("plasma_shell_org.kde.plasma.desktop", "Logging out in 1 second", "Logging out in %1 seconds", root.remainingTime);
                    case ShutdownType.ShutdownTypeHalt:
                    default:
                        return softwareUpdatePending ? i18ndp("plasma_shell_org.kde.plasma.desktop", "Installing software updates and shutting down in 1 second", "Installing software updates and shutting down in %1 seconds", root.remainingTime)
                        : i18ndp("plasma_shell_org.kde.plasma.desktop", "Shutting down in 1 second", "Shutting down in %1 seconds", root.remainingTime);
                }
            }
            textFormat: Text.PlainText
        }

        PlasmaComponents.Label {
            font.pointSize: Kirigami.Theme.defaultFont.pointSize + 1
            Layout.maximumWidth: Math.max(Kirigami.Units.gridUnit * 16, logoutButtonsRow.implicitWidth)
            Layout.alignment: Qt.AlignHCenter
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WordWrap
            font.italic: true
            text: i18ndp("plasma_shell_org.kde.plasma.desktop",
                         "One other user is currently logged in. If the computer is shut down or restarted, that user may lose work.",
                         "%1 other users are currently logged in. If the computer is shut down or restarted, those users may lose work.",
                         otherSessionsModel.count)
            textFormat: Text.PlainText
            visible: otherSessionsModel.count > 0 && (sdtype !== ShutdownType.ShutdownTypeNone || root.showAllOptions)
        }

        PlasmaComponents.Label {
            font.pointSize: Kirigami.Theme.defaultFont.pointSize + 1
            Layout.maximumWidth: Math.max(Kirigami.Units.gridUnit * 16, logoutButtonsRow.implicitWidth)
            Layout.alignment: Qt.AlignHCenter
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WordWrap
            font.italic: true
            text: {
                if (isUefi) {
                    return i18nd("plasma_shell_org.kde.plasma.desktop", "When restarted, the computer will enter UEFI firmware settings.");
                } else {
                    return i18nd("plasma_shell_org.kde.plasma.desktop", "When restarted, the computer will enter the firmware setup screen.");
                }
            }
            textFormat: Text.PlainText
            visible: rebootToFirmwareSetup
        }

        PlasmaComponents.Label {
            font.pointSize: Kirigami.Theme.defaultFont.pointSize + 1
            Layout.maximumWidth: Math.max(Kirigami.Units.gridUnit * 16, logoutButtonsRow.implicitWidth)
            Layout.alignment: Qt.AlignHCenter
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WordWrap
            font.italic: true
            text: i18nd("plasma_shell_org.kde.plasma.desktop", "When restarted, the computer will enter the boot loader menu.")
            textFormat: Text.PlainText
            visible: rebootToBootLoaderMenu
        }

        PlasmaComponents.Label {
            font.pointSize: Kirigami.Theme.defaultFont.pointSize + 1
            Layout.maximumWidth: Math.max(Kirigami.Units.gridUnit * 16, logoutButtonsRow.implicitWidth)
            Layout.alignment: Qt.AlignHCenter
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WordWrap
            font.italic: true
            text: i18ndc("plasma_shell_org.kde.plasma.desktop", "%1 is a boot loader entry", "When restarted, the computer will switch to %1.", rebootToBootLoaderEntry)
            textFormat: Text.PlainText
            visible: rebootToBootLoaderEntry != ""
        }

        GridLayout {
            id: logoutButtonsRow

            readonly property int spacing: Kirigami.Units.largeSpacing
            rowSpacing: spacing
            columnSpacing: spacing

            Layout.topMargin: Kirigami.Units.gridUnit * 2 - column.spacing
            Layout.alignment: Qt.AlignHCenter

            readonly property int buttonCount: visibleChildren.length
            readonly property int singleRowWidth: (children[0].implicitWidth * buttonCount) + (spacing * (buttonCount - 1))
            columns: singleRowWidth < root.width ? buttonCount : Math.ceil(buttonCount / 2)

            LogoutButton {
                id: suspendButton
                icon.name: "system-suspend"
                text: root.showAllOptions ? i18ndc("plasma_shell_org.kde.plasma.desktop", "Suspend to RAM", "Slee&p")
                                          : i18ndc("plasma_shell_org.kde.plasma.desktop", "Suspend to RAM", "Slee&p Now")
                onClicked: root.sleepRequested()
                KeyNavigation.left: cancelButton
                KeyNavigation.right: hibernateButton.visible ? hibernateButton : (rebootButton.visible ? rebootButton : (shutdownButton.visible ? shutdownButton : (logoutButton.visible ? logoutButton : cancelButton)))
                visible: spdMethods.SuspendState && root.showAllOptions
            }
            LogoutButton {
                id: hibernateButton
                icon.name: "system-suspend-hibernate"
                text: root.showAllOptions ? i18nd("plasma_shell_org.kde.plasma.desktop", "&Hibernate")
                                          : i18nd("plasma_shell_org.kde.plasma.desktop", "&Hibernate Now")
                onClicked: root.hibernateRequested()
                KeyNavigation.left: suspendButton.visible ? suspendButton : cancelButton
                KeyNavigation.right: rebootButton.visible ? rebootButton : (shutdownButton.visible ? shutdownButton : (logoutButton.visible ? logoutButton : cancelButton))
                visible: spdMethods.HibernateState && root.showAllOptions
            }
            LogoutButton {
                id: rebootButton
                icon.name: softwareUpdatePending ? "system-reboot-update" : "system-reboot"
                text: {
                    if (softwareUpdatePending) {
                        return i18ndc("plasma_shell_org.kde.plasma.desktop", "@action:button Keep short", "Install Updates and Restart")
                    } else {
                        return root.showAllOptions ? i18nd("plasma_shell_org.kde.plasma.desktop", "&Restart")
                                                   : i18nd("plasma_shell_org.kde.plasma.desktop", "&Restart Now")
                    }
                }
                onClicked: {
                    if (softwareUpdatePending) {
                        root.rebootUpdateRequested();
                    } else {
                        root.rebootRequested();
                    }
                }
                KeyNavigation.left: hibernateButton.visible ? hibernateButton : (suspendButton.visible ? suspendButton : cancelButton)
                KeyNavigation.right: rebootWithoutUpdatesButton.visible ? rebootWithoutUpdatesButton : (shutdownButton.visible ? shutdownButton : (logoutButton.visible ? logoutButton : cancelButton))
                focus: sdtype === ShutdownType.ShutdownTypeReboot
                visible: maysd && (sdtype === ShutdownType.ShutdownTypeReboot || root.showAllOptions)
            }
            LogoutButton {
                id: rebootWithoutUpdatesButton
                icon.name: "system-reboot"
                text: root.showAllOptions ? i18nd("plasma_shell_org.kde.plasma.desktop", "&Restart")
                                          : i18nd("plasma_shell_org.kde.plasma.desktop", "&Restart Now")
                onClicked: {
                    root.rebootRequested();
                }
                KeyNavigation.left: rebootButton
                KeyNavigation.right: shutdownButton.visible ? shutdownButton : (logoutButton.visible ? logoutButton : cancelButton)
                visible: maysd && softwareUpdatePending && (sdtype === ShutdownType.ShutdownTypeReboot || root.showAllOptions)
            }
            LogoutButton {
                id: shutdownButton
                icon.name: softwareUpdatePending ? "system-shutdown-update" : "system-shutdown"
                text: {
                    if (softwareUpdatePending) {
                        return i18ndc("plasma_shell_org.kde.plasma.desktop", "@action:button Keep short", "Install Updates and Shut Down")
                    } else {
                        return root.showAllOptions ? i18nd("plasma_shell_org.kde.plasma.desktop", "&Shut Down")
                                                   : i18nd("plasma_shell_org.kde.plasma.desktop", "&Shut Down Now")
                    }
                }
                onClicked: {
                    if (softwareUpdatePending) {
                        root.haltUpdateRequested();
                    } else {
                        root.haltRequested();
                    }
                }
                KeyNavigation.left: rebootWithoutUpdatesButton.visible ? rebootWithoutUpdatesButton : (rebootButton.visible ? rebootButton : (hibernateButton.visible ? hibernateButton : (suspendButton.visible ? suspendButton : cancelButton)))
                KeyNavigation.right: shutdownWithoutUpdatesButton.visible ? shutdownWithoutUpdatesButton : (logoutButton.visible ? logoutButton : cancelButton)
                focus: sdtype === ShutdownType.ShutdownTypeHalt || root.showAllOptions
                visible: maysd && (sdtype === ShutdownType.ShutdownTypeHalt || root.showAllOptions)
            }
            LogoutButton {
                id: shutdownWithoutUpdatesButton
                icon.name: "system-shutdown"
                text: root.showAllOptions ? i18nd("plasma_shell_org.kde.plasma.desktop", "&Shut Down")
                                          : i18nd("plasma_shell_org.kde.plasma.desktop", "&Shut Down Now")
                onClicked: {
                    root.haltRequested();
                }
                KeyNavigation.left: shutdownButton
                KeyNavigation.right: logoutButton.visible ? logoutButton : cancelButton
                focus: sdtype === ShutdownType.ShutdownTypeHalt || root.showAllOptions
                visible: maysd && softwareUpdatePending && (sdtype === ShutdownType.ShutdownTypeHalt || root.showAllOptions)
            }
            LogoutButton {
                id: logoutButton
                icon.name: "system-log-out"
                text: root.showAllOptions ? i18nd("plasma_shell_org.kde.plasma.desktop", "&Log Out")
                                          : i18nd("plasma_shell_org.kde.plasma.desktop", "&Log Out Now")
                onClicked: root.logoutRequested()
                KeyNavigation.left: shutdownWithoutUpdatesButton.visible ? shutdownWithoutUpdatesButton : (shutdownButton.visible ? shutdownButton : (rebootWithoutUpdatesButton.visible ? rebootWithoutUpdatesButton : (rebootButton.visible ? rebootButton : (hibernateButton.visible ? hibernateButton : (suspendButton.visible ? suspendButton : cancelButton)))))
                KeyNavigation.right: cancelButton
                focus: sdtype === ShutdownType.ShutdownTypeNone
                visible: canLogout && (sdtype === ShutdownType.ShutdownTypeNone || root.showAllOptions)
            }
            LogoutButton {
                id: cancelButton
                icon.name: "dialog-cancel"
                text: i18nd("plasma_shell_org.kde.plasma.desktop", "&Cancel")
                onClicked: root.cancelRequested()
                KeyNavigation.left: logoutButton.visible ? logoutButton : (shutdownWithoutUpdatesButton.visible ? shutdownWithoutUpdatesButton : (shutdownButton.visible ? shutdownButton : (rebootWithoutUpdatesButton.visible ? rebootWithoutUpdatesButton : (rebootButton.visible ? rebootButton : (hibernateButton.visible ? hibernateButton : suspendButton)))))
                KeyNavigation.right: suspendButton.visible ? suspendButton : (hibernateButton.visible ? hibernateButton : rebootButton)
            }
        }
    }
}
