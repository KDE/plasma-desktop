/*
    Copyright (C) 2011  Martin *Gräßlin <mgraesslin@kde.org>
    Copyright (C) 2012  Gregor Taetzner <gregor@freenet.de>
    Copyright 2014 Sebastian Kügler <sebas@kde.org>
    Copyright (C) 2015-2018  Eike Hein <hein@kde.org>
    Copyright (C) 2021 by Mikel Johnson <mikel5764@gmail.com>
    Copyright (C) 2021 by Noah Davis <noahadvs@gmail.com>
    Copyright (C) 2022 Nate Graham <nate@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
import QtQuick 2.15
import QtQml 2.15
import QtQuick.Layouts 1.15
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 3.0 as PC3
import org.kde.kirigami 2.16 as Kirigami

AbstractKickoffItemDelegate {
    id: root

    property bool compact: Kirigami.Settings.tabletMode ? false : plasmoid.configuration.compactMode

    leftPadding: KickoffSingleton.listItemMetrics.margins.left
    + (mirrored ? KickoffSingleton.fontMetrics.descent : 0)
    rightPadding: KickoffSingleton.listItemMetrics.margins.right
    + (!mirrored ? KickoffSingleton.fontMetrics.descent : 0)
    // Otherwise it's *too* compact :)
    topPadding: compact ? PlasmaCore.Units.mediumSpacing : PlasmaCore.Units.smallSpacing
    bottomPadding: compact ? PlasmaCore.Units.mediumSpacing : PlasmaCore.Units.smallSpacing

    icon.width: compact || root.isCategory ? PlasmaCore.Units.iconSizes.smallMedium : PlasmaCore.Units.iconSizes.medium
    icon.height: compact || root.isCategory ? PlasmaCore.Units.iconSizes.smallMedium : PlasmaCore.Units.iconSizes.medium

    labelTruncated: label.truncated
    descriptionTruncated: descriptionLabel.truncated
    descriptionVisible: descriptionLabel.visible

    contentItem: RowLayout {
        id: row
        spacing: KickoffSingleton.listItemMetrics.margins.left * 2

        PlasmaCore.IconItem {
            id: icon
            implicitWidth: root.icon.width
            implicitHeight: root.icon.height
            Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter

            animated: false
            usesPlasmaTheme: false

            source: root.decoration || root.icon.name || root.icon.source
        }

        GridLayout {
            id: gridLayout
            Layout.fillWidth: true

            rows: root.compact ? 1 : 2
            columns: root.compact ? 2 : 1
            rowSpacing: 0
            columnSpacing: root.spacing

            PC3.Label {
                id: label
                Layout.fillWidth: !descriptionLabel.visible
                Layout.maximumWidth: root.width - root.leftPadding - root.rightPadding - icon.width - row.spacing
                Layout.preferredHeight: {
                    if (root.isCategory) {
                        return root.compact ? implicitHeight : Math.round(implicitHeight * 1.5);
                    }
                    if (!root.compact && !descriptionLabel.visible) {
                        return implicitHeight * 2;
                    }
                    return implicitHeight;
                }
                text: root.text
                elide: Text.ElideRight
                verticalAlignment: Text.AlignVCenter
                maximumLineCount: 1
            }

            PC3.Label {
                id: descriptionLabel
                Layout.fillWidth: true
                visible: text.length > 0 && text !== root.text
                enabled: false
                text: root.description
                font: PlasmaCore.Theme.smallestFont
                elide: Text.ElideRight
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: root.compact ? Text.AlignRight : Text.AlignLeft
                maximumLineCount: 1
            }
        }
    }
}
