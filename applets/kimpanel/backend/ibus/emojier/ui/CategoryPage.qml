/*
 *  Copyright (C) 2019 Aleix Pol Gonzalez <aleixpol@kde.org>
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License as
 *  published by the Free Software Foundation; either version 2 of
 *  the License or (at your option) version 3 or any later version
 *  accepted by the membership of KDE e.V. (or its successor approved
 *  by the membership of KDE e.V.), which shall act as a proxy
 *  defined in Section 14 of version 3 of the license.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.11
import QtQuick.Layouts 1.3
import org.kde.kirigami 2.6 as Kirigami
import QtQuick.Controls 2.11 as QQC2
import org.kde.plasma.emoji 1.0

Kirigami.ScrollablePage
{
    id: view
    property alias model: emojiModel.sourceModel
    property alias category: filter.category
    leftPadding: 0
    rightPadding: 0

    actions.main: Kirigami.Action {
        icon.name: "search"
        tooltip: i18n("Search...")
        shortcut: StandardKey.Find
        onTriggered: {
            checked = !checked
        }
    }

    titleDelegate: RowLayout {
        Layout.fillWidth: true
        Layout.preferredWidth: view.width
        Kirigami.Heading {
            text: view.title
            Layout.fillWidth: true
        }

        QQC2.TextField {
            id: searchField
            Layout.fillWidth: true
            placeholderText: i18n("Search...")
            onTextChanged: {
                emojiModel.search = text
                if (emojiView.currentIndex < 0) {
                    emojiView.currentIndex = 0
                }
            }
            onAccepted: {
                if (emojiView.currentItem)
                    emojiView.currentItem.reportEmoji()
            }
            visible: view.actions.main.checked
            onVisibleChanged: if (visible) forceActiveFocus()
            Keys.onEscapePressed: {
                text = ""
                view.actions.main.checked = false
            }
        }
    }

    GridView {
        id: emojiView

        readonly property real desiredSize: Kirigami.Units.gridUnit * 3
        readonly property real columnsToHave: Math.ceil(width/desiredSize)

        cellWidth: width/columnsToHave
        cellHeight: desiredSize

        model: CategoryModelFilter {
            id: filter
            sourceModel: SearchModelFilter {
                id: emojiModel
            }
        }

        highlight: Rectangle {
            Kirigami.Theme.inherit: false
            Kirigami.Theme.colorSet: Kirigami.Theme.Selection
            color: Kirigami.Theme.backgroundColor
            z: -1
        }
        currentIndex: -1

        delegate: MouseArea {
            QQC2.Label {
                font.pointSize: 30
                fontSizeMode: Text.Fit
                minimumPointSize: 10
                text: model.display
                horizontalAlignment: Text.AlignHCenter

                anchors.fill: parent
                anchors.margins: 1
            }
            width: emojiView.cellWidth
            height: emojiView.cellHeight

            QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
            QQC2.ToolTip.text: model.toolTip
            QQC2.ToolTip.visible: mouse.containsMouse

            opacity: mouse.containsMouse ? 0.7 : 1

            Keys.onReturnPressed: {
                reportEmoji()
            }

            function reportEmoji() {
                window.report(model.display, model.toolTip)
            }


            id: mouse
            hoverEnabled: true
            onClicked: reportEmoji()
        }
    }
}
