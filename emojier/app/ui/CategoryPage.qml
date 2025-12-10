/*
    SPDX-FileCopyrightText: 2019 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/
pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2

import org.kde.kirigami as Kirigami
import org.kde.plasma.emoji

Kirigami.ScrollablePage {
    id: view

    property alias model: emojiModel.sourceModel
    property string searchText: ""
    property alias category: filter.category
    property bool showClearHistoryButton: false

    signal copyRequested(string text)
    signal addToRecentsRequested(string text, string description)
    signal clearHistoryRequested
    signal allDataRequested
    signal searchFieldFocusRequested

    leftPadding: undefined
    rightPadding: undefined
    horizontalPadding: 0
    Keys.onEscapePressed: Qt.quit()

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
                    view.allDataRequested()
                }
            }

            Keys.onEscapePressed: event => {
                if (text) {
                    clear()
                } else {
                    event.accepted = false
                }
            }
            Keys.onEnterPressed: event => emojiView.currentItem?.Keys.enterPressed(event)
            Keys.onReturnPressed: event => emojiView.currentItem?.Keys.returnPressed(event)
            Keys.onDownPressed: event => {
                emojiView.currentIndex = Math.max(emojiView.currentIndex, 0)
                event.accepted = false
            }
            KeyNavigation.down: emojiView
            KeyNavigation.right: clearHistoryButton
            Binding {
                view.Keys.forwardTo: [searchField]
                view.KeyNavigation.up: searchField.KeyNavigation.down // explicitly set as this and clear button point there
            }
            Connections {
                target: view
                function onSearchFieldFocusRequested() {
                    searchField.forceActiveFocus(Qt.TabFocusReason)
                }
            }

            Component.onCompleted: {
                Qt.callLater(forceActiveFocus);
            }
        }

        QQC2.ToolButton {
            id: clearHistoryButton

            readonly property int effectiveWidth: !visible ? 0 : implicitWidth + titleRowLayout.spacing

            visible: view.showClearHistoryButton
            enabled: emojiView.count > 0
            text: i18n("Clear History")
            icon.name: "edit-clear-history"
            onClicked: view.clearHistoryRequested()
            Keys.onDownPressed: event => searchField.Keys.downPressed(event)
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
            id: menu
            required property QQC2.ItemDelegate label

            onClosed: destroy()

            QQC2.MenuItem {
                icon.name: "edit-copy"
                text: i18nc("@item:inmenu", "Copy Character")
                onClicked: view.copyRequested(menu.label.text)
            }
            QQC2.MenuItem {
                icon.name: "edit-copy"
                text: i18nc("@item:inmenu", "Copy Description")
                onClicked: view.copyRequested(menu.label.QQC2.ToolTip.text)
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
        Keys.onUpPressed: event => {
            if (currentIndex < columnsToHave - 1) {
                currentIndex = -1
            }
            event.accepted = false
        }

        delegate: QQC2.ItemDelegate {
            id: emojiLabel

            required property var model
            required property string toolTip

            width: emojiView.cellWidth
            height: emojiView.cellHeight

            text: model.display
            contentItem: QQC2.Label {
                font.pointSize: 25
                font.family: 'emoji' // Avoid monochrome fonts like DejaVu Sans
                fontSizeMode: emojiLabel.text.length > 5 ? Text.Fit : Text.FixedSize
                minimumPointSize: 10
                text: emojiLabel.text
                textFormat: Text.PlainText
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }

            Accessible.name: emojiLabel.toolTip
            Accessible.onPressAction: tapHandler.action()

            QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
            QQC2.ToolTip.text: emojiLabel.toolTip
            QQC2.ToolTip.visible: hoverHandler.hovered

            Keys.onMenuPressed: event => contextMenuHandler.action()
            Keys.onReturnPressed: event => tapHandler.action()
            Keys.onEnterPressed: event => Keys.returnPressed()

            HoverHandler {
                id: hoverHandler
            }

            TapHandler {
                id: tapHandler
                function action() {
                    view.copyRequested(emojiLabel.text)
                    view.addToRecentsRequested(emojiLabel.text, emojiLabel.toolTip);
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
                    }) as QQC2.Menu;
                    menu.popup();
                }
                onTapped: (eventPoint, button) => action()
            }
        }

        Kirigami.PlaceholderMessage {
            anchors.centerIn: parent
            width: parent.width - (Kirigami.Units.largeSpacing * 8)
            text: view.showClearHistoryButton ? i18n("No recent Emojis") : i18nc("@label placeholder for no emoji found in category", "No matching emoji found")
            visible: emojiView.count === 0 && view.showClearHistoryButton
        }
    }
}
