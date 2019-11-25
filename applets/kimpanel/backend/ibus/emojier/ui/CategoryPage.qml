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

    header: QQC2.TextField {
        id: searchField
        Layout.fillWidth: true
        placeholderText: i18n("Search...")
        onTextChanged: {
            emojiModel.search = text
            if (emojiView.currentIndex < 0) {
                emojiView.currentIndex = 0
            }
        }
        onEditingFinished: {
            if (emojiView.currentItem)
                emojiView.currentItem.reportEmoji()
        }
        height: visible ? implicitHeight : 0
        visible: false
        Keys.onEscapePressed: visible = false
    }

    actions.main: Kirigami.Action {
        icon.name: "search"
        tooltip: i18n("Search...")
        shortcut: StandardKey.Find
        onTriggered: {
            searchField.visible = true
            searchField.focus = true
        }
    }

    GridView {
        id: emojiView
        cellWidth: 64
        cellHeight: cellWidth
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

        delegate: QQC2.Label {
            font.pointSize: 30
            fontSizeMode: Text.Fit
            minimumPointSize: 10
            text: model.display

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

            MouseArea {
                id: mouse
                anchors.fill: parent
                hoverEnabled: true
                onClicked: reportEmoji()
            }
        }
    }
}
