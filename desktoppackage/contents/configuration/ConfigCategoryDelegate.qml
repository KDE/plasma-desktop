/*
 *  Copyright 2013 Marco Martin <mart@kde.org>
 *  Copyright 2020 Carl Schwan <carlschwan@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  2.010-1301, USA.
 */

import QtQuick 2.0
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.3 as QtControls
import QtQuick.Window 2.2
import org.kde.plasma.core 2.0 as PlasmaCore

import org.kde.kquickcontrolsaddons 2.0
import org.kde.kirigami 2.5 as Kirigami

QtControls.ItemDelegate {
    id: delegate

    signal activated()

//BEGIN properties
    Layout.fillWidth: true
    Layout.maximumWidth: Kirigami.Units.gridUnit * 7
    hoverEnabled: true

    Accessible.role: Accessible.MenuItem
    Accessible.name: model.name
    Accessible.description: i18n("Open configuration page")

    property var item
//END properties

//BEGIN connections
    onClicked: {
        if (highlighted) {
            return;
        }

        activated()
    }
//END connections

//BEGIN UI components
    contentItem: ColumnLayout {
        id: delegateContents
        spacing: PlasmaCore.Units.smallSpacing

        QIconItem {
            id: iconItem
            Layout.alignment: Qt.AlignHCenter
            width: PlasmaCore.Units.iconSizes.medium
            height: width
            icon: model.icon
            state: highlighted && categoriesScroll.activeFocus ? QIconItem.SelectedState : QIconItem.DefaultState
        }

        QtControls.Label {
            id: nameLabel
            Layout.fillWidth: true
            Layout.leftMargin: PlasmaCore.Units.smallSpacing
            Layout.rightMargin: PlasmaCore.Units.smallSpacing
            text: model.name
            wrapMode: Text.Wrap
            horizontalAlignment: Text.AlignHCenter
            color: highlighted && categoriesScroll.activeFocus ? Kirigami.Theme.highlightedTextColor : Kirigami.Theme.textColor
            Behavior on color {
                ColorAnimation {
                    duration: PlasmaCore.Units.longDuration
                    easing.type: Easing.InOutQuad
                }
            }
        }
    }
//END UI components
}
