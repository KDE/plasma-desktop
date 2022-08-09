/*
    SPDX-FileCopyrightText: 2011 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2013 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Layouts 1.15

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 3.0 as PC3

PC3.ToolButton {
    id: button

    property QtObject qAction

    property PlasmaCore.Svg svg
    property alias elementId: icon.elementId
    property int iconSize: 32
    property alias toolTip: toolTip.text

    implicitWidth: Math.min(buttonColumn.implicitWidth, PlasmaCore.Units.gridUnit * 10) + leftPadding + rightPadding

    Layout.fillWidth: true

    onClicked: {
        if (qAction) {
            qAction.trigger()
        }
        if (!plasmoid.editMode) {
            appletContainer.editMode = false;
        }
    }


    PC3.ToolTip {
        id: toolTip
        text: button.qAction ? button.qAction.text : ""
        delay: 0
        visible: button.hovered && text.length > 0
        PlasmaCore.ColorScope.colorGroup: PlasmaCore.Theme.NormalColorGroup
        PlasmaCore.ColorScope.inherit: false
    }
    contentItem: ColumnLayout {
        id: buttonColumn

        PlasmaCore.SvgItem {
            id: icon
            Layout.preferredWidth: PlasmaCore.Units.roundToIconSize(button.iconSize)
            Layout.preferredHeight: Layout.preferredWidth
            Layout.alignment: Qt.AlignHCenter
            svg: button.svg
        }

        PC3.Label {
            id: actionText
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter
            text: button.text
            wrapMode: Text.WordWrap
            elide: Text.ElideRight
            // The handle uses always the main global theme
            color: PlasmaCore.Theme.textColor
            visible: text.length > 0
        }
    }
}
