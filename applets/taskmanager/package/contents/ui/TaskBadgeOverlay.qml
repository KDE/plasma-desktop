/*
    SPDX-FileCopyrightText: 2016 Kai Uwe Broulik <kde@privat.broulik.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15

import org.kde.plasma.core 2.0 as PlasmaCore

Item {
    readonly property int iconWidthDelta: (icon.width - icon.paintedWidth) / 2
    readonly property bool shiftBadgeDown: (plasmoid.pluginName === "org.kde.plasma.icontasks") && task.audioStreamIcon !== null

    Item {
        id: badgeMask
        anchors.fill: parent

        Rectangle {
            readonly property int offset: Math.round(Math.max(PlasmaCore.Units.smallSpacing / 2, badgeMask.width / 32))
            anchors.right: Qt.application.layoutDirection === Qt.RightToLeft ? undefined : parent.right
            anchors.left: Qt.application.layoutDirection === Qt.RightToLeft ? parent.left : undefined
            anchors.rightMargin: Qt.application.layoutDirection === Qt.RightToLeft ? 0 : -offset
            anchors.leftMargin: Qt.application.layoutDirection === Qt.RightToLeft ? -offset : 0
            y: shiftBadgeDown ? (icon.height/2) : 0
            Behavior on y {
                NumberAnimation { duration: PlasmaCore.Units.longDuration }
            }

            visible: task.smartLauncherItem.countVisible
            width: badgeRect.width + offset * 2
            height: badgeRect.height + offset * 2
            radius: badgeRect.radius + offset * 2

            // Badge changes width based on number.
            onWidthChanged: maskShaderSource.scheduleUpdate()
            onVisibleChanged: maskShaderSource.scheduleUpdate()
            onYChanged: maskShaderSource.scheduleUpdate()
        }
    }

    ShaderEffectSource {
        id: iconShaderSource
        sourceItem: icon
        hideSource: GraphicsInfo.api !== GraphicsInfo.Software
    }

    ShaderEffectSource {
        id: maskShaderSource
        sourceItem: badgeMask
        hideSource: true
        live: false
    }

    ShaderEffect {
        id: shader

        anchors.fill: parent
        property var source: iconShaderSource
        property var mask: maskShaderSource

        onWidthChanged: maskShaderSource.scheduleUpdate()
        onHeightChanged: maskShaderSource.scheduleUpdate()

        supportsAtlasTextures: true

        fragmentShader: `
            varying highp vec2 qt_TexCoord0;
            uniform highp float qt_Opacity;
            uniform lowp sampler2D source;
            uniform lowp sampler2D mask;
            void main() {
                gl_FragColor = texture2D(source, qt_TexCoord0.st) * (1.0 - (texture2D(mask, qt_TexCoord0.st).a)) * qt_Opacity;
            }
        `
    }

    Badge {
        readonly property int offset: Math.round(Math.max(PlasmaCore.Units.smallSpacing / 2, badgeMask.width / 32))
        id: badgeRect
        anchors.right: Qt.application.layoutDirection === Qt.RightToLeft ? undefined : parent.right
        anchors.left: Qt.application.layoutDirection === Qt.RightToLeft ? parent.left : undefined
        y: offset + (shiftBadgeDown ? (icon.height/2) : 0)
        Behavior on y {
            NumberAnimation { duration: PlasmaCore.Units.longDuration }
        }
        height: Math.round(parent.height * 0.4)
        visible: task.smartLauncherItem.countVisible
        number: task.smartLauncherItem.count
    }
}
