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
                    emojiView.forceActiveFocus(Qt.TabFocusReason)
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
                let example = "";

                switch (view.model.skinTone) {
                case SkinTone.Neutral:
                    example = "🖐️";
                    break
                case SkinTone.Light:
                    example = "🖐🏻";
                    break
                case SkinTone.MediumLight:
                    example ="🖐🏼";
                    break
                case SkinTone.Medium:
                    example = "🖐🏽";
                    break
                case SkinTone.MediumDark:
                    example = "🖐🏾";
                    break
                case SkinTone.Dark:
                    example = "🖐🏿";
                    break
                default:
                    break
                }

                return i18nc("@action:button Button to open a menu that lets you choose a skin tone", "Skin tone: %1", example);
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
        },
        Kirigami.Action {
            icon.name: "zoom-out"
            displayHint: Kirigami.DisplayHint.IconOnly
            enabled: emojiView.targetSize > emojiView.minSize + 0.5
            text: i18nc("@action:button zoom the emoji grid out", "Zoom Out")
            onTriggered: emojiView.applyZoom(-1)
        },
        Kirigami.Action {
            text: i18nc("@info:status label for the current zoom-level indicator", "Zoom level")
            displayComponent: QQC2.Label {
                text: i18nc("@info:status current emoji zoom level as a percentage, %1 is a number", "%1%",
                            Math.round(emojiView.targetSize / emojiView.defaultSize * 100))
                textFormat: Text.PlainText
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                leftPadding: Kirigami.Units.smallSpacing
                rightPadding: Kirigami.Units.smallSpacing
            }
        },
        Kirigami.Action {
            icon.name: "zoom-in"
            displayHint: Kirigami.DisplayHint.IconOnly
            enabled: emojiView.targetSize < emojiView.maxSize - 0.5
            text: i18nc("@action:button zoom the emoji grid in", "Zoom In")
            onTriggered: emojiView.applyZoom(1)
        },
        Kirigami.Action {
            icon.name: "zoom-original"
            displayHint: Kirigami.DisplayHint.IconOnly
            enabled: Math.abs(emojiView.targetSize - emojiView.defaultSize) > 0.5
            text: i18nc("@action:button reset the emoji grid zoom to 100%", "Reset Zoom")
            onTriggered: emojiView.resetZoom()
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
                // Track the main grid's zoom so variants match the chosen size.
                readonly property real desiredSize: emojiView.desiredSize

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

    Shortcut {
        sequences: [StandardKey.ZoomIn]
        enabled: emojiView.targetSize < emojiView.maxSize - 0.5
        onActivated: emojiView.applyZoom(1)
    }
    Shortcut {
        sequences: [StandardKey.ZoomOut]
        enabled: emojiView.targetSize > emojiView.minSize + 0.5
        onActivated: emojiView.applyZoom(-1)
    }
    Shortcut {
        sequence: "Ctrl+0"
        enabled: Math.abs(emojiView.targetSize - emojiView.defaultSize) > 0.5
        onActivated: emojiView.resetZoom()
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

        readonly property real defaultSize: Kirigami.Units.gridUnit * 3
        readonly property real minSize: Kirigami.Units.gridUnit * 1.5
        readonly property real maxSize: defaultSize * 2.75
        readonly property real zoomStep: 0.25

        property real desiredSize: defaultSize
        // Ctrl+wheel zoom accumulates into targetSize; desiredSize (which drives
        // the relayout) is updated on a throttle so a fast wheel spin coalesces
        // into at most one relayout per frame instead of one per event.
        property real targetSize: defaultSize
        readonly property int columnsToHave: Math.ceil(width / desiredSize)
        readonly property int delayInterval: Math.min(300, columnsToHave * 10)
        property int hoveredIndex: -1

        function applyZoom(steps: int): void {
            const stepPx = defaultSize * zoomStep
            const index = Math.round((targetSize - defaultSize) / stepPx) + steps
            targetSize = Math.max(minSize, Math.min(maxSize, defaultSize + index * stepPx))
            emoji.emojiZoom = targetSize / defaultSize
            if (!zoomThrottle.running) {
                desiredSize = targetSize
                zoomThrottle.start()
            }
        }

        function resetZoom(): void {
            targetSize = defaultSize
            desiredSize = defaultSize
            emoji.emojiZoom = 1
        }

        // Clamp in case the stored zoom or the size bounds changed since it was saved.
        Component.onCompleted: {
            const saved = Math.max(minSize, Math.min(maxSize, defaultSize * emoji.emojiZoom))
            targetSize = saved
            desiredSize = saved
        }

        cellWidth: width / columnsToHave
        cellHeight: desiredSize

        Timer {
            id: zoomThrottle
            interval: 33 // ms; caps relayouts at ~30 Hz so a fast spin coalesces
            onTriggered: emojiView.desiredSize = emojiView.targetSize
        }

        MouseArea {
            anchors.fill: parent
            acceptedButtons: Qt.NoButton
            onWheel: wheel => {
                if (wheel.modifiers & Qt.ControlModifier) {
                    emojiView.applyZoom(wheel.angleDelta.y > 0 ? 1 : -1)
                    wheel.accepted = true
                } else {
                    wheel.accepted = false
                }
            }
        }

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
                font.pixelSize: Math.round(emojiLabel.height * 0.6)
                font.family: 'emoji' // Avoid monochrome fonts like DejaVu Sans
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
