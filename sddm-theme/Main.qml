/*
    SPDX-FileCopyrightText: 2016 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as QQC2
import Qt5Compat.GraphicalEffects

import org.kde.plasma.components 3.0 as PlasmaComponents3
import org.kde.plasma.private.keyboardindicator as KeyboardIndicator
import org.kde.kirigami 2.20 as Kirigami

import org.kde.breeze.components

Item {
    id: root

    // If we're using software rendering, draw outlines instead of shadows
    // See https://bugs.kde.org/show_bug.cgi?id=398317
    readonly property bool softwareRendering: GraphicsInfo.api === GraphicsInfo.Software

    Kirigami.Theme.colorSet: Kirigami.Theme.Complementary
    Kirigami.Theme.inherit: false

    width: 1600
    height: 900

    property string notificationMessage

    LayoutMirroring.enabled: Qt.application.layoutDirection === Qt.RightToLeft
    LayoutMirroring.childrenInherit: true

    KeyboardIndicator.KeyState {
        id: capsLockState
        key: Qt.Key_CapsLock
    }

    Item {
        id: wallpaper
        anchors.fill: parent
        Repeater {
            model: screenModel

            Background {
                x: geometry.x; y: geometry.y; width: geometry.width; height: geometry.height
                sceneBackgroundType: config.type
                sceneBackgroundColor: config.color
                sceneBackgroundImage: config.background
            }
        }
    }

    RejectPasswordAnimation {
        id: rejectPasswordAnimation
        target: mainStack
    }

    MouseArea {
        id: loginScreenRoot
        anchors.fill: parent

        property bool uiVisible: true
        property bool blockUI: mainStack.depth > 1 || userListComponent.mainPasswordBox.text.length > 0 || inputPanel.keyboardActive || config.type !== "image"

        hoverEnabled: true
        drag.filterChildren: true
        onPressed: uiVisible = true;
        onPositionChanged: uiVisible = true;
        onUiVisibleChanged: {
            if (blockUI) {
                fadeoutTimer.running = false;
            } else if (uiVisible) {
                fadeoutTimer.restart();
            }
        }
        onBlockUIChanged: {
            if (blockUI) {
                fadeoutTimer.running = false;
                uiVisible = true;
            } else {
                fadeoutTimer.restart();
            }
        }

        Keys.onPressed: event => {
            uiVisible = true;
            event.accepted = false;
        }

        //takes one full minute for the ui to disappear
        Timer {
            id: fadeoutTimer
            running: true
            interval: 60000
            onTriggered: {
                if (!loginScreenRoot.blockUI) {
                    userListComponent.mainPasswordBox.showPassword = false;
                    loginScreenRoot.uiVisible = false;
                }
            }
        }
        WallpaperFader {
            visible: config.type === "image"
            anchors.fill: parent
            state: loginScreenRoot.uiVisible ? "on" : "off"
            source: wallpaper
            mainStack: mainStack
            footer: footer
            clock: clock
        }

        DropShadow {
            id: clockShadow
            anchors.fill: clock
            source: clock
            visible: !softwareRendering && config.showClock === "true"
            radius: 6
            samples: 14
            spread: 0.3
            color : "black" // shadows should always be black
            Behavior on opacity {
                OpacityAnimator {
                    duration: Kirigami.Units.veryLongDuration * 2
                    easing.type: Easing.InOutQuad
                }
            }
        }

        Clock {
            id: clock
            property Item shadow: clockShadow
            visible: y > 0 && config.showClock === "true"
            anchors.horizontalCenter: parent.horizontalCenter
            y: (userListComponent.userList.y + mainStack.y)/2 - height/2
            Layout.alignment: Qt.AlignBaseline
        }

        QQC2.StackView {
            id: mainStack
            anchors {
                left: parent.left
                right: parent.right
            }
            height: root.height + Kirigami.Units.gridUnit * 3

            // If true (depends on the style and environment variables), hover events are always accepted
            // and propagation stopped. This means the parent MouseArea won't get them and the UI won't be shown.
            // Disable capturing those events while the UI is hidden to avoid that, while still passing events otherwise.
            // One issue is that while the UI is visible, mouse activity won't keep resetting the timer, but when it
            // finally expires, the next event should immediately set uiVisible = true again.
            hoverEnabled: loginScreenRoot.uiVisible ? undefined : false

            focus: true //StackView is an implicit focus scope, so we need to give this focus so the item inside will have it

            Timer {
                //SDDM has a bug in 0.13 where even though we set the focus on the right item within the window, the window doesn't have focus
                //it is fixed in 6d5b36b28907b16280ff78995fef764bb0c573db which will be 0.14
                //we need to call "window->activate()" *After* it's been shown. We can't control that in QML so we use a shoddy timer
                //it's been this way for all Plasma 5.x without a huge problem
                running: true
                repeat: false
                interval: 200
                onTriggered: mainStack.forceActiveFocus()
            }

            initialItem: Login {
                id: userListComponent
                userListModel: userModel
                loginScreenUiVisible: loginScreenRoot.uiVisible
                userListCurrentIndex: userModel.lastIndex >= 0 ? userModel.lastIndex : 0
                lastUserName: userModel.lastUser
                showUserList: {
                    if (!userListModel.hasOwnProperty("count")
                        || !userListModel.hasOwnProperty("disableAvatarsThreshold")) {
                        return false
                    }

                    if (userListModel.count === 0 ) {
                        return false
                    }

                    if (userListModel.hasOwnProperty("containsAllUsers") && !userListModel.containsAllUsers) {
                        return false
                    }

                    return userListModel.count <= userListModel.disableAvatarsThreshold
                }

                notificationMessage: {
                    const parts = [];
                    if (capsLockState.locked) {
                        parts.push(i18nd("plasma-desktop-sddm-theme", "Caps Lock is on"));
                    }
                    if (root.notificationMessage) {
                        parts.push(root.notificationMessage);
                    }
                    return parts.join(" • ");
                }

                actionItemsVisible: !inputPanel.keyboardActive
                actionItems: [
                    ActionButton {
                        icon.name: "system-suspend"
                        text: i18ndc("plasma-desktop-sddm-theme", "Suspend to RAM", "Sleep")
                        onClicked: sddm.suspend()
                        enabled: sddm.canSuspend
                    },
                    ActionButton {
                        icon.name: "system-reboot"
                        text: i18nd("plasma-desktop-sddm-theme", "Restart")
                        onClicked: sddm.reboot()
                        enabled: sddm.canReboot
                    },
                    ActionButton {
                        icon.name: "system-shutdown"
                        text: i18nd("plasma-desktop-sddm-theme", "Shut Down")
                        onClicked: sddm.powerOff()
                        enabled: sddm.canPowerOff
                    },
                    ActionButton {
                        icon.name: "system-user-prompt"
                        text: i18ndc("plasma-desktop-sddm-theme", "For switching to a username and password prompt", "Other…")
                        onClicked: mainStack.push(userPromptComponent)
                        visible: !userListComponent.showUsernamePrompt
                    }]

                onLoginRequest: {
                    root.notificationMessage = ""
                    sddm.login(username, password, sessionButton.currentIndex)
                }
            }

            Behavior on opacity {
                OpacityAnimator {
                    duration: Kirigami.Units.longDuration
                }
            }

            readonly property real zoomFactor: 1.5

            popEnter: Transition {
                ScaleAnimator {
                    from: mainStack.zoomFactor
                    to: 1
                    duration: Kirigami.Units.veryLongDuration
                    easing.type: Easing.OutCubic
                }
                OpacityAnimator {
                    from: 0
                    to: 1
                    duration: Kirigami.Units.veryLongDuration
                    easing.type: Easing.OutCubic
                }
            }

            popExit: Transition {
                ScaleAnimator {
                    from: 1
                    to: 1 / mainStack.zoomFactor
                    duration: Kirigami.Units.veryLongDuration
                    easing.type: Easing.OutCubic
                }
                OpacityAnimator {
                    from: 1
                    to: 0
                    duration: Kirigami.Units.veryLongDuration
                    easing.type: Easing.OutCubic
                }
            }

            pushEnter: Transition {
                ScaleAnimator {
                    from: 1 / mainStack.zoomFactor
                    to: 1
                    duration: Kirigami.Units.veryLongDuration
                    easing.type: Easing.OutCubic
                }
                OpacityAnimator {
                    from: 0
                    to: 1
                    duration: Kirigami.Units.veryLongDuration
                    easing.type: Easing.OutCubic
                }
            }

            pushExit: Transition {
                ScaleAnimator {
                    from: 1
                    to: mainStack.zoomFactor
                    duration: Kirigami.Units.veryLongDuration
                    easing.type: Easing.OutCubic
                }
                OpacityAnimator {
                    from: 1
                    to: 0
                    duration: Kirigami.Units.veryLongDuration
                    easing.type: Easing.OutCubic
                }
            }
        }

        VirtualKeyboardLoader {
            id: inputPanel

            z: 1

            screenRoot: root
            mainStack: mainStack
            mainBlock: userListComponent
            passwordField: userListComponent.mainPasswordBox
        }

        Component {
            id: userPromptComponent
            Login {
                showUsernamePrompt: true
                notificationMessage: root.notificationMessage
                loginScreenUiVisible: loginScreenRoot.uiVisible
                fontSize: Kirigami.Theme.defaultFont.pointSize + 2

                // using a model rather than a QObject list to avoid QTBUG-75900
                userListModel: ListModel {
                    ListElement {
                        name: ""
                        icon: ""
                    }
                    Component.onCompleted: {
                        // as we can't bind inside ListElement
                        setProperty(0, "name", i18nd("plasma-desktop-sddm-theme", "Type in Username and Password"));
                        setProperty(0, "icon", Qt.resolvedUrl("faces/.face.icon"))
                    }
                }

                onLoginRequest: {
                    root.notificationMessage = ""
                    sddm.login(username, password, sessionButton.currentIndex)
                }

                actionItemsVisible: !inputPanel.keyboardActive
                actionItems: [
                    ActionButton {
                        icon.name: "system-suspend"
                        text: i18ndc("plasma-desktop-sddm-theme", "Suspend to RAM", "Sleep")
                        onClicked: sddm.suspend()
                        enabled: sddm.canSuspend
                    },
                    ActionButton {
                        icon.name: "system-reboot"
                        text: i18nd("plasma-desktop-sddm-theme", "Restart")
                        onClicked: sddm.reboot()
                        enabled: sddm.canReboot
                    },
                    ActionButton {
                        icon.name: "system-shutdown"
                        text: i18nd("plasma-desktop-sddm-theme", "Shut Down")
                        onClicked: sddm.powerOff()
                        enabled: sddm.canPowerOff
                    },
                    ActionButton {
                        icon.name: "system-user-list"
                        text: i18nd("plasma-desktop-sddm-theme", "List Users")
                        onClicked: mainStack.pop()
                    }
                ]
            }
        }

        DropShadow {
            id: logoShadow
            anchors.fill: logo
            source: logo
            visible: !softwareRendering && config.showlogo === "shown"
            horizontalOffset: 1
            verticalOffset: 1
            radius: 6
            samples: 14
            spread: 0.3
            color : "black" // shadows should always be black
            opacity: loginScreenRoot.uiVisible ? 0 : 1
            Behavior on opacity {
                //OpacityAnimator when starting from 0 is buggy (it shows one frame with opacity 1)"
                NumberAnimation {
                    duration: Kirigami.Units.longDuration
                    easing.type: Easing.InOutQuad
                }
            }
        }

        Image {
            id: logo
            visible: config.showlogo === "shown"
            source: config.logo
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.bottom: footer.top
            anchors.bottomMargin: Kirigami.Units.largeSpacing
            asynchronous: true
            sourceSize.height: height
            opacity: loginScreenRoot.uiVisible ? 0 : 1
            fillMode: Image.PreserveAspectFit
            height: Math.round(Kirigami.Units.gridUnit * 3.5)
            Behavior on opacity {
                // OpacityAnimator when starting from 0 is buggy (it shows one frame with opacity 1)"
                NumberAnimation {
                    duration: Kirigami.Units.longDuration
                    easing.type: Easing.InOutQuad
                }
            }
        }

        // Note: Containment masks stretch clickable area of their buttons to
        // the screen edges, essentially making them adhere to Fitts's law.
        // Due to virtual keyboard button having an icon, buttons may have
        // different heights, so fillHeight is required.
        //
        // Note for contributors: Keep this in sync with LockScreenUi.qml footer.
        RowLayout {
            id: footer
            anchors {
                bottom: parent.bottom
                left: parent.left
                right: parent.right
                margins: Kirigami.Units.smallSpacing
            }
            spacing: Kirigami.Units.smallSpacing

            Behavior on opacity {
                OpacityAnimator {
                    duration: Kirigami.Units.longDuration
                }
            }

            PlasmaComponents3.ToolButton {
                id: virtualKeyboardButton

                text: i18ndc("plasma-desktop-sddm-theme", "Button to show/hide virtual keyboard", "Virtual Keyboard")
                icon.name: inputPanel.keyboardActive ? "input-keyboard-virtual-on" : "input-keyboard-virtual-off"
                onClicked: {
                    // Otherwise the password field loses focus and virtual keyboard
                    // keystrokes get eaten
                    userListComponent.mainPasswordBox.forceActiveFocus();
                    inputPanel.showHide()
                }
                visible: inputPanel.status === Loader.Ready

                Layout.fillHeight: true
                containmentMask: Item {
                    parent: virtualKeyboardButton
                    anchors.fill: parent
                    anchors.leftMargin: -footer.anchors.margins
                    anchors.bottomMargin: -footer.anchors.margins
                }
            }

            KeyboardButton {
                id: keyboardButton

                onKeyboardLayoutChanged: {
                    // Otherwise the password field loses focus and virtual keyboard
                    // keystrokes get eaten
                    userListComponent.mainPasswordBox.forceActiveFocus();
                }

                Layout.fillHeight: true
                containmentMask: Item {
                    parent: keyboardButton
                    anchors.fill: parent
                    anchors.leftMargin: virtualKeyboardButton.visible ? 0 : -footer.anchors.margins
                    anchors.bottomMargin: -footer.anchors.margins
                }
            }

            SessionButton {
                id: sessionButton

                onSessionChanged: {
                    // Otherwise the password field loses focus and virtual keyboard
                    // keystrokes get eaten
                    userListComponent.mainPasswordBox.forceActiveFocus();
                }

                Layout.fillHeight: true
                containmentMask: Item {
                    parent: sessionButton
                    anchors.fill: parent
                    anchors.leftMargin: virtualKeyboardButton.visible || keyboardButton.visible
                        ? 0 : -footer.anchors.margins
                    anchors.bottomMargin: -footer.anchors.margins
                }
            }

            Item {
                Layout.fillWidth: true
            }

            Battery {}
        }
    }

    Connections {
        target: sddm
        function onLoginFailed() {
            notificationMessage = i18nd("plasma-desktop-sddm-theme", "Login Failed")
            footer.enabled = true
            mainStack.enabled = true
            userListComponent.userList.opacity = 1
            rejectPasswordAnimation.start()
        }
        function onLoginSucceeded() {
            //note SDDM will kill the greeter at some random point after this
            //there is no certainty any transition will finish, it depends on the time it
            //takes to complete the init
            mainStack.opacity = 0
            footer.opacity = 0
        }
    }

    onNotificationMessageChanged: {
        if (notificationMessage) {
            notificationResetTimer.start();
        }
    }

    Timer {
        id: notificationResetTimer
        interval: 3000
        onTriggered: notificationMessage = ""
    }
}
