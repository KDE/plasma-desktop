/***************************************************************************
 *   Copyright 2012 by Sebastian Kügler <sebas@kde.org>                    *
 *   Copyright 2015 by Kai Uwe Broulik <kde@privat.broulik.de>             *
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
import QtQuick.Layouts 1.1

import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.kquickcontrolsaddons 2.0 as KQuickControlsAddons

MouseArea {
    id: delegate
    property alias icon: iconItem.icon
    property alias text: textLabel.text

    height: labelRow.implicitHeight + units.smallSpacing * 2
    hoverEnabled: true
    onClicked: {
        if (typeof modelData !== "undefined") {
            modelData.trigger()
        }
        toolBoxLoader.item.visible = false
    }
    onEntered: {
        exitTimer.stop()
        actionsColumn.currentItem = delegate
    }
    onExited: exitTimer.restart()

    RowLayout {
        id: labelRow
        anchors {
            left: parent.left
            right: parent.right
            margins: units.smallSpacing
            verticalCenter: parent.verticalCenter
        }
        spacing: units.smallSpacing

        KQuickControlsAddons.QIconItem {
            id: iconItem
            width: units.iconSizes.medium
            height: width
        }

        PlasmaComponents.Label {
            id: textLabel
            Layout.fillWidth: true
        }
    }
}
