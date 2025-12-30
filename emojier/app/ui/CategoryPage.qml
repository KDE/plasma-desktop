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
            Layout.maximumWidth: Math.max((Kirigami.Units.gridUnit * 15) - clearHistoryButton.effectiveWidth - skinToneButton.effectiveWidth, Layout.minimumWidth)
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
            text: i18nc("@action:button clear emoji history", "Clear History")
            icon.name: "edit-clear-history"
            onClicked: view.clearHistoryRequested()
            KeyNavigation.right: skinToneButton
            Keys.onDownPressed: event => searchField.Keys.downPressed(event)
        }


        QQC2.ToolButton {
            id: skinToneButton

            readonly property int effectiveWidth: !visible ? 0 : implicitWidth + titleRowLayout.spacing
            readonly property string label: i18nc("@action:button Button to open a menu that lets you choose a skin tone", "Skin tone:")

            function openMenu() {
                if (!skinToneMenu.visible) {
                    skinToneMenu.open()
                } else {
                    skinToneMenu.dismiss()
                }
            }

            down: pressed || skinToneMenu.visible
            visible: view.title === i18nc("@title:page All emojis", "All") || view.category === "People and Body"

            Accessible.name: label
            Accessible.role: Accessible.ButtonMenu

            contentItem: RowLayout {
                spacing: Kirigami.Units.smallSpacing

                QQC2.Label {
                    text: skinToneButton.label
                }
                QQC2.Label {
                    text: {
                        switch (view.model.skinTone) {
                        case SkinTone.Neutral:
                            return "🖐️"
                        case SkinTone.Light:
                            return "🖐🏻"
                        case SkinTone.MediumLight:
                            return "🖐🏼"
                        case SkinTone.Medium:
                            return "🖐🏽"
                        case SkinTone.MediumDark:
                            return "🖐🏾"
                        case SkinTone.Dark:
                            return "🖐🏿"
                        default:
                            return ""
                        }
                    }
                    font.pixelSize: Kirigami.Units.iconSizes.smallMedium
                }
            }

            Layout.preferredHeight: clearHistoryButton.implicitHeight

            leftPadding: LayoutMirroring.enabled ? Kirigami.Units.iconSizes.smallMedium : Kirigami.Units.largeSpacing
            rightPadding: LayoutMirroring.enabled ? Kirigami.Units.largeSpacing : Kirigami.Units.iconSizes.smallMedium

            onPressed: openMenu()
            Keys.onReturnPressed: openMenu()
            Keys.onEnterPressed: openMenu()

            Keys.onDownPressed: event => searchField.Keys.downPressed(event)

            QQC2.Menu {
                id: skinToneMenu
                y: skinToneButton.height
                QQC2.ActionGroup {
                    id: skinToneGroup
                    exclusive: true
                }
                Kirigami.Action {
                    QQC2.ActionGroup.group: skinToneGroup
                    text: i18nc("@action:inmenu Skin Tone", "🖐️ Neutral")
                    Accessible.name: i18nc("@action:inmenu Skin Tone", "Neutral")
                    shortcut: "ctrl+1"
                    checkable: true
                    checked: view.model.skinTone == SkinTone.Neutral
                    onTriggered: view.model.skinTone = SkinTone.Neutral
                }
                Kirigami.Action {
                    QQC2.ActionGroup.group: skinToneGroup
                    text: i18nc("@action:inmenu Skin Tone", "🖐🏻 Light")
                    Accessible.name: i18nc("@action:inmenu Skin Tone", "Light")
                    shortcut: "ctrl+2"
                    checkable: true
                    checked: view.model.skinTone == SkinTone.Light
                    onTriggered: view.model.skinTone = SkinTone.Light
                }
                Kirigami.Action {
                    QQC2.ActionGroup.group: skinToneGroup
                    text: i18nc("@action:inmenu Skin Tone", "🖐🏼 Medium Light")
                    Accessible.name: i18nc("@action:inmenu Skin Tone", "Medium Light")
                    shortcut: "ctrl+3"
                    checkable: true
                    checked: view.model.skinTone == SkinTone.MediumLight
                    onTriggered: view.model.skinTone = SkinTone.MediumLight
                }
                Kirigami.Action {
                    QQC2.ActionGroup.group: skinToneGroup
                    text: i18nc("@action:inmenu Skin Tone", "🖐🏽 Medium")
                    Accessible.name: i18nc("@action:inmenu Skin Tone", "Medium")
                    shortcut: "ctrl+4"
                    checkable: true
                    checked: view.model.skinTone == SkinTone.Medium
                    onTriggered: view.model.skinTone = SkinTone.Medium
                }
                Kirigami.Action {
                    QQC2.ActionGroup.group: skinToneGroup
                    text: i18nc("@action:inmenu Skin Tone", "🖐🏾 Medium Dark")
                    Accessible.name: i18nc("@action:inmenu Skin Tone", "Medium Dark")
                    shortcut: "ctrl+5"
                    checkable: true
                    checked: view.model.skinTone == SkinTone.MediumDark
                    onTriggered: view.model.skinTone = SkinTone.MediumDark
                }
                Kirigami.Action {
                    QQC2.ActionGroup.group: skinToneGroup
                    text: i18nc("@action:inmenu Skin Tone", "🖐🏿 Dark")
                    Accessible.name: i18nc("@action:inmenu Skin Tone", "Dark")
                    shortcut: "ctrl+6"
                    checkable: true
                    checked: view.model.skinTone == SkinTone.Dark
                    onTriggered: view.model.skinTone = SkinTone.Dark
                }
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
            text: view.showClearHistoryButton ? i18nc("@label placeholder for empty recent emoji list", "No recent Emojis") : i18nc("@label placeholder for no emoji found in category", "No matching emoji found")
            visible: emojiView.count === 0 && view.showClearHistoryButton
        }
    }
}
