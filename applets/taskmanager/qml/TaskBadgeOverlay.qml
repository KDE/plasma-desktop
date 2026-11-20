/*
    SPDX-FileCopyrightText: 2016 Kai Uwe Broulik <kde@privat.broulik.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import org.kde.kirigami as Kirigami
import org.kde.graphicaleffects as KGraphicalEffects
import org.kde.plasma.plasmoid

Item {
    id: root

    property alias badgeAnchors: badgeRect.anchors
    property alias badgeText: badgeRect.text
    property alias badgeVisible: badgeRect.visible
    property alias badgeType: badgeRect.type
    property alias badgeFont: badgeRect.font

    readonly property bool verySmallIcon:
        icon.width < Kirigami.Units.iconSizes.smallMedium ||
        icon.height < Kirigami.Units.iconSizes.smallMedium
    readonly property bool shiftBadgeDown: (Plasmoid.pluginName === "org.kde.plasma.icontasks") && task.audioStreamIcon !== null
    readonly property int badgeMaskY: shiftBadgeDown ? root.height - badgeRect.height : 0
    readonly property int offset: Math.round(Math.max(Kirigami.Units.smallSpacing / 2, badgeMask.width / 32))
    readonly property int extraHorizontalOffset: verySmallIcon && badgeRect.text.length > 2 ? -2 : 0


    // The small font is likely to not be small enough here for very space-constrained
    // panels; allow going this much under it.
    readonly property real smallestFractionOfSmallFontSize: 0.75
    // If the actual height is less, scale the font down. If it's greater, scale it up.
    readonly property int iconSizeForDefaultFontSize: Kirigami.Units.iconSizes.large
    readonly property real badgeScaleFactor: Math.max(smallestFractionOfSmallFontSize, icon.height / iconSizeForDefaultFontSize)

    Item {
        id: badgeMask
        anchors.fill: parent

        Rectangle {

            anchors.right: parent.right
            anchors.rightMargin: -root.offset + root.extraHorizontalOffset
            y: root.badgeMaskY

            Behavior on y {
                NumberAnimation { duration: Kirigami.Units.longDuration }
            }

            visible: task.smartLauncherItem.countVisible
            width: badgeRect.width + root.offset * 2
            height: badgeRect.height + root.offset * 2
            radius: height

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

    KGraphicalEffects.BadgeEffect {
        id: shader

        anchors.fill: parent
        source: iconShaderSource
        mask: maskShaderSource

        onWidthChanged: maskShaderSource.scheduleUpdate()
        onHeightChanged: maskShaderSource.scheduleUpdate()
    }

    Kirigami.Badge {
        id: badgeRect

        y: root.badgeMaskY + root.offset

        Behavior on y {
            NumberAnimation { duration: Kirigami.Units.longDuration }
        }

        padding: Math.floor(root.badgeScaleFactor * 2)

        font.pointSize: Math.floor((Kirigami.Theme.smallFont.pointSize * root.badgeScaleFactor))
    }
}
