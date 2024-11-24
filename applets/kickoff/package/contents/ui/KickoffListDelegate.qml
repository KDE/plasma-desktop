/*
    SPDX-FileCopyrightText: 2011 Martin *Gräßlin <mgraesslin@kde.org>
    SPDX-FileCopyrightText: 2012 Gregor Taetzner <gregor@freenet.de>
    SPDX-FileCopyrightText: 2014 Sebastian Kügler <sebas@kde.org>
    SPDX-FileCopyrightText: 2015-2018 Eike Hein <hein@kde.org>
    SPDX-FileCopyrightText: 2021 Mikel Johnson <mikel5764@gmail.com>
    SPDX-FileCopyrightText: 2021 Noah Davis <noahadvs@gmail.com>
    SPDX-FileCopyrightText: 2022 Nate Graham <nate@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
 */

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import org.kde.ksvg as KSvg
import org.kde.plasma.components as PC3
import org.kde.kirigami as Kirigami
import org.kde.plasma.plasmoid

AbstractKickoffItemDelegate {
    id: root

    property bool compact: Kirigami.Settings.tabletMode ? false : Plasmoid.configuration.compactMode

    leftPadding: KickoffSingleton.listItemMetrics.margins.left
    + (mirrored ? KickoffSingleton.fontMetrics.descent : 0)
    rightPadding: KickoffSingleton.listItemMetrics.margins.right
    + (!mirrored ? KickoffSingleton.fontMetrics.descent : 0)
    // Otherwise it's *too* compact :)
    topPadding: compact ? Kirigami.Units.mediumSpacing : Kirigami.Units.smallSpacing
    bottomPadding: compact ? Kirigami.Units.mediumSpacing : Kirigami.Units.smallSpacing

    icon.width: compact || root.isCategoryListItem ? Kirigami.Units.iconSizes.smallMedium : Kirigami.Units.iconSizes.medium
    icon.height: compact || root.isCategoryListItem ? Kirigami.Units.iconSizes.smallMedium : Kirigami.Units.iconSizes.medium

    labelTruncated: label.truncated
    descriptionTruncated: descriptionLabel.truncated
    descriptionVisible: descriptionLabel.visible

    dragIconItem: icon

    contentItem: RowLayout {
        id: row
        spacing: KickoffSingleton.listItemMetrics.margins.left * 2

        Kirigami.Icon {
            id: icon
            implicitWidth: root.icon.width
            implicitHeight: root.icon.height
            Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter

            animated: false
            selected: root.iconAndLabelsShouldlookSelected

            // decorations provided by Kicker AbstractModel subclasses are strings, but can be paths or icon names.
            source: {
                const src = root.decoration || root.icon.name
                // make sure there aren't any characters that shouldn't be in
                // icon names and exclude icons that already end with "-symbolic"
                if (root.isCategoryListItem && src.length > 0
                    && src.search(/:|\/|\\|-symbolic$/g) === -1) {
                    return src + "-symbolic"
                }
                return src || root.icon.source
            }
        }

        GridLayout {
            id: gridLayout

            readonly property color textColor: root.iconAndLabelsShouldlookSelected ? Kirigami.Theme.highlightedTextColor : Kirigami.Theme.textColor

            Layout.fillWidth: true

            rows: root.compact ? 1 : 2
            columns: root.compact ? 2 : 1
            rowSpacing: 0
            columnSpacing: Kirigami.Units.largeSpacing

            PC3.Label {
                id: label
                Layout.fillWidth: !descriptionLabel.visible
                Layout.maximumWidth: root.width - root.leftPadding - root.rightPadding - icon.width - row.spacing
                Layout.preferredHeight: {
                    if (root.isCategoryListItem) {
                        return root.compact ? implicitHeight : Math.round(implicitHeight * 1.5);
                    }
                    if (!root.compact && !descriptionLabel.visible) {
                        return implicitHeight + descriptionLabel.implicitHeight
                    }
                    return implicitHeight;
                }
                text: root.text
                textFormat: root.isMultilineText ? Text.StyledText : Text.PlainText
                elide: Text.ElideRight
                wrapMode: root.isMultilineText ? Text.WordWrap : Text.NoWrap
                verticalAlignment: Text.AlignVCenter
                maximumLineCount: root.isMultilineText ? Infinity : 1
                color: gridLayout.textColor
            }

            PC3.Label {
                id: descriptionLabel
                Layout.fillWidth: true
                // Don't want to show descriptions for apps in the category list, because
                // there's not enough room for them with the category list item height
                // Also don't display if it's configured to show only name or only descriotion
                visible: text.length > 0 && text !== label.text && !root.isCategoryListItem && Plasmoid.configuration.appNameFormat > 1
                enabled: false
                text: root.description
                textFormat: Text.PlainText
                font: Kirigami.Theme.smallFont
                elide: Text.ElideRight
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: root.compact ? Text.AlignRight : Text.AlignLeft
                maximumLineCount: 1
                color: gridLayout.textColor
            }
        }
    }

    Loader {
        id: separatorLoader

        anchors.left: root.left
        anchors.right: root.right
        anchors.verticalCenter: root.verticalCenter

        active: root.isSeparator

        asynchronous: false
        sourceComponent: KSvg.SvgItem {
            width: parent.width
            height: KickoffSingleton.lineSvg.horLineHeight

            svg: KickoffSingleton.lineSvg
            elementId: "horizontal-line"
        }
    }
}
