/*
    SPDX-FileCopyrightText: 2011 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2013 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Layouts 1.15

import org.kde.plasma.plasmoid 2.0
import org.kde.kirigami 2.20 as Kirigami
import org.kde.plasma.components 3.0 as PC3

PC3.ToolButton {
    id: button

    property QtObject qAction
    readonly property int iconSize: Kirigami.Settings.hasTransientTouchInput
        ? Kirigami.Units.iconSizes.medium
        : Kirigami.Units.iconSizes.small

    property alias toolTip: toolTip.text

    onClicked: {
        if (qAction) {
            qAction.trigger()
        }
        if (!Plasmoid.containment.corona.editMode) {
            appletContainer.editMode = false;
        }
    }

    icon.width: iconSize
    icon.height: iconSize

    PC3.ToolTip {
        id: toolTip
        text: button.qAction ? button.qAction.text : ""
        delay: 0
        visible: button.hovered && text.length > 0
        Kirigami.Theme.colorSet: Kirigami.Theme.Window
        Kirigami.Theme.inherit: false
    }
}
