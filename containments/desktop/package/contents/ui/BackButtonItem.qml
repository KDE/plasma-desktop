/*
    SPDX-FileCopyrightText: 2011-2013 Sebastian Kügler <sebas@kde.org>
    SPDX-FileCopyrightText: 2011 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2014-2015 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick

import org.kde.ksvg as KSvg
import org.kde.plasma.components as PlasmaComponents3
import org.kde.kirigami as Kirigami

DropArea {
    id: backButton

    property int iconSize
    property alias active: hoverActivateTimer.running
    property alias containsMouse: mouseArea.containsMouse

    signal backRequested()

    onEntered: hoverActivateTimer.restart();
    onExited: hoverActivateTimer.stop();

    MouseArea {
        id: mouseArea

        anchors.fill: parent

        acceptedButtons: Qt.LeftButton | Qt.BackButton
        hoverEnabled: true

        onPressed: mouse => {
            if (mouse.buttons & Qt.BackButton) {
                backButton.backRequested();
            }
        }

        onClicked: mouse => {
            if (mouse.button == Qt.BackButton) {
                return;
            }

            backButton.backRequested();
        }
    }

    KSvg.FrameSvgItem {
        id: background
        anchors.fill: parent
        imagePath: "widgets/viewitem"
    }

    Kirigami.Icon {
        id: icon

        anchors {
            left: parent.left
            leftMargin: Kirigami.Units.smallSpacing
            verticalCenter: parent.verticalCenter
        }

        width: backButton.iconSize
        height: backButton.iconSize

        source: "arrow-left"
    }

    PlasmaComponents3.Label {
        id: label

        anchors {
            left: icon.right
            leftMargin: Kirigami.Units.smallSpacing * 2
            verticalCenter: parent.verticalCenter
        }

        width:  parent.width - icon.width - (Kirigami.Units.smallSpacing * 4);

        height: undefined // Unset PlasmaComponents.Label's default.

        textFormat: Text.PlainText

        maximumLineCount: 1
        wrapMode: Text.Wrap
        elide: Text.ElideRight

        text: i18nc("@action:button", "Back")
    }

    Timer {
        id: hoverActivateTimer

        interval: root.hoverActivateDelay

        onTriggered: backButton.backRequested()
    }

    states: [
        State {
            name: "hover"
            when: mouseArea.containsMouse || backButton.containsDrag

            PropertyChanges {
                background.prefix: "hover"
            }
        }
    ]
}
