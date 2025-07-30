/*
 * SPDX-FileCopyrightText: 2022 Carson Black <uhhadd@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.10
import QtQuick.Layouts 1.10
import QtQuick.Controls 2.10
import org.kde.plasma.core as PlasmaCore
import org.kde.ksvg 1.0 as KSvg
import org.kde.plasma.components 3.0 as PC3
import org.kde.kirigami 2.20 as Kirigami
import org.kde.plasma.plasmoid 2.0

AbstractButton {
    id: controlRoot

    hoverEnabled: true

    Kirigami.MnemonicData.controlType: Kirigami.MnemonicData.SecondaryControl
    Kirigami.MnemonicData.label: controlRoot.text

    leftPadding: rest.margins.left
    rightPadding: rest.margins.right

    // needed since icon property can't hold QIcon, which the tasks manager gives us
    property alias iconSource: iconItem.source

    background: Item {
        id: background

        KSvg.FrameSvgItem {
            id: rest
            anchors.fill: parent
            visible: !controlRoot.down && !controlRoot.hovered
            imagePath: "widgets/menubaritem"
            prefix: "normal"
        }
        KSvg.FrameSvgItem {
            id: hover
            anchors.fill: parent
            visible: !controlRoot.down && controlRoot.hovered
            imagePath: "widgets/menubaritem"
            prefix: "hover"
        }
        KSvg.FrameSvgItem {
            id: down
            anchors.fill: parent
            visible: controlRoot.down
            imagePath: "widgets/menubaritem"
            prefix: "pressed"
        }
    }

    contentItem: RowLayout {
        Kirigami.Icon {
            id: iconItem
            visible: source !== "" && iconItem.valid

            implicitWidth: Kirigami.Units.iconSizes.sizeForLabels
            implicitHeight: Kirigami.Units.iconSizes.sizeForLabels

            Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
        }
        // Fall back to a generic icon if the application doesn't provide a valid one
        Kirigami.Icon {
            visible: !iconItem.valid

            source: "preferences-system-windows"

            implicitWidth: Kirigami.Units.iconSizes.sizeForLabels
            implicitHeight: Kirigami.Units.iconSizes.sizeForLabels

            Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
        }
        PC3.Label {
            id: label
            visible: Plasmoid.formFactor === PlasmaCore.Types.Horizontal && Plasmoid.configuration.showText

            text: controlRoot.Kirigami.MnemonicData.richTextLabel
            color: controlRoot.down || controlRoot.hovered ? Kirigami.Theme.highlightedTextColor : Kirigami.Theme.textColor
            verticalAlignment: Text.AlignVCenter
            Layout.fillHeight: true
        }
    }
}
