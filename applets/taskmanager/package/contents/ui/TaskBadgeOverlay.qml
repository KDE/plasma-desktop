/***************************************************************************
 *   Copyright (C) 2016 Kai Uwe Broulik <kde@privat.broulik.de>            *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

import QtQuick 2.4

import org.kde.plasma.components 2.0 as PlasmaComponents

Item {
    readonly property int iconWidthDelta: (icon.width - icon.paintedWidth) / 2

    Item {
        id: badgeMask
        anchors.fill: parent

        Rectangle {
            readonly property int offset: Math.round(Math.max(units.smallSpacing / 2, badgeMask.width / 32))
            x: Qt.application.layoutDirection === Qt.RightToLeft ? -offset + iconWidthDelta : parent.width - width + offset - iconWidthDelta
            y: -offset
            width: badgeRect.width + offset * 2
            height: badgeRect.height + offset * 2
            radius: width
        }
    }

    ShaderEffect {
        anchors.fill: parent
        property var source: ShaderEffectSource {
            sourceItem: icon
            hideSource: true
        }
        property var mask: ShaderEffectSource {
            sourceItem: badgeMask
            hideSource: true
            live: false
        }

        onWidthChanged: mask.scheduleUpdate()
        onHeightChanged: mask.scheduleUpdate()

        supportsAtlasTextures: true

        fragmentShader: "
            varying highp vec2 qt_TexCoord0;
            uniform highp float qt_Opacity;
            uniform lowp sampler2D source;
            uniform lowp sampler2D mask;
            void main() {
                gl_FragColor = texture2D(source, qt_TexCoord0.st) * (1.0 - (texture2D(mask, qt_TexCoord0.st).a)) * qt_Opacity;
            }
        "
    }

    Rectangle {
        id: badgeRect
        x: Qt.application.layoutDirection === Qt.RightToLeft ? iconWidthDelta : parent.width - width - iconWidthDelta
        width: height
        height: Math.round(parent.height * 0.4)
        color: theme.highlightColor
        radius: width

        PlasmaComponents.Label {
            anchors.centerIn: parent
            width: height
            height: Math.round(parent.height)
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            fontSizeMode: Text.Fit
            font.pointSize: 1024
            minimumPointSize: 5
            color: theme.backgroundColor
            text: task.smartLauncherItem.count
        }
    }
}
