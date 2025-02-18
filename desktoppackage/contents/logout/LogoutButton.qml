/*
    SPDX-FileCopyrightText: 2016 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.2
import QtQuick.Layouts 1.2

import org.kde.kirigami 2.20 as Kirigami

import org.kde.breeze.components
import "timer.js" as AutoTriggerTimer

ActionButton {
    Layout.alignment: Qt.AlignTop

    icon.width: Kirigami.Units.iconSizes.huge
    icon.height: Kirigami.Units.iconSizes.huge

    font.underline: false // See https://phabricator.kde.org/D9452
    opacity: activeFocus || hovered ? 1 : 0.5

    Keys.onPressed: {
        AutoTriggerTimer.cancelAutoTrigger();
    }
}
