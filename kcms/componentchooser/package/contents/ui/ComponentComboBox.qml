/***************************************************************************
 *   Copyright (C) 2020 Tobias Fella <fella@posteo.de>                     *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA          *
 ***************************************************************************/

import QtQuick 2.12
import QtQuick.Controls 2.12 as Controls

import org.kde.kirigami 2.7 as Kirigami

Controls.ComboBox {
    id: comboBox
    
    property string label
    property var component
    
    Kirigami.FormData.label: label
    model: component.applications
    textRole: "name"
    currentIndex: component.index
    onActivated: component.select(currentIndex, true)
    
    delegate: Controls.ItemDelegate {
        width: comboBox.popup.width
        text: modelData[comboBox.textRole]
        highlighted: comboBox.highlightedIndex == index
        icon.name: modelData.icon
    }
    
    // HACK QQC2 doesn't support icons, so we just tamper with the desktop style ComboBox's background
    Component.onCompleted: {
        if (!background || !background.hasOwnProperty("properties")) {
            //not a KQuickStyleItem
            return;
        }

        var props = background.properties || {};

        background.properties = Qt.binding(function() {
            var newProps = props;
            newProps.currentIcon = component.applications[currentIndex]["icon"];
            newProps.iconColor = Kirigami.Theme.textColor;
            return newProps;
        });
    }
}
