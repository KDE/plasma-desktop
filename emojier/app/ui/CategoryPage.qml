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

    QQC2.ActionGroup {
        id: skinToneGroup
        exclusive: true
    }

    actions: [
        Kirigami.Action {
            text: i18nc("@action Search field action", "Search")

            displayComponent: Kirigami.SearchField {
                id: searchField

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
        },
        Kirigami.Action {
            visible: view.showClearHistoryButton
            enabled: emojiView.count > 0
            text: i18nc("@action:button clear emoji history", "Clear History")
            icon.name: "edit-clear-history"
            onTriggered: view.clearHistoryRequested()
        },
        Kirigami.Action {
            text: {
                let result = i18nc("@action:button Button to open a menu that lets you choose a skin tone", "Skin tone: %1")

                switch (view.model.skinTone) {
                case SkinTone.Neutral:
                    result = result.arg("🖐️")
                    break
                case SkinTone.Light:
                    result = result.arg("🖐🏻")
                    break
                case SkinTone.MediumLight:
                    result = result.arg("🖐🏼")
                    break
                case SkinTone.Medium:
                    result = result.arg("🖐🏽")
                    break
                case SkinTone.MediumDark:
                    result = result.arg("🖐🏾")
                    break
                case SkinTone.Dark:
                    result = result.arg("🖐🏿")
                    break
                default:
                    break
                }

                return result
            }
            visible: view.category === "All" || view.category === "People and Body"

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
    ]

    Kirigami.Dialog {
        id: variantDialog
        title: i18nc("@title:window A two-tone skin variant can be selected here", "Select Variant")

        ColumnLayout {
            GridView {
                id: dialogEmojiView

                Layout.alignment: Qt.AlignHCenter
                Layout.preferredHeight: contentHeight
                readonly property real desiredSize: Kirigami.Units.gridUnit * 3

                cellWidth: desiredSize
                cellHeight: desiredSize
                implicitWidth: (view.model.skinTone === SkinTone.Neutral ? 5 : 4) * desiredSize

                model: emoji.twoToneEmojiModel
                delegate: emojiDelegateComponent
            }
        }

        onVisibleChanged: {
            if (visible) {
                dialogEmojiView.currentIndex = 0;
                dialogEmojiView.forceActiveFocus();
            }
        }

        onClosed: emojiView.currentItem?.forceActiveFocus()
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
                onClicked: {
                    view.copyRequested(menu.label.text)
                    variantDialog.close()
                }
            }
            QQC2.MenuItem {
                icon.name: "edit-copy"
                text: i18nc("@item:inmenu", "Copy Description")
                onClicked: {
                    view.copyRequested(menu.label.QQC2.ToolTip.text)
                    variantDialog.close()
                }
            }
        }
    }

    GridView {
        id: emojiView

        readonly property real desiredSize: Kirigami.Units.gridUnit * 3
        readonly property int columnsToHave: Math.ceil(width / desiredSize)
        readonly property int delayInterval: Math.min(300, columnsToHave * 10)
        property int hoveredIndex: -1

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

        delegate: emojiDelegateComponent

        Kirigami.PlaceholderMessage {
            anchors.centerIn: parent
            width: parent.width - (Kirigami.Units.largeSpacing * 8)
            text: view.showClearHistoryButton ? i18nc("@label placeholder for empty recent emoji list", "No recent emojis") : i18nc("@label placeholder for no emoji found in category", "No matching emoji found")
            visible: emojiView.count === 0 && view.showClearHistoryButton
        }
    }

    Component {
        id: emojiDelegateComponent

        QQC2.ItemDelegate {
            id: emojiLabel

            required property var model
            required property string toolTip
            required property int index

            readonly property bool hasVariants: emojiLabel.model.twoToneIndex != undefined
                                             && emojiLabel.model.twoToneIndex != 0

            readonly property bool isHoveredOrFocused: hoverHandler.hovered
                                                    || variantButton.hovered
                                                    || emojiLabel.GridView.isCurrentItem
                                                    || variantButton.activeFocus

            width: GridView.view.cellWidth
            height: GridView.view.cellHeight

            z: (
                (index === emojiView.hoveredIndex  && emojiView.hoveredIndex + 1 === emojiView.currentIndex) ||
                (emojiLabel.GridView.isCurrentItem && emojiView.currentIndex + 1 === emojiView.hoveredIndex)
            ) ? 2 : (emojiLabel.isHoveredOrFocused ? 1 : 0)

            text: model.display
            contentItem: QQC2.Label {
                font.pointSize: 25
                font.family: 'emoji' // Avoid monochrome fonts like DejaVu Sans
                minimumPointSize: 10
                text: emojiLabel.text
                textFormat: Text.PlainText
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter

                QQC2.Button {
                    id: variantButton
                    icon.name: "view-more-horizontal-symbolic"
                    icon.width: Kirigami.Units.iconSizes.small
                    icon.height: Kirigami.Units.iconSizes.small

                    display: QQC2.Button.IconOnly
                    text: i18nc("@action:button Opens a window where a two-tone skin variant can be selected. %1 is the emoji's name", "Select Variant of \"%1\"", emojiLabel.toolTip)

                    QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
                    QQC2.ToolTip.text: variantButton.text
                    QQC2.ToolTip.visible: variantButton.hovered || variantButton.activeFocus

                    visible: emojiLabel.hasVariants && emojiLabel.isHoveredOrFocused

                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.horizontalCenterOffset: emojiLabel.width  / 2 - ((emojiLabel.index+1) % emojiView.columnsToHave ? 0 : variantButton.width / 2)
                    anchors.verticalCenterOffset: - emojiLabel.height / 2 + ( emojiLabel.index   >= emojiView.columnsToHave ? 0 : variantButton.width / 2)

                    onClicked: {
                        emoji.twoToneEmojiModel.twoToneIndex = emojiLabel.model.twoToneIndex
                        variantDialog.open()
                    }
                }
            }

            Accessible.name: emojiLabel.toolTip
            Accessible.onPressAction: tapHandler.action()

            QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
            QQC2.ToolTip.text: emojiLabel.toolTip
            QQC2.ToolTip.visible: hoverHandler.hovered && !variantButton.hovered

            Keys.onMenuPressed: event => contextMenuHandler.action()
            Keys.onReturnPressed: event => tapHandler.action()
            Keys.onEnterPressed: event => Keys.returnPressed()

            HoverHandler {
                id: hoverHandler
                onHoveredChanged: {
                    if(hovered) {
                        emojiView.hoveredIndex = emojiLabel.index
                    }
                }
            }

            TapHandler {
                id: tapHandler
                function action() {
                    view.copyRequested(emojiLabel.text)
                    view.addToRecentsRequested(emojiLabel.text, emojiLabel.toolTip);
                    variantDialog.close()
                }
                onTapped: (eventPoint, button) => action()
            }

            TapHandler {
                acceptedButtons: Qt.LeftButton
                acceptedDevices: PointerDevice.TouchScreen | PointerDevice.Stylus
                onLongPressed: contextMenuHandler.action()
            }

            MouseArea {
                id: contextMenuHandler
                acceptedButtons: Qt.RightButton

                implicitHeight: emojiLabel.implicitHeight
                implicitWidth: emojiLabel.implicitWidth

                function action() {
                    const menu = menuComponent.createObject(emojiLabel, {
                        "label": emojiLabel,
                    }) as QQC2.Menu;
                    menu.popup();
                }
                onClicked: action()
            }
        }
    }
}
