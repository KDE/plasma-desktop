/*
    SPDX-FileCopyrightText: 2025 Yifan Zhu <fanzhuyifan@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

pragma Singleton

import QtQml
import QtQuick

Item {
    id: lockscreenState

    property string password: ""
    property bool showPassword: false

    // change to activeLockScreenUi?
    property var activeWindow: null

    readonly property bool inhibitGreeterTimeout: {
        if (lockscreenState.password.length > 0) {
            return true;
        }

        // inputPanel.keyboardActive of activeWindow?

        return false;
    }

    onInhibitGreeterTimeoutChanged: {
        let greeterShouldTimeOut = lockscreenState.activeWindow !== null && !lockscreenState.inhibitGreeterTimeout;
        if (greeterTimeoutTimer.running !== greeterShouldTimeOut) {
            greeterTimeoutTimer.running = greeterShouldTimeOut;
        }
    }

    Timer {
        id: greeterTimeoutTimer
        running: false
        interval: 10000
        onTriggered: {
            if (lockscreenState.activeWindow) {
                lockscreenState.showPassword = false;
                timeoutWindow(lockscreenState.activeWindow);
            }
        }
    }

    function clearPassword(): void {
        password = "";
    }

    function activateWindow(window): void {
        if (!window) {
            return;
        }

        lockscreenState.activeWindow = window;

        window.requestActivate();

        if (!inhibitGreeterTimeout) {
            greeterTimeoutTimer.restart();
        }
    }

    function timeoutWindow(window): void {
        if (lockscreenState.activeWindow == window) {
            lockscreenState.activeWindow = null;
        }

        greeterTimeoutTimer.stop();
    }
}
