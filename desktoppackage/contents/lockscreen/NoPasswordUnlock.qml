/*
    SPDX-FileCopyrightText: 2022 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
import QtQuick

import org.kde.plasma.components as PlasmaComponents3

import org.kde.breeze.components

SessionManagementScreen {
    focus: true
    PlasmaComponents3.Button {
        id: loginButton
        focus: true
        text: i18ndc("plasma_shell_org.kde.plasma.desktop", "@action:button no-password unlock", "Unlock")
        icon.name: "unlock"
        onClicked: Qt.quit();
        Keys.onEnterPressed: clicked()
        Keys.onReturnPressed: clicked()
    }

    Component.onCompleted: {
        forceActiveFocus();
    }
}
