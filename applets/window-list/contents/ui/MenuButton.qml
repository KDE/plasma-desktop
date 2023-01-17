/*
 * SPDX-FileCopyrightText: 2022 Carson Black <uhhadd@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.10
import QtQuick.Layouts 1.10
import QtQuick.Controls 2.10
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 3.0 as PC3
import org.kde.kirigami 2.12 as Kirigami

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

        PlasmaCore.FrameSvgItem {
            id: rest
            anchors.fill: parent
            visible: !controlRoot.down && !controlRoot.hovered
            imagePath: "widgets/menubaritem"
            prefix: "normal"
        }
        PlasmaCore.FrameSvgItem {
            id: hover
            anchors.fill: parent
            visible: !controlRoot.down && controlRoot.hovered
            imagePath: "widgets/menubaritem"
            prefix: "hover"
        }
        PlasmaCore.FrameSvgItem {
            id: down
            anchors.fill: parent
            visible: controlRoot.down
            imagePath: "widgets/menubaritem"
            prefix: "pressed"
        }
    }

    contentItem: RowLayout {
        PlasmaCore.IconItem {
            id: iconItem
            visible: source !== "" && iconItem.valid

            implicitWidth: PlasmaCore.Units.roundToIconSize(label.implicitHeight)
            implicitHeight: PlasmaCore.Units.roundToIconSize(label.implicitHeight)

            Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
        }
        // Fall back to a generic icon if the application doesn't provide a valid one
        PlasmaCore.IconItem {
            visible: !iconItem.valid

            source: "preferences-system-windows"

            implicitWidth: PlasmaCore.Units.roundToIconSize(label.implicitHeight)
            implicitHeight: PlasmaCore.Units.roundToIconSize(label.implicitHeight)

            Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
        }
        PC3.Label {
            id: label
            visible: plasmoid.formFactor === PlasmaCore.Types.Horizontal && plasmoid.configuration.showText

            text: controlRoot.Kirigami.MnemonicData.richTextLabel

            verticalAlignment: Text.AlignVCenter
            Layout.fillHeight: true
        }
    }
}
