/*
 * Copyright 2020 Noah Davis <noahadvs@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12 as QQC2
import org.kde.kirigami 2.12 as Kirigami

Kirigami.ShadowedRectangle {
    id: root

    property real itemHeight: Math.max(Kirigami.Units.iconSizes.smallMedium, 0)
    property real titleBarCornerRadius: 3 * Math.floor(Kirigami.Units.devicePixelRatio)
    
    implicitWidth: Kirigami.Units.gridUnit * 32
    implicitHeight: itemHeight + Kirigami.Units.smallSpacing*2
    
    corners{
        topLeftRadius: titleBarCornerRadius
        topRightRadius: titleBarCornerRadius
    }

    Kirigami.Theme.inherit: false
    //TODO: replace with titlebar colors
    Kirigami.Theme.colorSet: Kirigami.Theme.Header
    color: Kirigami.Theme.backgroundColor
    
    RowLayout {
        id: titlebarLayout
        anchors {
            fill: parent
            leftMargin: Math.round((root.height - root.itemHeight)/2)
            rightMargin: anchors.leftMargin
        }
        spacing: anchors.leftMargin

        Kirigami.Icon {
            id: appIcon
            Layout.alignment: Qt.AlignLeft
            width: Kirigami.Units.iconSizes.smallMedium
            height: width
            smooth: true
            source: "preferences-desktop-color"
        }
        Item { // Keeps the title perfectly centered
            width: appIcon.width
            Layout.fillHeight: true
        }
        Item { // Keeps the title perfectly centered
            width: appIcon.width
            Layout.fillHeight: true
        }
        QQC2.Label {
            id: title
            Layout.alignment: Qt.AlignHCenter
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            color: Kirigami.Theme.textColor
            text: i18n("Preview Window")
            elide: Text.ElideRight
        }
        Kirigami.Icon {
            id: minimizeIcon
            width: closeIcon.width
            height: Kirigami.Units.iconSizes.small
            smooth: true
            source: "window-minimize"
            color: Kirigami.Theme.textColor
        }
        Kirigami.Icon {
            id: maximizeIcon
            width: closeIcon.width
            height: minimizeIcon.height
            smooth: true
            source: "window-maximize"
            color: Kirigami.Theme.textColor
        }
        Kirigami.Icon {
            id: closeIcon
            Layout.alignment: Qt.AlignRight
            width: Kirigami.Units.iconSizes.smallMedium
            height: width
            smooth: true
            source: "window-close"
            color: mouseArea.containsMouse ? Kirigami.ColorUtils.linearInterpolation(Kirigami.Theme.negativeTextColor, "white", 0.2) : Kirigami.Theme.negativeTextColor
            
            MouseArea {
                id: mouseArea
                anchors.fill: closeIcon
                hoverEnabled: true
            }
        }
    }
}
