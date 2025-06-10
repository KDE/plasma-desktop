/*
    SPDX-FileCopyrightText: 2018 Roman Gilg <subdiff@gmail.com>
    SPDX-FileCopyrightText: 2018 Furkan Tokac <furkantokac34@gmail.com>
    SPDX-FileCopyrightText: 2024 Jakob Petsovits <jpetso@petsovits.com>
    SPDX-FileCopyrightText: 2024 ivan tkachenko <me@ratijas.tk>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Dialogs
import QtQuick.Layouts
import org.kde.kcmutils as KCMUtils

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.components as KirigamiAddonsComponents
import org.kde.kquickcontrols as KQuickControls

import org.kde.plasma.private.kcm_mouse as Mouse

import org.kde.kirigami.layouts // TODO: gesture demo only, for DisplayHint

KCMUtils.ScrollViewKCM {
//KCMUtils.SimpleKCM {
    id: root

    //readonly property Mouse.KWinWaylandBackend backend: KCMUtils.ConfigModule.inputBackend
    //readonly property Mouse.InputDevice device: backend.inputDevices[KCMUtils.ConfigModule.currentDeviceIndex] ?? null

    //title: device?.name ?? ""

    header: Header {
        saveLoadMessage: root.KCMUtils.ConfigModule.saveLoadMessage
        hotplugMessage: root.KCMUtils.ConfigModule.hotplugMessage
    }

    actions: [
        Kirigami.Action {
            icon.name: "document-import-symbolic"
            text: i18nc("@action:button Import shortcut scheme", "Import…")
            onTriggered: importSheet.open()
        },
        Kirigami.Action {
            icon.name: "document-export-symbolic"
            text: i18nc("@action:button Export shortcut scheme", "Export…")
        },
        Kirigami.Action {
            text: i18nc("@action:button Add new gesture/action assignment", "Add Gesture…")
            icon.name: "list-add-symbolic"
            displayHint: Kirigami.DisplayHint.KeepVisible
            onTriggered: selectGestureDialog.open()
        }
    ]

/*
    MouseArea {
        // Deliberately using MouseArea on the page instead of a TapHandler on the button, so we can capture clicks anywhere
        id: buttonCapture

        property var lastButton: undefined

        parent: root.contentItem
        anchors.fill: parent
        enabled: newBinding.checked
        preventStealing: true
        acceptedButtons: Qt.AllButtons & ~(Qt.LeftButton | Qt.RightButton | Qt.MiddleButton)

        onClicked: mouse => {
            lastButton = buttonMappings.extraButtons.find(entry => Qt[entry.buttonName] === mouse.button)
            newBinding.visible = false
            newKeySequenceItem.visible = true
            newKeySequenceItem.startCapturing()
        }
    }

    ColumnLayout {
        spacing: Kirigami.Units.smallSpacing

        Kirigami.FormLayout {
            id: buttonLayout

            twinFormLayouts: otherLayout

            Item {
                Kirigami.FormData.isSection: true
                Kirigami.FormData.label: i18ndc("kcmmouse", "@title:group", "Extra Button Bindings")
            }

            Repeater {
                id: buttonMappings

                model: extraButtons.filter(entry => root.device?.buttonMapping[entry.buttonName] !== undefined).map(entry => {
                    return {"buttonData": entry, "keySeq": root.device.buttonMapping[entry.buttonName]};
                })

                readonly property var extraButtons: Array.from({length: 24}, (value, index) => ({
                    buttonName: "ExtraButton" + (index + 1),
                    button: Qt["ExtraButton" + (index + 1)],
                    label: i18ndc("kcmmouse", "@label for assigning an action to a numbered button", "Extra Button %1:", index + 1)
                }))

                delegate: KQuickControls.KeySequenceItem {
                    required property var buttonData
                    required property var keySeq

                    Kirigami.FormData.label: buttonData.label

                    keySequence: keySeq

                    patterns: ShortcutPattern.Modifier | ShortcutPattern.Key | ShortcutPattern.ModifierAndKey
                    multiKeyShortcutsAllowed: false
                    checkForConflictsAgainst: KQuickControls.ShortcutType.None

                    onCaptureFinished: {
                        const copy = root.device.buttonMapping
                        copy[buttonData.buttonName] = keySequence
                        root.device.buttonMapping = copy
                    }
                }
            }
        }

        Kirigami.InlineMessage {
            id: explanationLabel
            Layout.fillWidth: true
            visible: newBinding.checked || newKeySequenceItem.visible

            text: newBinding.visible
                ? i18ndc("kcmmouse","@action:button", "Press the mouse button for which you want to add a key binding")
                : i18ndc("kcmmouse","@action:button, %1 is the translation of 'Extra Button %1' from above",
                    "Enter the new key combination for %1", buttonCapture.lastButton?.label ?? "")

            actions: Kirigami.Action {
                icon.name: "dialog-cancel"
                text: i18ndc("kcmmouse", "@action:button", "Cancel")
                onTriggered: source => {
                    newKeySequenceItem.visible = false;
                    newBinding.visible = true
                    newBinding.checked = false
                }
            }
        }

        Kirigami.FormLayout {
            id: otherLayout

            twinFormLayouts: buttonLayout

            QQC2.Button {
                id: newBinding
                checkable: true
                text: checked
                    ? i18ndc("kcmmouse", "@action:button", "Press a mouse button")
                    : i18ndc("kcmmouse", "@action:button, Bind a mousebutton to keyboard key(s)", "Add Binding…")
                icon.name: "list-add"
            }

            KQuickControls.KeySequenceItem {
                id: newKeySequenceItem
                visible: false

                patterns: ShortcutPattern.Modifier | ShortcutPattern.Key | ShortcutPattern.ModifierAndKey
                multiKeyShortcutsAllowed: false
                checkForConflictsAgainst: KQuickControls.ShortcutType.None

                onCaptureFinished: {
                    visible = false
                    newBinding.visible = true
                    newBinding.checked = false
                    const copy = root.device.buttonMapping
                    copy[buttonCapture.lastButton.buttonName] = keySequence
                    root.device.buttonMapping = copy
                }
            }
        }
    }
*/
    title: i18n("Gestures")
    //anchors.fill: parent

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        Kirigami.NavigationTabBar {
            Layout.fillWidth: true

            Kirigami.Theme.inherit: false
            Kirigami.Theme.colorSet: Kirigami.Theme.Window

            actions: [
                Kirigami.Action {
                    text: i18n("Stroke Gestures")
                    icon.name: "draw-polyline-symbolic"
                    checked: false
                },
                Kirigami.Action {
                    text: i18n("Touchpad Gestures")
                    icon.name: "input-touchpad-symbolic"
                    checked: true
                },
                Kirigami.Action {
                    text: i18n("Scroll Gestures")
                    icon.name: "gnumeric-object-scrollbar" // TODO: obviously this can't be the final icon, just look at the name
                    checked: false
                }
            ]
        }

        ListView {
            id: gestureList
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true

            model: ListModel {
                ListElement { trigger: "Swipe up with 3 fingers"; triggerSection: "Touchpad Swipe"; component: "Window Management"; componentIcon: "kwin"; actionText: "Cycle through Window Overview and Desktop Grid" }
                ListElement { trigger: "Swipe right with 3 fingers"; triggerSection: "Touchpad Swipe"; component: "Window Management"; componentIcon: "kwin"; actionText: "Walk through Windows" }
                ListElement { trigger: "Swipe left with 3 fingers"; triggerSection: "Touchpad Swipe"; component: "Window Management"; componentIcon: "kwin"; actionText: "Walk through Windows - backwards" }
                ListElement { trigger: "Swipe up with 4 fingers"; triggerSection: "Touchpad Swipe"; component: "Window Management"; componentIcon: "kwin"; actionText: "Cycle through Desktop Grid and Window Overview" }
                ListElement { trigger: "Swipe right with 4 fingers"; triggerSection: "Touchpad Swipe"; component: "Window Management"; componentIcon: "kwin"; actionText: "Switch to Next Desktop" }
                ListElement { trigger: "Swipe left with 4 fingers"; triggerSection: "Touchpad Swipe"; component: "Window Management"; componentIcon: "kwin"; actionText: "Switch to Previous Desktop" }
                ListElement { trigger: "Spread with 3 fingers"; triggerSection: "Touchpad Pinch"; component: "Window Management"; componentIcon: "kwin"; actionText: "Zoom In Screen" }
                ListElement { trigger: "Pinch with 3 fingers"; triggerSection: "Touchpad Pinch"; component: "Window Management"; componentIcon: "kwin"; actionText: "Peek at Desktop" }
                ListElement { trigger: "Pinch with 4 fingers"; triggerSection: "Touchpad Pinch"; component: "Session Management"; componentIcon: "preferences-system-login"; actionText: "Lock Session" }
                ListElement { trigger: "Rotate clockwise with 3 fingers"; triggerSection: "Touchpad Rotate"; component: "Audio Volume"; componentIcon: "kmix"; actionText: "Increase Volume" }
                ListElement { trigger: "Rotate counter-clockwise with 3 fingers"; triggerSection: "Touchpad Rotate"; component: "Audio Volume"; componentIcon: "kmix"; actionText: "Decrease Volume" }
                ListElement { trigger: "Rotate clockwise with 4 fingers"; triggerSection: "Touchpad Rotate"; component: "Power Management"; componentIcon: "preferences-system-power-management"; actionText: "Increase Screen Brightness" }
                ListElement { trigger: "Rotate counter-clockwise with 4 fingers"; triggerSection: "Touchpad Rotate"; component: "Power Management"; componentIcon: "preferences-system-power-management"; actionText: "Decrease Screen Brightness" }
            }

            section.property: "triggerSection"
            section.delegate: Kirigami.ListSectionHeader {
                required property string section

                width: ListView.view.width
                text: section
            }

            delegate: QQC2.ItemDelegate {
                id: delegate
                implicitWidth: gestureList.width

                // There's no need for a list item to ever be selected
                down: false
                highlighted: false

                required property string index
                required property string trigger
                required property string actionText
                required property string componentIcon

                contentItem: RowLayout {
                    Layout.fillWidth: true
                    spacing: Kirigami.Units.smallSpacing

                    Kirigami.IconTitleSubtitle {
                        title: delegate.actionText
                        subtitle: delegate.trigger
                        icon.name: delegate.componentIcon
                        Layout.fillWidth: true
                        selected: delegate.highlighted || delegate.down
                    }

                    QQC2.ToolButton {
                        action: Kirigami.Action {
                            icon.name: "edit-entry-symbolic"
                            text: i18nc("@action:button", "Edit action assignment")
                            Accessible.name: i18nc("@action:button accessible", "Edit action \"%1\" for gesture \"%2\"", delegate.actionText, delegate.trigger)
                            onTriggered: {
                                selectGestureDialog.editing = true;
                                selectGestureDialog.open();
                            }
                        }
                        display: QQC2.AbstractButton.IconOnly
                        QQC2.ToolTip.text: text
                        QQC2.ToolTip.visible: hovered
                    }

                    QQC2.ToolButton {
                        action: Kirigami.Action {
                            icon.name: "edit-delete-remove-symbolic"
                            text: i18nc("@action:button", "Remove gesture")
                            Accessible.name: i18nc("@action:button accessible", "Remove gesture \"%1\" with action \"%2\"", delegate.trigger, delegate.actionText)
                            //onTriggered: kcm.gestureModel.remove(index)
                        }
                        display: QQC2.AbstractButton.IconOnly
                        QQC2.ToolTip.text: text
                        QQC2.ToolTip.visible: hovered
                    }
                }
            }
        }
    }


    Kirigami.PromptDialog {
        id: selectGestureDialog
        property bool editing: false
        property string componentName: ""
        property string oldExec: ""
        property string name: ""

        title: editing ? i18n("Edit Gesture") : i18n("Select Gesture")
        //iconName: 'folder-script-symbolic'

        onVisibleChanged: {
            if (visible) {
                gestureType.forceActiveFocus();
                // cmdField.clear();
                // nameField.clear();
                // cmdField.forceActiveFocus();
                // if (editing) {
                //     cmdField.text = oldExec;
                //     nameField.text = name;
                //     cmdField.selectAll();
                // }
            }
        }
        onRejected: {
            if (selectGestureDialog.editing) {
                selectGestureDialog.editing = false;
            }
        }

        // standardButtonActions are located to the left of customFooterActions, which is against
        // KDE HIG where Cancel should be to the right of the "OK"-style action
        //standardButtons: Kirigami.Dialog.Cancel

        customFooterActions: [
            Kirigami.Action {
                text: (selectedActionMessage.visible && preAssignedActionMessage.visible) ? i18n("Replace Assigned Action") :
                    (selectedActionMessage.visible && !preAssignedActionMessage.visible) ? i18n("Update Gesture") :
                    (!selectedActionMessage.visible && preAssignedActionMessage.visible) ? i18n("Assign New Action…") :
                    i18n("Assign Action…")
                icon.name: selectedActionMessage.visible ? "dialog-ok-symbolic" : // replace or update
                    (!selectedActionMessage.visible && preAssignedActionMessage.visible) ? "edit-entry-symbolic" :
                    "list-add-symbolic"
                onTriggered: selectGestureDialog.accept()
                //enabled: cmdField.length > 0
            },
            Kirigami.Action {
                text: i18n("Cancel")
                icon.name: "dialog-cancel-symbolic"
                onTriggered: selectGestureDialog.reject()
            }
        ]

        ColumnLayout {
            spacing: Kirigami.Units.smallSpacing

            Kirigami.FormLayout {
                KirigamiAddonsComponents.RadioSelector {
                    id: gestureCategory
                    Layout.fillWidth: true
                    Kirigami.FormData.isSection: true
                    visible: !selectGestureDialog.editing

                    actions: [
                        Kirigami.Action {
                            text: i18nc("Type of Gesture", "Stroke")
                            icon.name: "draw-polyline-symbolic"
                            checked: false
                        },
                        Kirigami.Action {
                            text: i18nc("Type of Gesture", "Touchpad")
                            icon.name: "input-touchpad-symbolic"
                            checked: true
                        },
                        Kirigami.Action {
                            text: i18nc("Type of Gesture", "Scroll")
                            icon.name: "gnumeric-object-scrollbar" // TODO: obviously this can't be the final icon, just look at the name
                            checked: false
                        }
                    ]
                }
                Kirigami.Separator {
                    Kirigami.FormData.isSection: true
                    visible: gestureCategory.visible
                }

                RowLayout {
                    id: gestureType
                    Kirigami.FormData.label: i18nc("@label:combobox Type of gesture (e.g. swipe)", "Gesture:")
                    Layout.fillWidth: true
                    Layout.leftMargin: shortcutsScroll.x
                    Layout.alignment: Qt.AlignLeft
                    spacing: Kirigami.Units.smallSpacing

                    QQC2.RadioButton {
                        text: i18n("Swipe")
                        checked: true
                    }
                    QQC2.RadioButton {
                        text: i18n("Pinch")
                    }
                    QQC2.RadioButton {
                        text: i18n("Rotate")
                    }
                    QQC2.RadioButton {
                        text: i18n("Hold")
                    }
                }
                QQC2.ComboBox {
                    Layout.fillWidth: true
                    model: ListModel {
                        ListElement { name: "with 2 fingers"; value: 0 }
                        ListElement { name: "with 3 fingers"; value: 1 }
                        ListElement { name: "with 4 fingers"; value: 2 }
                    }
                    textRole: "name"
                    valueRole: "value"
                    currentIndex: 1
                }
                GridLayout {
                    columns: 3
                    Kirigami.FormData.label: i18nc("@label:combobox Direction of gesture (e.g. swipe up)", "Direction:")
                    QQC2.Button {
                        id: swipeUpLeftButton
                        text: "↖"
                        checkable: true
                    }
                    QQC2.Button {
                        id: swipeUpButton
                        text: "↑"
                        checkable: true
                        checked: true
                    }
                    QQC2.Button {
                        id: swipeUpRightButton
                        text: "↗"
                        checkable: true
                    }
                    QQC2.Button {
                        text: "←"
                        checkable: true
                    }
                    Item {
                        // no center button
                    }
                    QQC2.Button {
                        text: "→"
                        checkable: true
                    }
                    QQC2.Button {
                        text: "↙"
                        checkable: true
                    }
                    QQC2.Button {
                        text: "↓"
                        checkable: true
                    }
                    QQC2.Button {
                        text: "↘"
                        checkable: true
                    }
                }
                KQuickControls.KeySequenceItem {
                    Kirigami.FormData.label: i18nc("@label:button", "Require modifier keys:")

                    modifierlessAllowed: false
                    modifierOnlyAllowed: true
                    multiKeyShortcutsAllowed: false
                    checkForConflictsAgainst: KQuickControls.ShortcutType.None

                    // onCaptureFinished: {
                    //     const copy = root.device.buttonMapping
                    //     copy[buttonData.buttonName] = keySequence
                    //     root.device.buttonMapping = copy
                    // }
                }

                Kirigami.InlineMessage {
                    id: selectedActionMessage
                    Layout.fillWidth: true
                    Kirigami.FormData.isSection: true
                    visible: selectGestureDialog.editing && (swipeUpButton.checked || swipeUpLeftButton.checked || swipeUpRightButton.checked)

                    text: i18n("This gesture will trigger \"%1\".", "Maximize Window")
                    icon.source: "kwin"
                }
                Kirigami.InlineMessage {
                    id: preAssignedActionMessage
                    Layout.fillWidth: true
                    Kirigami.FormData.isSection: true
                    visible: swipeUpButton.checked
                    type: selectedActionMessage.visible ? Kirigami.MessageType.Warning : Kirigami.MessageType.Information

                    text: selectedActionMessage.visible ?
                        i18n("\"%1\" is already assigned to this gesture. Replace with \"%2\"?", "Cycle through Window Overview and Desktop Grid", "Maximize Window") :
                        selectedActionMessage.text
                    icon.source: "kwin"
                }
            }

            // QQC2.Label {
            //     Layout.fillWidth: true
            //     text: i18n("something about assigning an action")
            //     textFormat: Text.PlainText
            //     wrapMode: Text.Wrap
            // }
        }
    }
}
