/*
    SPDX-FileCopyrightText: 2020 Tobias Fella <fella@posteo.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

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
