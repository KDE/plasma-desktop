/*
    SPDX-FileCopyrightText: 2016 David Edmundson <davidedmundson@kde.org>
    SPDX-FileCopyrightText: 2024 Noah Davis <noahadvs@gmail.com>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import QtQuick.Templates as T
import Qt5Compat.GraphicalEffects

import org.kde.plasma.components as PlasmaComponents3
import org.kde.plasma.private.keyboardindicator as KeyboardIndicator
import org.kde.kirigami as Kirigami

import org.kde.breeze.components

T.Page {
    id: root

    // If we're using software rendering, draw outlines instead of shadows
    // See https://bugs.kde.org/show_bug.cgi?id=398317
    readonly property bool softwareRendering: GraphicsInfo.api === GraphicsInfo.Software

    // Not setting implicit size because this is a fullscreen UI

    Kirigami.Theme.colorSet: Kirigami.Theme.Complementary
    Kirigami.Theme.inherit: false

    width: 1600
    height: 900

    font.pointSize: Kirigami.Theme.defaultFont.pointSize + 2

    LayoutMirroring.enabled: Qt.application.layoutDirection === Qt.RightToLeft
    LayoutMirroring.childrenInherit: true

    KeyboardIndicator.KeyState {
        id: capsLockState
        key: Qt.Key_CapsLock
    }

    property bool uiVisible: true
    onUiVisibleChanged: {
        if (blockUiFade) {
            fadeoutTimer.running = false;
        } else if (uiVisible) {
            fadeoutTimer.restart();
        }
    }

    readonly property bool blockUiFade: mainStack.depth > 1
        || inputPanel.keyboardActive || config.type !== "image"
        || (mainStack.currentItem?.passwordField?.text.length ?? 0) > 0
        || (mainStack.currentItem?.waitingForResponse ?? false)
        || activityDetector.active
    onBlockUiFadeChanged: {
        if (blockUiFade) {
            fadeoutTimer.running = false;
            uiVisible = true;
        } else {
            fadeoutTimer.restart();
        }
    }

    property alias uiOpacity: wallpaperFader.uiOpacity
    background: WallpaperFader {
        id: wallpaperFader
        visible: config.type === "image"
        state: root.uiVisible ? "on" : "off"
        source: Item {
            z: -1
            parent: wallpaperFader
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
    }

    // When using forwardTo, be careful to avoid event propagation loops
    // Example: child1 -> this -> child2 -> this -> child2 -> this...
    // That shouldn't happen here since the activity detector isn't a child.
    Keys.forwardTo: activityDetector
    ActivityDetectorItem {
        id: activityDetector
        // To ensure that it covers absolutely everything
        parent: root
        anchors.fill: parent
        z: 99
        cursorShape: root.uiVisible ? undefined : Qt.BlankCursor
    }

    RejectPasswordAnimation {
        id: rejectPasswordAnimation
        target: mainStack.currentItem?.contentItem ?? mainStack
    }

    //takes one full minute for the ui to disappear
    Timer {
        id: fadeoutTimer
        running: true
        interval: 60000
        onTriggered: {
            if (!root.blockUiFade) {
                let passwordField = mainStack.currentItem?.passwordField
                if (passwordField) {
                    passwordField.showPassword = false;
                }
                root.uiVisible = false;
            }
        }
    }

    // Item for freeform layouting with animations and/or anchors
    // In Pane subclasses like Page, this is where child items go by default.
    contentItem: Item {
        anchors.fill: parent // override normal contentItem size handling
        implicitWidth: Math.max(clock.implicitWidth, mainStack.implicitWidth)
        implicitHeight: clock.implicitHeight + mainStack.implicitHeight
    }

    Clock {
        id: clock
        visible: height >= implicitHeight
        width: Math.min(parent.width, implicitWidth)
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        anchors.bottom: mainStack.top
        uiOpacity: root.uiOpacity
    }

    component SuspendButton : ActionButton {
        icon.name: "system-suspend"
        text: i18ndc("plasma-desktop-sddm-theme", "Suspend to RAM", "Sleep")
        onClicked: sddm.suspend()
        enabled: sddm.canSuspend
    }
    component RestartButton : ActionButton {
        icon.name: "system-reboot"
        text: i18nd("plasma-desktop-sddm-theme", "Restart")
        onClicked: sddm.reboot()
        enabled: sddm.canReboot
    }
    component ShutdownButton : ActionButton {
        icon.name: "system-shutdown"
        text: i18nd("plasma-desktop-sddm-theme", "Shut Down")
        onClicked: sddm.powerOff()
        enabled: sddm.canPowerOff
    }

    QQC2.StackView {
        id: mainStack
        anchors.horizontalCenter: parent.horizontalCenter
        y: {
            const vkbY = inputPanel.y
            const parentH1_2 = parent.height / 2
            if (vkbY <= parentH1_2 - height / 2) {
                return vkbY - (currentItem?.virtualKeyboardBoundary ?? 0)
            } else {
                return parentH1_2 - (currentItem?.contentItem.y ?? 0)
            }
        }
        width: Math.min(parent.width, implicitWidth)
        height: Math.min(parent.height, implicitHeight)
        implicitWidth: userListComponent.implicitWidth
        implicitHeight: userListComponent.implicitHeight
        opacity: root.uiOpacity
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

        background: null
        initialItem: Login {
            id: userListComponent
            onYChanged:console.log(y)
            userListModel: userModel
            userListCurrentIndex: userModel.lastIndex >= 0 ? userModel.lastIndex : 0
            sessionIndex: sessionButton.currentIndex
            showUserList: (userListModel?.count ?? false)
                && (userListModel?.containsAllUsers ?? true)
                && userListModel.count <= (userListModel?.disableAvatarsThreshold ?? 0)
            usernameField.text: userModel.lastUser
            actionItemsVisible: !inputPanel.keyboardActive
            actionItems: [
                SuspendButton {},
                RestartButton {},
                ShutdownButton {},
                ActionButton {
                    icon.name: "system-user-prompt"
                    text: i18ndc("plasma-desktop-sddm-theme", "For switching to a username and password prompt", "Other…")
                    onClicked: mainStack.push(userPromptComponent)
                    visible: !userListComponent.usernameField.visible
                }
            ]
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
        parent: root
        z: 1
        passwordField: mainStack.currentItem?.passwordField ?? null
    }

    Component {
        id: userPromptComponent
        Login {
            onYChanged: console.log(this, y)
            usernameField.visible: true
            sessionIndex: sessionButton.currentIndex

            // using a model rather than a QObject list to avoid QTBUG-75900
            userListModel: ListModel {
                ListElement {
                    name: ""
                    icon: ""
                }
                Component.onCompleted: {
                    // as we can't bind inside ListElement
                    setProperty(0, "name", i18nd("plasma-desktop-sddm-theme", "Type in Username and Password"));
                    // SDDM uses strings for URLs in its models.
                    // The delegates expect urls as strings, so we use strings too.
                    setProperty(0, "icon", Qt.resolvedUrl("faces/.face.icon").toString())
                }
            }

            actionItemsVisible: !inputPanel.keyboardActive
            actionItems: [
                SuspendButton {},
                RestartButton {},
                ShutdownButton {},
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
        opacity: root.uiOpacity
    }

    Image {
        id: logo
        visible: config.showlogo === "shown"
        source: config.logo
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: Kirigami.Units.largeSpacing
        asynchronous: true
        sourceSize.height: Kirigami.Units.iconSizes.huge
        opacity: root.uiOpacity
        fillMode: Image.PreserveAspectFit
        height: Kirigami.Units.iconSizes.huge
    }

    // Note: Containment masks stretch clickable area of their buttons to
    // the screen edges, essentially making them adhere to Fitts's law.
    // Due to virtual keyboard button having an icon, buttons may have
    // different heights, so fillHeight is required.
    component ContainmentMaskItem : Item {
        anchors.fill: parent
        anchors.leftMargin: -parent.Layout.leftMargin
        anchors.rightMargin: -parent.Layout.rightMargin
        anchors.topMargin: -parent.Layout.topMargin
        anchors.bottomMargin: -parent.Layout.bottomMargin
    }

    // Note for contributors: Keep this in sync with LockScreenUi.qml footer.
    footer: PlasmaComponents3.ToolBar {
        id: footer
        enabled: mainStack.currentItem?.enabled ?? true
        opacity: root.uiOpacity
        background: null
        contentItem: RowLayout {
            readonly property real margins: Kirigami.Units.smallSpacing
            spacing: Kirigami.Units.smallSpacing

            PlasmaComponents3.ToolButton {
                id: virtualKeyboardButton

                text: i18ndc("plasma-desktop-sddm-theme", "Button to show/hide virtual keyboard", "Virtual Keyboard")
                icon.name: inputPanel.keyboardActive ? "input-keyboard-virtual-on" : "input-keyboard-virtual-off"
                onClicked: {
                    // Otherwise the password field loses focus and virtual keyboard
                    // keystrokes get eaten
                    mainStack.currentItem?.passwordField?.forceActiveFocus();
                    inputPanel.showHide()
                }
                visible: inputPanel.status === Loader.Ready

                Layout.fillHeight: true
                Layout.leftMargin: parent.margins
                Layout.bottomMargin: parent.margins
                containmentMask: ContainmentMaskItem {
                    parent: virtualKeyboardButton
                }
            }

            KeyboardButton {
                id: keyboardButton

                onKeyboardLayoutChanged: {
                    // Otherwise the password field loses focus and virtual keyboard
                    // keystrokes get eaten
                    mainStack.currentItem?.passwordField?.forceActiveFocus();
                }

                Layout.fillHeight: true
                Layout.leftMargin: virtualKeyboardButton.visible ? 0 : parent.margins
                Layout.bottomMargin: parent.margins
                containmentMask: ContainmentMaskItem {
                    parent: keyboardButton
                }
            }

            SessionButton {
                id: sessionButton

                onSessionChanged: {
                    // Otherwise the password field loses focus and virtual keyboard
                    // keystrokes get eaten
                    mainStack.currentItem?.passwordField?.forceActiveFocus();
                }

                Layout.fillHeight: true
                Layout.leftMargin: virtualKeyboardButton.visible || keyboardButton.visible
                    ? 0 : parent.margins
                Layout.bottomMargin: parent.margins
                containmentMask: ContainmentMaskItem {
                    parent: sessionButton
                }
            }

            Item {
                Layout.fillWidth: true
            }

            Battery {
                Layout.rightMargin: parent.margins
                Layout.bottomMargin: parent.margins
            }
        }
    }

    Connections {
        target: sddm
        function onLoginFailed() {
            root.setNotificationMessage(i18nd("plasma-desktop-sddm-theme", "Login Failed"))
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
}
