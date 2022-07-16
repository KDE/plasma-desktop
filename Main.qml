/*
    SPDX-FileCopyrightText: 2016 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as QQC2
import QtGraphicalEffects 1.15

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 3.0 as PlasmaComponents3
import org.kde.plasma.extras 2.0 as PlasmaExtras

import "components"
import "components/animation"

// TODO: Once SDDM 0.19 is released and we are setting the font size using the
// SDDM KCM's syncing feature, remove the `config.fontSize` overrides here and
// the fontSize properties in various components, because the theme's default
// font size will be correctly propagated to the login screen

PlasmaCore.ColorScope {
    id: root

    // If we're using software rendering, draw outlines instead of shadows
    // See https://bugs.kde.org/show_bug.cgi?id=398317
    readonly property bool softwareRendering: GraphicsInfo.api === GraphicsInfo.Software

    colorGroup: PlasmaCore.Theme.ComplementaryColorGroup

    width: 1600
    height: 900

    property string notificationMessage

    LayoutMirroring.enabled: Qt.application.layoutDirection === Qt.RightToLeft
    LayoutMirroring.childrenInherit: true

    PlasmaCore.DataSource {
        id: keystateSource
        engine: "keystate"
        connectedSources: "Caps Lock"
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

        Keys.onPressed: {
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
            visible: !softwareRendering
            horizontalOffset: 1
            verticalOffset: 1
            radius: 6
            samples: 14
            spread: 0.3
            color : "black" // shadows should always be black
            Behavior on opacity {
                OpacityAnimator {
                    duration: PlasmaCore.Units.veryLongDuration * 2
                    easing.type: Easing.InOutQuad
                }
            }
        }

        Clock {
            id: clock
            property Item shadow: clockShadow
            visible: y > 0
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
            height: root.height + PlasmaCore.Units.gridUnit * 3

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
                    if (keystateSource.data["Caps Lock"]["Locked"]) {
                        parts.push(i18nd("plasma_lookandfeel_org.kde.lookandfeel", "Caps Lock is on"));
                    }
                    if (root.notificationMessage) {
                        parts.push(root.notificationMessage);
                    }
                    return parts.join(" • ");
                }

                actionItemsVisible: !inputPanel.keyboardActive
                actionItems: [
                    ActionButton {
                        iconSource: "system-suspend"
                        text: i18ndc("plasma_lookandfeel_org.kde.lookandfeel", "Suspend to RAM", "Sleep")
                        fontSize: parseInt(config.fontSize) + 1
                        onClicked: sddm.suspend()
                        enabled: sddm.canSuspend
                    },
                    ActionButton {
                        iconSource: "system-reboot"
                        text: i18nd("plasma_lookandfeel_org.kde.lookandfeel", "Restart")
                        fontSize: parseInt(config.fontSize) + 1
                        onClicked: sddm.reboot()
                        enabled: sddm.canReboot
                    },
                    ActionButton {
                        iconSource: "system-shutdown"
                        text: i18nd("plasma_lookandfeel_org.kde.lookandfeel", "Shut Down")
                        fontSize: parseInt(config.fontSize) + 1
                        onClicked: sddm.powerOff()
                        enabled: sddm.canPowerOff
                    },
                    ActionButton {
                        iconSource: "system-user-prompt"
                        text: i18ndc("plasma_lookandfeel_org.kde.lookandfeel", "For switching to a username and password prompt", "Other…")
                        fontSize: parseInt(config.fontSize) + 1
                        onClicked: mainStack.push(userPromptComponent)
                        enabled: true
                        visible: !userListComponent.showUsernamePrompt
                    }]

                onLoginRequest: {
                    root.notificationMessage = ""
                    sddm.login(username, password, sessionButton.currentIndex)
                }
            }

            Behavior on opacity {
                OpacityAnimator {
                    duration: PlasmaCore.Units.longDuration
                }
            }

            readonly property real zoomFactor: 3

            popEnter: Transition {
                ScaleAnimator {
                    from: mainStack.zoomFactor
                    to: 1
                    duration: PlasmaCore.Units.longDuration * (mainStack.zoomFactor / 2)
                    easing.type: Easing.OutCubic
                }
                OpacityAnimator {
                    from: 0
                    to: 1
                    duration: PlasmaCore.Units.longDuration * (mainStack.zoomFactor / 2)
                    easing.type: Easing.OutCubic
                }
            }

            popExit: Transition {
                ScaleAnimator {
                    from: 1
                    to: 0
                    duration: PlasmaCore.Units.longDuration * (mainStack.zoomFactor / 2)
                    easing.type: Easing.OutCubic
                }
                OpacityAnimator {
                    from: 1
                    to: 0
                    duration: PlasmaCore.Units.longDuration * (mainStack.zoomFactor / 2)
                    easing.type: Easing.OutCubic
                }
            }

            pushEnter: Transition {
                ScaleAnimator {
                    from: 0
                    to: 1
                    duration: PlasmaCore.Units.longDuration * (mainStack.zoomFactor / 2)
                    easing.type: Easing.OutCubic
                }
                OpacityAnimator {
                    from: 0
                    to: 1
                    duration: PlasmaCore.Units.longDuration * (mainStack.zoomFactor / 2)
                    easing.type: Easing.OutCubic
                }
            }

            pushExit: Transition {
                ScaleAnimator {
                    from: 1
                    to: mainStack.zoomFactor
                    duration: PlasmaCore.Units.longDuration * (mainStack.zoomFactor / 2)
                    easing.type: Easing.OutCubic
                }
                OpacityAnimator {
                    from: 1
                    to: 0
                    duration: PlasmaCore.Units.longDuration * (mainStack.zoomFactor / 2)
                    easing.type: Easing.OutCubic
                }
            }
        }

        Loader {
            id: inputPanel
            state: "hidden"
            property bool keyboardActive: item ? item.active : false
            onKeyboardActiveChanged: {
                if (keyboardActive) {
                    state = "visible"
                    // Otherwise the password field loses focus and virtual keyboard
                    // keystrokes get eaten
                    userListComponent.mainPasswordBox.forceActiveFocus();
                } else {
                    state = "hidden";
                }
            }

            source: Qt.platform.pluginName.includes("wayland") ? "components/VirtualKeyboard_wayland.qml" : "components/VirtualKeyboard.qml"
            anchors {
                left: parent.left
                right: parent.right
            }

            function showHide() {
                state = state === "hidden" ? "visible" : "hidden";
            }

            states: [
                State {
                    name: "visible"
                    PropertyChanges {
                        target: mainStack
                        y: Math.min(0, root.height - inputPanel.height - userListComponent.visibleBoundary)
                    }
                    PropertyChanges {
                        target: inputPanel
                        y: root.height - inputPanel.height
                        opacity: 1
                    }
                },
                State {
                    name: "hidden"
                    PropertyChanges {
                        target: mainStack
                        y: 0
                    }
                    PropertyChanges {
                        target: inputPanel
                        y: root.height - root.height/4
                        opacity: 0
                    }
                }
            ]
            transitions: [
                Transition {
                    from: "hidden"
                    to: "visible"
                    SequentialAnimation {
                        ScriptAction {
                            script: {
                                inputPanel.item.activated = true;
                                Qt.inputMethod.show();
                            }
                        }
                        ParallelAnimation {
                            NumberAnimation {
                                target: mainStack
                                property: "y"
                                duration: PlasmaCore.Units.longDuration
                                easing.type: Easing.InOutQuad
                            }
                            NumberAnimation {
                                target: inputPanel
                                property: "y"
                                duration: PlasmaCore.Units.longDuration
                                easing.type: Easing.OutQuad
                            }
                            OpacityAnimator {
                                target: inputPanel
                                duration: PlasmaCore.Units.longDuration
                                easing.type: Easing.OutQuad
                            }
                        }
                    }
                },
                Transition {
                    from: "visible"
                    to: "hidden"
                    SequentialAnimation {
                        ParallelAnimation {
                            NumberAnimation {
                                target: mainStack
                                property: "y"
                                duration: PlasmaCore.Units.longDuration
                                easing.type: Easing.InOutQuad
                            }
                            NumberAnimation {
                                target: inputPanel
                                property: "y"
                                duration: PlasmaCore.Units.longDuration
                                easing.type: Easing.InQuad
                            }
                            OpacityAnimator {
                                target: inputPanel
                                duration: PlasmaCore.Units.longDuration
                                easing.type: Easing.InQuad
                            }
                        }
                        ScriptAction {
                            script: {
                                inputPanel.item.activated = false;
                                Qt.inputMethod.hide();
                            }
                        }
                    }
                }
            ]
        }

        Component {
            id: userPromptComponent
            Login {
                showUsernamePrompt: true
                notificationMessage: root.notificationMessage
                loginScreenUiVisible: loginScreenRoot.uiVisible
                fontSize: parseInt(config.fontSize) + 2

                // using a model rather than a QObject list to avoid QTBUG-75900
                userListModel: ListModel {
                    ListElement {
                        name: ""
                        iconSource: ""
                    }
                    Component.onCompleted: {
                        // as we can't bind inside ListElement
                        setProperty(0, "name", i18nd("plasma_lookandfeel_org.kde.lookandfeel", "Type in Username and Password"));
                    }
                }

                onLoginRequest: {
                    root.notificationMessage = ""
                    sddm.login(username, password, sessionButton.currentIndex)
                }

                actionItemsVisible: !inputPanel.keyboardActive
                actionItems: [
                    ActionButton {
                        iconSource: "system-suspend"
                        text: i18ndc("plasma_lookandfeel_org.kde.lookandfeel", "Suspend to RAM", "Sleep")
                        fontSize: parseInt(config.fontSize) + 1
                        onClicked: sddm.suspend()
                        enabled: sddm.canSuspend
                    },
                    ActionButton {
                        iconSource: "system-reboot"
                        text: i18nd("plasma_lookandfeel_org.kde.lookandfeel", "Restart")
                        fontSize: parseInt(config.fontSize) + 1
                        onClicked: sddm.reboot()
                        enabled: sddm.canReboot
                    },
                    ActionButton {
                        iconSource: "system-shutdown"
                        text: i18nd("plasma_lookandfeel_org.kde.lookandfeel", "Shut Down")
                        fontSize: parseInt(config.fontSize) + 1
                        onClicked: sddm.powerOff()
                        enabled: sddm.canPowerOff
                    },
                    ActionButton {
                        iconSource: "system-user-list"
                        text: i18nd("plasma_lookandfeel_org.kde.lookandfeel", "List Users")
                        fontSize: parseInt(config.fontSize) + 1
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
                    duration: PlasmaCore.Units.longDuration
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
            anchors.bottomMargin: PlasmaCore.Units.largeSpacing
            asynchronous: true
            sourceSize.height: height
            opacity: loginScreenRoot.uiVisible ? 0 : 1
            fillMode: Image.PreserveAspectFit
            height: Math.round(PlasmaCore.Units.gridUnit * 3.5)
            Behavior on opacity {
                // OpacityAnimator when starting from 0 is buggy (it shows one frame with opacity 1)"
                NumberAnimation {
                    duration: PlasmaCore.Units.longDuration
                    easing.type: Easing.InOutQuad
                }
            }
        }

        //Footer
        RowLayout {
            id: footer
            anchors {
                bottom: parent.bottom
                left: parent.left
                right: parent.right
                margins: PlasmaCore.Units.smallSpacing
            }

            Behavior on opacity {
                OpacityAnimator {
                    duration: PlasmaCore.Units.longDuration
                }
            }

            PlasmaComponents3.ToolButton {
                text: i18ndc("plasma_lookandfeel_org.kde.lookandfeel", "Button to show/hide virtual keyboard", "Virtual Keyboard")
                font.pointSize: config.fontSize
                icon.name: inputPanel.keyboardActive ? "input-keyboard-virtual-on" : "input-keyboard-virtual-off"
                onClicked: inputPanel.showHide()
                visible: inputPanel.status === Loader.Ready
            }

            KeyboardButton {
                font.pointSize: config.fontSize

                onKeyboardLayoutChanged: {
                    // Otherwise the password field loses focus and virtual keyboard
                    // keystrokes get eaten
                    userListComponent.mainPasswordBox.forceActiveFocus();
                }
            }

            SessionButton {
                id: sessionButton
                font.pointSize: config.fontSize

                onSessionChanged: {
                    // Otherwise the password field loses focus and virtual keyboard
                    // keystrokes get eaten
                    userListComponent.mainPasswordBox.forceActiveFocus();
                }
            }

            Item {
                Layout.fillWidth: true
            }

            Battery {
                fontSize: config.fontSize
            }
        }
    }

    Connections {
        target: sddm
        function onLoginFailed() {
            notificationMessage = i18nd("plasma_lookandfeel_org.kde.lookandfeel", "Login Failed")
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
