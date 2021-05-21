/*
    SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2020 Carl Schwan <carlschwan@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
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
            state: highlighted && Window.active ? QIconItem.SelectedState : QIconItem.DefaultState
        }

        QtControls.Label {
            id: nameLabel
            Layout.fillWidth: true
            Layout.leftMargin: PlasmaCore.Units.smallSpacing
            Layout.rightMargin: PlasmaCore.Units.smallSpacing
            text: model.name
            wrapMode: Text.Wrap
            horizontalAlignment: Text.AlignHCenter
            color: highlighted && Window.active ? Kirigami.Theme.highlightedTextColor : Kirigami.Theme.textColor
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
