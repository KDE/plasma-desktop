/*
    SPDX-FileCopyrightText: 2016 David Edmundson <davidedmundson@kde.org>
    SPDX-FileCopyrightText: 2024 Noah Davis <noahadvs@gmail.com>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import org.kde.breeze.components

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2

import org.kde.plasma.components as PlasmaComponents3
import org.kde.plasma.extras as PlasmaExtras
import org.kde.kirigami as Kirigami

SessionManagementScreen {
    id: root
    required property int sessionIndex

    focus: true

    requestLoginCallback: (username, password) => {
        root.setNotificationMessage();
        // We want to do the login after the connections to the signals
        Qt.callLater(sddm.login, username, password, root.sessionIndex);
        return [sddm.loginFailed, sddm.loginSucceeded]
    }

    Connections {
        target: sddm
        function onLoginFailed() {
            passwordField.selectAll()
            passwordField.forceActiveFocus(Qt.TabFocusReason)
        }
    }

    // Disable reveal password action because SDDM does not have the breeze icon set loaded
    passwordField.rightActions: []
}
