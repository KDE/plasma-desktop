/*
   Copyright (c) 2017 Marco Martin <mart@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

import QtQuick 2.5
import QtQuick.Controls 2.5 as QQC2
import QtQuick.Layouts 1.1

import org.kde.kirigami 2.5 as Kirigami

MouseArea {
    id: item
    property alias icon: iconItem.source
    property alias text: label.text
    property string module
    property int iconSize: Kirigami.Units.iconSizes.medium

    implicitWidth: Kirigami.Units.gridUnit * 5
    implicitHeight: Kirigami.Units.gridUnit * 4

    cursorShape: Qt.PointingHandCursor
    activeFocusOnTab: true
    hoverEnabled: true

    Accessible.role: Accessible.Button
    Accessible.name: label.text
    Accessible.description: i18n("Most used module number %1", index+1)
    Accessible.onPressAction: { item.clicked(model.kcmPlugin); }
    Keys.onReturnPressed: { item.clicked(model.kcmPlugin); }
    Keys.onEnterPressed: { item.clicked(model.kcmPlugin); }
    Keys.onSpacePressed: { item.clicked(model.kcmPlugin); }

    Kirigami.Separator {
        anchors {
            left: parent.left
            right: parent.right
            bottom: parent.bottom
        }
        visible: item.activeFocus
        color: Kirigami.Theme.highlightColor
    }
    ColumnLayout {
        anchors.fill: parent
        Kirigami.Icon {
            id: iconItem
            active: item.containsMouse || item.activeFocus
            Layout.alignment: Qt.AlignHCenter
            implicitWidth: item.iconSize
            implicitHeight: item.iconSize
        }
        QQC2.Label {
            id: label
            Layout.fillWidth: true
            Layout.fillHeight: true
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignTop
            wrapMode: Text.Wrap
        }
    }
}

