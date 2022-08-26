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
import QtQuick 2.15
import QtQml 2.15
import QtQuick.Layouts 1.15
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 3.0 as PC3

AbstractKickoffItemDelegate {
    id: root

    leftPadding: KickoffSingleton.listItemMetrics.margins.left
    rightPadding: KickoffSingleton.listItemMetrics.margins.right
    topPadding: PlasmaCore.Units.smallSpacing * 2
    bottomPadding: PlasmaCore.Units.smallSpacing * 2

    icon.width: PlasmaCore.Units.iconSizes.large
    icon.height: PlasmaCore.Units.iconSizes.large

    labelTruncated: label.truncated
    descriptionVisible: false

    dragIconItem: iconItem

    contentItem: ColumnLayout {
        spacing: root.spacing

        PlasmaCore.IconItem {
            id: iconItem
            implicitWidth: root.icon.width
            implicitHeight: root.icon.height
            Layout.alignment: Qt.AlignHCenter | Qt.AlignBottom

            animated: false
            usesPlasmaTheme: false

            source: root.decoration || root.icon.name || root.icon.source
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
        }
    }
}
