/*
 *   Copyright 2011 Marco Martin <mart@kde.org>
 *   Copyright 2013 Sebastian Kügler <sebas@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Library General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

import QtQuick 2.12
import QtQuick.Layouts 1.4

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 3.0 as PlasmaComponents

PlasmaComponents.ToolButton {
    id: button
    Layout.fillWidth: true

    property QtObject qAction

    property PlasmaCore.Svg svg
    property alias elementId: icon.elementId
    property int iconSize: 32
    property alias toolTip: toolTip.text

    
    implicitWidth: Math.min(buttonColumn.implicitWidth, units.gridUnit * 10) + leftPadding + rightPadding

    onClicked: {
        if (qAction) {
            qAction.trigger()
        }
        if (!plasmoid.editMode) {
            appletContainer.editMode = false;
        }
    }


    PlasmaComponents.ToolTip {
        id: toolTip
        text: button.qAction ? button.qAction.text : ""
        delay: 0
        visible: button.hovered && text.length > 0
        x: button.width
        y: button.height/2 - height/2
        PlasmaCore.ColorScope.colorGroup: PlasmaCore.Theme.NormalColorGroup
        PlasmaCore.ColorScope.inherit: false
    }
    contentItem: ColumnLayout {
        id: buttonColumn

        PlasmaCore.SvgItem {
            id: icon
            Layout.preferredWidth: units.iconSizes.small
            Layout.preferredHeight: Layout.preferredWidth
            Layout.alignment: Qt.AlignHCenter
            width: iconSize
            height: iconSize
            svg: button.svg
        }

        PlasmaComponents.Label {
            id: actionText
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter
            text: button.text
            wrapMode: Text.WordWrap
            elide: Text.ElideRight
            // The handle uses always the main global theme
            color: theme.textColor
            visible: text.length > 0
        }
    }
}
