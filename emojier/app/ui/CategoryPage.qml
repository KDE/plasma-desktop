/*
    SPDX-FileCopyrightText: 2019 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

import QtQuick
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import QtQuick.Controls as QQC2
import org.kde.plasma.emoji

Kirigami.ScrollablePage {
    id: view

    property alias model: emojiModel.sourceModel
    property string searchText: ""
    property alias category: filter.category
    property bool showClearHistoryButton: false

    leftPadding: undefined
    rightPadding: undefined
    horizontalPadding: 0

    Keys.onPressed: event => {
        if (event.key === Qt.Key_Escape) {
            Qt.quit()
        }
        if (event.text.length > 0 && event.modifiers === Qt.NoModifier) {
            // We want to prevent unprintable characters like backspace
            model = emoji
            searchText += /[\x00-\x1F\x7F]/.test(event.text) ? "" : event.text
            text: i18nc("@title:page All emojis", "All")
            category = ""
        }
    }

    titleDelegate: RowLayout {
        id: titleRowLayout

        Layout.fillWidth: true
        Layout.preferredWidth: view.width

        spacing: Kirigami.Units.smallSpacing

        Kirigami.Heading {
            text: view.title
            textFormat: Text.PlainText
            elide: Text.ElideRight
            Layout.fillWidth: true
        }

        Kirigami.SearchField {
            id: searchField
            Layout.fillWidth: true
            Layout.minimumWidth: Kirigami.Units.gridUnit * 5
            Layout.maximumWidth: (Kirigami.Units.gridUnit * 15) - clearHistoryButton.effectiveWidth
            text: view.searchText
            inputMethodHints: Qt.ImhNoPredictiveText

            onTextChanged: {
                forceActiveFocus()
                emojiModel.search = text

                // Always focus the first item if there is one
                if (emojiView.count === 0) {
                    emojiView.currentIndex = -1;
                } else {
                    emojiView.currentIndex = 0;
                }

                // If nothing was found, try again with all emojis
                if (emojiView.currentIndex < 0) {
                    let anythingChanged = false;

                    if (view.category.length > 0) {
                        view.category = "";
                        anythingChanged = true;
                    }

                    if (view.model != emoji) {
                        view.model = emoji;
                        anythingChanged = true;
                    }

                    if (anythingChanged) {
                        view.title = i18nc("@title:page All emojis", "All");
                    }
                }
            }
            Component.onCompleted: {
                Qt.callLater(forceActiveFocus);
            }
            Keys.onEscapePressed: event => {
                if (text) {
                    clear()
                } else {
                    Qt.quit()
                }
            }
            Keys.forwardTo: emojiView
        }

        QQC2.ToolButton {
            id: clearHistoryButton

            readonly property int effectiveWidth: !visible ? 0 : implicitWidth + titleRowLayout.spacing

            visible: view.showClearHistoryButton
            enabled: emojiView.count > 0
            text: i18n("Clear History")
            icon.name: "edit-clear-history"
            onClicked: {
                recentEmojiModel.clearHistory();
            }
        }
    }

    Shortcut {
        sequences: [StandardKey.Copy]
        enabled: emojiView.currentItem
        onActivated: {
            emojiView.currentItem.Keys.returnPressed(null)
        }
    }

    Component {
        id: menuComponent

        QQC2.Menu {
            required property Text label

            onClosed: destroy()

            QQC2.MenuItem {
                icon.name: "edit-copy"
                text: i18nc("@item:inmenu", "Copy Character")
                onClicked: {
                    CopyHelper.copyTextToClipboard(label.text);
                    window.showPassiveNotification(i18n("%1 copied to the clipboard", label.text));
                }
            }
            QQC2.MenuItem {
                icon.name: "edit-copy"
                text: i18nc("@item:inmenu", "Copy Description")
                onClicked: {
                    CopyHelper.copyTextToClipboard(label.QQC2.ToolTip.text);
                    window.showPassiveNotification(i18n("%1 copied to the clipboard", label.QQC2.ToolTip.text));
                }
            }
        }
    }

    GridView {
        id: emojiView

        readonly property real desiredSize: Kirigami.Units.gridUnit * 3
        readonly property int columnsToHave: Math.ceil(width / desiredSize)
        readonly property int delayInterval: Math.min(300, columnsToHave * 10)

        cellWidth: width / columnsToHave
        cellHeight: desiredSize

        model: CategoryModelFilter {
            id: filter
            sourceModel: SearchModelFilter {
                id: emojiModel
            }
        }

        currentIndex: -1
        reuseItems: true

        delegate: QQC2.ItemDelegate {
            id: emojiLabel
            width: emojiView.cellWidth
            height: emojiView.cellHeight

            highlighted: GridView.isCurrentItem
            contentItem: QQC2.Label {
                font.pointSize: 25
                font.family: 'emoji' // Avoid monochrome fonts like DejaVu Sans
                fontSizeMode: model.display.length > 5 ? Text.Fit : Text.FixedSize
                minimumPointSize: 10
                text: model.display
                textFormat: Text.PlainText
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }

            Accessible.name: model.toolTip
            Accessible.onPressAction: tapHandler.action()

            QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
            QQC2.ToolTip.text: model.toolTip
            QQC2.ToolTip.visible: hoverHandler.hovered

            Keys.onMenuPressed: event => contextMenuHandler.action()
            Keys.onReturnPressed: event => tapHandler.action()

            HoverHandler {
                id: hoverHandler
            }

            TapHandler {
                id: tapHandler
                function action() {
                    window.report(model.display, model.toolTip);
                }
                onTapped: (eventPoint, button) => action()
            }

            TapHandler {
                acceptedButtons: Qt.LeftButton
                acceptedDevices: PointerDevice.TouchScreen | PointerDevice.Stylus
                onLongPressed: contextMenuHandler.action()
            }

            TapHandler {
                id: contextMenuHandler
                acceptedButtons: Qt.RightButton
                function action() {
                    const menu = menuComponent.createObject(emojiLabel, {
                        "label": emojiLabel,
                    });
                    menu.popup();
                }
                onTapped: (eventPoint, button) => action()
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
