/***************************************************************************
 *   Copyright (C) 2011-2013 Sebastian Kügler <sebas@kde.org>              *
 *   Copyright (C) 2011 Marco Martin <mart@kde.org>                        *
 *   Copyright (C) 2014-2015 by Eike Hein <hein@kde.org>                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

import QtQuick 2.0
import org.kde.plasma.plasmoid 2.0

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.kquickcontrolsaddons 2.0

PlasmaCore.FrameSvgItem {
    id: upButton

    width: gridView.cellWidth
    height: visible ? gridView.cellHeight : 0

    visible: dir.resolvedUrl != dir.resolve(plasmoid.configuration.url)

    imagePath: "widgets/viewitem"

    MouseArea {
        id: mouseArea

        anchors.fill: parent

        hoverEnabled: true

        onContainsMouseChanged: {
            gridView.hoveredItem = null;
        }

        onClicked: {
            if (systemSettings.singleClick()) {
                dir.up();
            }
        }

        onDoubleClicked: {
            if (!systemSettings.singleClick()) {
                dir.up();
            }
        }
    }

    QIconItem {
        id: icon

        anchors {
            left: parent.left
            leftMargin: units.smallSpacing
            verticalCenter: parent.verticalCenter
        }

        width: gridView.iconSize
        height: gridView.iconSize

        icon: "arrow-up"
    }

    PlasmaComponents.Label {
        id: label

        anchors {
            left: icon.right
            leftMargin: units.smallSpacing * 2
            verticalCenter: parent.verticalCenter
        }

        width:  parent.width - icon.width - (units.smallSpacing * 4);

        height: undefined // Unset PlasmaComponents.Label's default.

        textFormat: Text.PlainText

        maximumLineCount: root.isPopup ? 1 : plasmoid.configuration.textLines
        wrapMode: Text.Wrap
        elide: Text.ElideRight

        text: i18n("Up")
    }

    states: [
        State {
            name: "hover"
            when: mouseArea.containsMouse

            PropertyChanges {
                target: upButton
                prefix: "hover"
            }
        }
    ]
}
