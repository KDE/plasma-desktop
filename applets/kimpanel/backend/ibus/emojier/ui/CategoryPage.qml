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
    property string searchText: ""
    property alias category: filter.category
    property bool showSearch: false
    property bool showClearHistoryButton: false
    leftPadding: 0
    rightPadding: 0

    Keys.onPressed: {
        if (event.key == Qt.Key_Escape) {
            Qt.quit()
        }
        if (event.text.length > 0 && !view.showSearch && event.modifiers === Qt.NoModifier) {
            // We want to prevent unprintable characters like backspace
            window.startSearch(/[\x00-\x1F\x7F]/.test(event.text) ? "" : event.text)
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
            text: view.searchText
            placeholderText: i18n("Search...")
            visible: view.showSearch
            onTextChanged: {
                emojiModel.search = text
                if (emojiView.currentIndex < 0) {
                    Qt.callLater(function() {
                        emojiView.currentIndex = 0
                    })
                }
            }
            Component.onCompleted: if (visible) Qt.callLater(forceActiveFocus)
            Keys.onEscapePressed: {
                if (text) {
                    clear()
                } else {
                    Qt.quit()
                }
            }
        }

        QQC2.ToolButton {
            visible: view.showClearHistoryButton
            text: i18n("Clear History")
            icon.name: "edit-clear-history"
            onClicked: { recentEmojiModel.clearHistory(); }
        }
    }

    Shortcut {
        sequence: StandardKey.Copy
        enabled: emojiView.currentItem
        onActivated: {
            emojiView.currentItem.reportEmoji()
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
                font.pointSize: 25
                font.family: 'emoji' // Avoid monochrome fonts like DejaVu Sans
                fontSizeMode: model.display.length > 5 ? Text.Fit : Text.FixedSize
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
