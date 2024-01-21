/*
    SPDX-FileCopyrightText: 2020 Tobias Fella <fella@posteo.de>
    SPDX-FileCopyrightText: 2023 ivan tkachenko <me@ratijas.tk>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kitemmodels as KItemModels

QQC2.ComboBox {
    id: comboBox

    property var component

    model: component.applications
    textRole: "name"
    currentIndex: component.index

    onActivated: component.select(currentIndex)

    // HACK QQC2 doesn't support icons, so we just tamper with the desktop style ComboBox's background
    function loadProps() {
        if (!background || !background.hasOwnProperty("properties")) {
            //not a KQuickStyleItem
            return;
        }

        const props = background.properties || {};

        background.properties = Qt.binding(() => {
            const modelIndex = model.index(currentIndex, 0);
            const currentIcon = model.data(modelIndex, model.KItemModels.KRoleNames.role("icon"));
            return Object.assign(props, {
                currentIcon,
                iconColor: Kirigami.Theme.textColor,
            });
        });
    }

    Connections {
        target: comboBox.component
        function onIndexChanged() {
            comboBox.loadProps();
        }
    }

    delegate: QQC2.ItemDelegate {
        width: ListView.view.width
        text: model.name
        highlighted: comboBox.highlightedIndex == index
        icon.name: model.icon
    }

    Component.onCompleted: loadProps()
}
