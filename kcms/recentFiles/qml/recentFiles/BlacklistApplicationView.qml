/*   vim:set foldenable foldmethod=marker:

    SPDX-FileCopyrightText: 2012 Ivan Cukic <ivan.cukic@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.5
import QtQuick.Controls 2.5 as QQC2

import org.kde.kirigami 2.5 as Kirigami

QQC2.ScrollView {
    enabled: applicationModel.enabled
    Component.onCompleted: background.visible = true;

    GridView {
        id: gridView
        clip: true
        cellHeight: Kirigami.Units.gridUnit * 5
        cellWidth: Kirigami.Units.gridUnit * 9
        model: applicationModel
        delegate: Item {
            height: gridView.cellHeight
            width: gridView.cellWidth

            Rectangle {
                anchors.fill: parent
                visible: mouseArea.containsMouse
                color: Kirigami.Theme.hoverColor
            }

            Kirigami.Icon {
                id: icon
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.bottom: parent.verticalCenter
                height: Kirigami.Units.iconSizes.medium
                width: height
                source: model.icon
                opacity: model.blocked ? 0.6 : 1.0

                Behavior on opacity { NumberAnimation { duration: Kirigami.Units.shortDuration } }
            }

            Kirigami.Icon {
                anchors.bottom: icon.bottom
                anchors.right: icon.right
                height: Kirigami.Units.iconSizes.small
                width: height
                source: "emblem-unavailable"
                opacity: model.blocked ? 1.0 : 0.0

                Behavior on opacity { NumberAnimation { duration: Kirigami.Units.shortDuration } }
            }

            QQC2.Label {
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.top: parent.verticalCenter
                width: parent.width - 20
                text: model.title
                horizontalAlignment: Text.AlignHCenter
                elide: Text.ElideRight
                maximumLineCount: 2
                wrapMode: Text.Wrap
                opacity: model.blocked ? 0.6 : 1.0

                Behavior on opacity { NumberAnimation { duration: Kirigami.Units.shortDuration } }
            }

            MouseArea {
                id: mouseArea
                anchors.fill: parent
                hoverEnabled: true
                onClicked: applicationModel.toggleApplicationBlocked(model.index)
            }
        }
    }
}
