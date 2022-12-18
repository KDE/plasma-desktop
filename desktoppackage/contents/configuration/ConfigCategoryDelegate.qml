/*
    SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2020 Carl Schwan <carlschwan@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.0
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.3 as QQC2
import QtQuick.Window 2.2

import org.kde.kquickcontrolsaddons 2.0
import org.kde.kirigami 2.5 as Kirigami

QQC2.ItemDelegate {
    id: delegate

    signal activated()

//BEGIN properties
    Layout.fillWidth: true
    Layout.maximumWidth: Kirigami.Units.gridUnit * 7
    hoverEnabled: true

    Accessible.role: Accessible.MenuItem
    Accessible.name: model.name
    Accessible.description: i18nd("plasma_shell_org.kde.plasma.desktop", "Open configuration page")

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
        spacing: Kirigami.Units.smallSpacing

        Kirigami.Icon {
            Layout.alignment: Qt.AlignHCenter
            implicitWidth: Kirigami.Units.iconSizes.medium
            implicitHeight: Kirigami.Units.iconSizes.medium
            source: model.icon
            selected: Window.active && (delegate.highlighted || delegate.pressed)
        }

        QQC2.Label {
            id: nameLabel
            Layout.fillWidth: true
            Layout.leftMargin: Kirigami.Units.smallSpacing
            Layout.rightMargin: Kirigami.Units.smallSpacing
            text: model.name
            wrapMode: Text.Wrap
            horizontalAlignment: Text.AlignHCenter
            color: Window.active && (delegate.highlighted || delegate.pressed) ? Kirigami.Theme.highlightedTextColor : Kirigami.Theme.textColor
            font.bold: delegate.highlighted && delegate.parent.activeFocus
        }
    }
//END UI components
}
