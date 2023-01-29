/*
    SPDX-FileCopyrightText: 2019 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

import QtQuick 2.15
import QtQuick.Layouts 1.3
import org.kde.kirigami 2.15 as Kirigami
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
            model = emoji
            searchText += /[\x00-\x1F\x7F]/.test(event.text) ? "" : event.text
            showSearch = true
            title = i18n("Search")
            category = ""
        }
    }

    titleDelegate: RowLayout {
        Layout.fillWidth: true
        Layout.preferredWidth: view.width
        Kirigami.Heading {
            text: view.title
            Layout.fillWidth: true
        }

        Kirigami.SearchField {
            id: searchField
            Layout.fillWidth: true
            text: view.searchText
            visible: view.showSearch
            inputMethodHints: Qt.ImhNoPredictiveText
            Keys.onEnterPressed: event => Keys.returnPressed(event)
            Keys.onReturnPressed: event => {
                if (emojiView.count > 0) {
                    emojiView.itemAtIndex(0).Keys.returnPressed(event);
                }
            }
            onTextChanged: {
                forceActiveFocus()
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
            Keys.forwardTo: emojiView
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
            emojiView.currentItem.Keys.returnPressed(null)
        }
    }

    GridView {
        id: emojiView

        readonly property real desiredSize: Kirigami.Units.gridUnit * 3
        readonly property int columnsToHave: Math.ceil(width / desiredSize)
        readonly property int delayInterval: Math.min(300, columnsToHave * 10)

        cellWidth: width/columnsToHave
        cellHeight: desiredSize

        model: CategoryModelFilter {
            id: filter
            sourceModel: SearchModelFilter {
                id: emojiModel
            }
        }

        currentIndex: -1
        reuseItems: true

        delegate: QQC2.Label {
            id: emojiLabel
            width: emojiView.cellWidth
            height: emojiView.cellHeight

            font.pointSize: 25
            font.family: 'emoji' // Avoid monochrome fonts like DejaVu Sans
            fontSizeMode: model.display.length > 5 ? Text.Fit : Text.FixedSize
            minimumPointSize: 10
            text: model.display
            horizontalAlignment: Text.AlignHCenter

            QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
            QQC2.ToolTip.text: model.toolTip
            QQC2.ToolTip.visible: hoverHandler.hovered

            opacity: hoverHandler.hovered ? 0.7 : 1
            scale: tapHandler.pressed ? 0.6 : 1

            Keys.onReturnPressed: tapHandler.tapped(null)

            HoverHandler {
                id: hoverHandler
            }

            TapHandler {
                id: tapHandler
                onTapped: window.report(model.display, model.toolTip)
            }

            Behavior on opacity {
                OpacityAnimator {
                    duration: Kirigami.Units.longDuration
                }
            }

            Behavior on scale {
                NumberAnimation {
                    duration: Kirigami.Units.longDuration
                }
            }
        }

        Kirigami.PlaceholderMessage {
            anchors.centerIn: parent
            width: parent.width - (Kirigami.Units.largeSpacing * 8)
            text: i18n("No recent Emojis")
            visible: emojiView.count === 0 && view.showClearHistoryButton
        }
    }
}
