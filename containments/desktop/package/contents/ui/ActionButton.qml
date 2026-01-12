/*
    SPDX-FileCopyrightText: 2011 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2013 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick

import org.kde.plasma.plasmoid
import org.kde.plasma.core as PlasmaCore
import org.kde.kirigami as Kirigami
import org.kde.plasma.components as PC3

PC3.ToolButton {
    id: button
    flat: false
    
    property PlasmaCore.Action qAction
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
