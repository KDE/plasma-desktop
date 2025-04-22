/*
    SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2020 Carl Schwan <carlschwan@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import QtQuick.Window

import org.kde.kquickcontrolsaddons
import org.kde.kirigami as Kirigami

QQC2.ItemDelegate {
    id: delegate

    signal activated()

//BEGIN properties
    Layout.fillWidth: true
    hoverEnabled: true

    Accessible.role: Accessible.PageTab
    Accessible.name: model.name
    Accessible.description: i18nd("plasma_shell_org.kde.plasma.desktop", "Open configuration page")
    Accessible.onPressAction: delegate.clicked()

    focus: highlighted // need to actually focus highlighted items for the screen reader to see them

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
            textFormat: Text.PlainText
            wrapMode: Text.Wrap
            horizontalAlignment: Text.AlignHCenter
            color: Window.active && (delegate.highlighted || delegate.pressed) ? Kirigami.Theme.highlightedTextColor : Kirigami.Theme.textColor
            font.bold: delegate.highlighted && delegate.parent.activeFocus
            Accessible.ignored: true
        }
    }
//END UI components
}
