/*
    SPDX-FileCopyrightText: 2020 Tobias Fella <fella@posteo.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.12
import QtQuick.Controls 2.12 as QQC2

import org.kde.kirigami 2.7 as Kirigami
import org.kde.kcmutils as KCM

QQC2.ComboBox {
    id: comboBox
    
    property string label
    property var component
    
    Kirigami.FormData.label: label
    model: component.applications
    textRole: "name"
    currentIndex: component.index
    onActivated: component.select(currentIndex, true)

    // HACK QQC2 doesn't support icons, so we just tamper with the desktop style ComboBox's background
    function loadProps(background) {
        if (!background || !background.hasOwnProperty("properties")) {
            //not a KQuickStyleItem
            return;
        }

        var props = background.properties || {};

        background.properties = Qt.binding(function() {
            var modelIndex = model.index(currentIndex, 0);
            var newProps = props;
            newProps.currentIcon = model.data(modelIndex, 0x0101 /* ApplicationModel::Roles::Icon */);
            newProps.iconColor = Kirigami.Theme.textColor;
            return newProps;
        });
    }

    Connections {
        target: component
        function onIndexChanged() {
            loadProps(background);
        }
    }

    delegate: QQC2.ItemDelegate {
        width: comboBox.popup.width;
        text: model.name
        highlighted: index == currentIndex
        icon.name: model.icon
    }

    Component.onCompleted: loadProps(background)

}
