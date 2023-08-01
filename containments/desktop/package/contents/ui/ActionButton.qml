/*
    SPDX-FileCopyrightText: 2011 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2013 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Layouts 1.15

import org.kde.kirigami 2.20 as Kirigami
import org.kde.ksvg 1.0 as KSvg
import org.kde.plasma.components 3.0 as PC3

PC3.ToolButton {
    id: button

    property QtObject qAction

    property KSvg.Svg svg
    property alias elementId: icon.elementId
    readonly property int iconSize: Kirigami.Settings.hasTransientTouchInput
        ? Kirigami.Units.iconSizes.medium
        : Kirigami.Units.iconSizes.small

    property alias toolTip: toolTip.text

    implicitWidth: Math.min(buttonColumn.implicitWidth, Kirigami.Units.gridUnit * 10) + leftPadding + rightPadding

    Layout.fillWidth: true

    onClicked: {
        if (qAction) {
            qAction.trigger()
        }
        if (!plasmoid.containment.corona.editMode) {
            appletContainer.editMode = false;
        }
    }

    PC3.ToolTip {
        id: toolTip
        text: button.qAction ? button.qAction.text : ""
        delay: 0
        visible: button.hovered && text.length > 0
        Kirigami.Theme.colorSet: Kirigami.Theme.Window
        Kirigami.Theme.inherit: false
    }

    contentItem: ColumnLayout {
        id: buttonColumn

        KSvg.SvgItem {
            id: icon
            Layout.preferredWidth: iconSize
            Layout.preferredHeight: Layout.preferredWidth
            Layout.alignment: Qt.AlignHCenter
            svg: button.svg
        }

        PC3.Label {
            id: actionText
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter
            text: button.text
            wrapMode: Text.WordWrap
            elide: Text.ElideRight
            // The handle uses always the main global theme
            color: Kirigami.Theme.textColor
            visible: text.length > 0
        }
    }
}
