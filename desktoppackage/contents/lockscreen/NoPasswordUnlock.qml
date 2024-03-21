/*
    SPDX-FileCopyrightText: 2022 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
import QtQuick 2.2

import org.kde.plasma.components 3.0 as PlasmaComponents3

import org.kde.breeze.components

SessionManagementScreen {
    focus: true
    PlasmaComponents3.Button {
        id: loginButton
        focus: true
        text: i18nd("plasma_lookandfeel_org.kde.lookandfeel", "Unlock")
        icon.name: "unlock"
        onClicked: Qt.quit();
        Keys.onEnterPressed: clicked()
        Keys.onReturnPressed: clicked()
    }

    Component.onCompleted: {
        forceActiveFocus();
        Qt.callLater(tryToSwitchUser, false)
    }
}
