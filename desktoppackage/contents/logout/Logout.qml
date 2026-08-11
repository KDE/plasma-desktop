/*
    SPDX-FileCopyrightText: 2014 Aleix Pol Gonzalez <aleixpol@blue-systems.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Window

import org.kde.kirigami as Kirigami

import "timer.js" as AutoTriggerTimer
import "controlsowner.js" as ControlsOwner

Item {
    id: root
    Kirigami.Theme.inherit: false
    Kirigami.Theme.colorSet: Kirigami.Theme.Complementary

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
    readonly property bool countingDown: countDownTimer.running

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

    Component {
        id: controlsComponent
        LogoutControls {}
    }

    function ensureControlsExist() {
        if (!ControlsOwner.controls) {
            ControlsOwner.controls = controlsComponent.createObject(root);
        }
    }

    Component.onCompleted: ensureControlsExist()

    // Not MouseArea: hovering a LogoutButton steals
    // the pointer from it, so containsMouse would flicker and the row would
    // flash in and out. HoverHandler is non-blocking, so it just tracks the
    // pointer position and stays accurate.
    HoverHandler {
        onHoveredChanged: {
            if (hovered) {
                if (root.Window.window) {
                    root.Window.window.requestActivate();
                }
                if (ControlsOwner.controls && ControlsOwner.controls.parent !== root) {
                    ControlsOwner.controls.parent = root;
                }
            }
        }
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
}
