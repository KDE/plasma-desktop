/*
    SPDX-FileCopyrightText: 2016 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts

import org.kde.kirigami as Kirigami

import org.kde.breeze.components
import "timer.js" as AutoTriggerTimer

ActionButton {
    // otherwise the Button color group is used
    // and text legibility is broken with light themes
    // which set their own color scheme
    Kirigami.Theme.inherit: false
    Kirigami.Theme.colorSet: Kirigami.Theme.Complementary

    Layout.alignment: Qt.AlignTop

    icon.width: Kirigami.Units.iconSizes.huge
    icon.height: Kirigami.Units.iconSizes.huge

    font.underline: false // See https://phabricator.kde.org/D9452
    opacity: activeFocus || hovered ? 1 : 0.75

    Keys.onPressed: {
        AutoTriggerTimer.cancelAutoTrigger();
    }
}
