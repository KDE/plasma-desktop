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
import org.kde.plasma.components as PC3
import org.kde.kirigami as Kirigami
import org.kde.plasma.plasmoid

AbstractKickoffItemDelegate {
    id: root

    leftPadding: KickoffSingleton.listItemMetrics.margins.left
    rightPadding: KickoffSingleton.listItemMetrics.margins.right
    topPadding: Kirigami.Units.smallSpacing * 2
    bottomPadding: Kirigami.Units.smallSpacing * 2

    icon.width: Kirigami.Units.iconSizes.large
    icon.height: Kirigami.Units.iconSizes.large

    labelTruncated: label.truncated
    descriptionVisible: false

    dragIconItem: iconItem

    contentItem: ColumnLayout {
        spacing: root.spacing

        Kirigami.Icon {
            id: iconItem
            implicitWidth: root.icon.width
            implicitHeight: root.icon.height
            Layout.alignment: Qt.AlignHCenter | Qt.AlignBottom

            animated: false
            selected: root.iconAndLabelsShouldlookSelected
            source: root.decoration || root.icon.name || root.icon.source

            Loader {
                anchors {
                    right: parent.right
                    rightMargin: -root.rightPadding
                    top: parent.top
                }
                visible: active
                active: root.model?.isNewlyInstalled ?? false

                sourceComponent: Badge {
                    text: i18nc("Newly installed app, badge, keep short", "New!")
                    Accessible.name: i18n("Newly installed application")
                }
            }
        }

        PC3.Label {
            id: label
            Layout.alignment: Qt.AlignHCenter | Qt.AlignTop
            Layout.fillWidth: true
            Layout.preferredHeight: label.lineCount === 1 ? label.implicitHeight * 2 : label.implicitHeight

            text: root.text
            textFormat: Text.PlainText
            elide: Text.ElideRight
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignTop
            maximumLineCount: 2
            wrapMode: Text.Wrap
            color: root.iconAndLabelsShouldlookSelected ? Kirigami.Theme.highlightedTextColor : Kirigami.Theme.textColor
        }
    }
}
