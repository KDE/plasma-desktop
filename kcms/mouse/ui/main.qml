/*
    SPDX-FileCopyrightText: 2018 Roman Gilg <subdiff@gmail.com>
    SPDX-FileCopyrightText: 2018 Furkan Tokac <furkantokac34@gmail.com>
    SPDX-FileCopyrightText: 2024 Jakob Petsovits <jpetso@petsovits.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kquickcontrols as KQuickControls

Kirigami.ApplicationItem {
    id: root

    pageStack.globalToolBar.style:  Kirigami.ApplicationHeaderStyle.None
    pageStack.columnView.columnResizeMode: Kirigami.ColumnView.SingleColumn
    pageStack.defaultColumnWidth: Kirigami.Units.gridUnit * 20

    property int deviceIndex: (
        (deviceSelector.currentIndex >= 0 && deviceSelector.currentIndex < backend.inputDevices?.length)
            ? deviceSelector.currentIndex
            : -1
    )
    property QtObject device: deviceIndex >= 0 ? backend.inputDevices[deviceIndex] : null
    signal changeSignal()

    enabled: device !== null

    pageStack.initialPage: Kirigami.ScrollablePage {
        spacing: Kirigami.Units.smallSpacing

        Kirigami.FormLayout {
            id: formLayout
            enabled: backend.inputDevices?.length > 0

            // Device
            QQC2.ComboBox {
                id: deviceSelector
                Kirigami.FormData.label: i18nd("kcmmouse", "Device:")
                visible: !backend.isAnonymousInputDevice
                enabled: count > 1
                Layout.fillWidth: true
                model: backend.inputDevices?.length > 0 ? backend.inputDevices : [""]
                textRole: "name"

                Connections {
                    target: backend
                    function onDeviceRemoved(index) {
                        if (index < deviceSelector.currentIndex) {
                            --deviceSelector.currentIndex;
                        }
                    }
                }
            }

            Item {
                Kirigami.FormData.isSection: false
            }

            // General
            QQC2.CheckBox {
                id: deviceEnabled
                Kirigami.FormData.label: i18nd("kcmmouse", "General:")
                text: i18nd("kcmmouse", "Device enabled")
                visible: !backend.isAnonymousInputDevice
                enabled: device?.supportsDisableEvents ?? false
                checked: enabled && device.enabled

                onToggled: {
                    if (device) {
                        device.enabled = checked
                        root.changeSignal()
                    }
                }

                QQC2.ToolTip.delay: 1000
                QQC2.ToolTip.visible: hovered
                QQC2.ToolTip.text: i18nd("kcmmouse", "Accept input through this device.")
            }

            QQC2.CheckBox {
                id: leftHanded
                Kirigami.FormData.label: deviceEnabled.visible ? null : deviceEnabled.Kirigami.FormData.label
                text: i18nd("kcmmouse", "Left handed mode")
                enabled: device?.supportsLeftHanded ?? false
                checked: enabled && device.leftHanded

                onToggled: {
                    if (device) {
                        device.leftHanded = checked
                        root.changeSignal()
                    }
                }

                QQC2.ToolTip.delay: 1000
                QQC2.ToolTip.visible: hovered
                QQC2.ToolTip.text: i18nd("kcmmouse", "Swap left and right buttons.")
            }

            RowLayout {
                spacing: Kirigami.Units.smallSpacing
                QQC2.CheckBox {
                    id: middleEmulation
                    text: i18nd("kcmmouse", "Press left and right buttons for middle-click")
                    enabled: device?.supportsMiddleEmulation ?? false
                    checked: enabled && device.middleEmulation

                    onToggled: {
                        if (device) {
                            device.middleEmulation = checked
                            root.changeSignal()
                        }
                    }

                    QQC2.ToolTip.delay: 1000
                    QQC2.ToolTip.visible: hovered
                    QQC2.ToolTip.text: i18nd("kcmmouse", "Clicking left and right button simultaneously sends middle button click.")
                }

                Kirigami.ContextualHelpButton {
                    toolTipText: i18nd("kcmmouse", "Activating this setting increases mouse click latency by 50ms. The extra delay is needed to correctly detect simultaneous left and right mouse clicks.")
                }
            }

            Item {
                Kirigami.FormData.isSection: false
            }

            // Acceleration
            RowLayout {
                Kirigami.FormData.label: i18nd("kcmmouse", "Pointer speed:")
                id: accelSpeed
                Layout.fillWidth: true

                function onAccelSpeedChanged(val) {
                    if ((val / 1000) != device.pointerAcceleration) {
                        device.pointerAcceleration = val / 100
                        root.changeSignal()
                    }
                }

                QQC2.Slider {
                    id: accelSpeedSlider
                    Layout.fillWidth: true

                    from: 1
                    to: 11
                    stepSize: 1
                    enabled: device?.supportsPointerAcceleration ?? false

                    // convert libinput pointer acceleration range [-1, 1] to slider range [1, 11]
                    value: enabled ? Math.round(6 + device.pointerAcceleration / 0.2) : 0

                    onMoved: {
                        if (device) {
                            // convert slider range [1, 11] to accelSpeedValue range [-100, 100]
                            const accelSpeedValue = Math.round(((value - 6) * 0.2) * 100)
                            accelSpeed.onAccelSpeedChanged(accelSpeedValue)
                        }
                    }
                }

                QQC2.SpinBox {
                    id: accelSpeedSpinbox
                    Layout.minimumWidth: Kirigami.Units.gridUnit * 5

                    from: -100
                    to: 100
                    stepSize: 1
                    editable: true
                    enabled: device?.supportsPointerAcceleration ?? false

                    // if existing configuration or another application set a value with more than 2 decimals
                    // we reduce the precision to 2
                    value: enabled ? Math.round(device.pointerAcceleration * 100) : 0

                    validator: DoubleValidator {
                        bottom: accelSpeedSpinbox.from
                        top: accelSpeedSpinbox.to
                    }

                    onValueModified: {
                        if (device) {
                            accelSpeed.onAccelSpeedChanged(value)
                            // Keyboard input breaks SpinBox value bindings with current Qt.
                            // Restore the binding so clicking "Reset" will update it correctly.
                            value = Qt.binding(() => accelSpeedSpinbox.enabled ? Math.round(device.pointerAcceleration * 100) : 0)
                        }
                    }

                    textFromValue: function(val, locale) {
                        return Number(val / 100).toLocaleString(locale, "f", 2)
                    }

                    valueFromText: function(text, locale) {
                        return Number.fromLocaleString(locale, text) * 100
                    }
                }
            }

            ColumnLayout {
                id: accelProfile
                spacing: Kirigami.Units.smallSpacing
                Kirigami.FormData.label: i18nd("kcmmouse", "Pointer acceleration:")
                Kirigami.FormData.buddyFor: accelProfileFlat
                enabled: device?.supportsPointerAccelerationProfileAdaptive ?? false

                QQC2.ButtonGroup {
                    buttons: [accelProfileFlat, accelProfileAdaptive]
                    onClicked: {
                        if (device) {
                            device.pointerAccelerationProfileFlat = accelProfileFlat.checked
                            device.pointerAccelerationProfileAdaptive = accelProfileAdaptive.checked
                            root.changeSignal()
                        }
                    }
                }
                QQC2.RadioButton {
                    id: accelProfileFlat
                    text: i18nd("kcmmouse", "None")
                    checked: accelProfile.enabled && device.pointerAccelerationProfileFlat

                    QQC2.ToolTip.delay: 1000
                    QQC2.ToolTip.visible: hovered
                    QQC2.ToolTip.text: i18nd("kcmmouse", "Cursor moves the same distance as the mouse movement.")
                }
                QQC2.RadioButton {
                    id: accelProfileAdaptive
                    text: i18nd("kcmmouse", "Standard")
                    checked: accelProfile.enabled && device.pointerAccelerationProfileAdaptive

                    QQC2.ToolTip.delay: 1000
                    QQC2.ToolTip.visible: hovered
                    QQC2.ToolTip.text: i18nd("kcmmouse", "Cursor travel distance depends on the mouse movement speed.")
                }
            }

            Item {
                Kirigami.FormData.isSection: false
            }

            // Scrolling
            QQC2.CheckBox {
                id: naturalScroll
                Kirigami.FormData.label: i18nd("kcmmouse", "Scrolling:")
                text: i18nd("kcmmouse", "Invert scroll direction")
                enabled: device?.supportsNaturalScroll ?? false
                checked: enabled && device.naturalScroll

                onToggled: {
                    if (device) {
                        device.naturalScroll = checked
                        root.changeSignal()
                    }
                }

                QQC2.ToolTip.delay: 1000
                QQC2.ToolTip.visible: hovered
                QQC2.ToolTip.text: i18nd("kcmmouse", "Touchscreen like scrolling.")
            }

            // Scroll Speed aka scroll Factor
            GridLayout {
                Kirigami.FormData.label: i18nd("kcm_touchpad", "Scrolling speed:")
                Kirigami.FormData.buddyFor: scrollFactor
                Layout.fillWidth: true

                visible: !backend.isAnonymousInputDevice
                columns: 3

                QQC2.Slider {
                    id: scrollFactor
                    Layout.fillWidth: true
                    Layout.columnSpan: 3

                    from: 0
                    to: 14
                    stepSize: 1
                    enabled: device

                    readonly property list<real> values: [
                        0.1,
                        0.3,
                        0.5,
                        0.75,
                        1, // default
                        1.5,
                        2,
                        3,
                        4,
                        5,
                        7,
                        9,
                        12,
                        15,
                        20
                    ]

                    function indexOf(val) {
                        const index = values.indexOf(val)
                        return index === -1 ? values.indexOf(1) : index
                    }
                    value: indexOf(device?.scrollFactor ?? 1)

                    onMoved: {
                        if (device) {
                            device.scrollFactor = values[value]
                            root.changeSignal()
                        }
                    }
                }

                //row 2
                QQC2.Label {
                    text: i18ndc("kcmmouse", "Slower Scroll", "Slower")
                    textFormat: Text.PlainText
                }
                Item {
                    Layout.fillWidth: true
                }
                QQC2.Label {
                    text: i18ndc("kcmmouse", "Faster Scroll Speed", "Faster")
                    textFormat: Text.PlainText
                }

            }

            Item {
                Kirigami.FormData.isSection: true
            }

            QQC2.Button  {
                text: i18ndc("kcmmouse", "@action:button", "Re-bind Additional Mouse Buttons…")
                visible: !backend.isAnonymousInputDevice && (
                    buttonMappings.model.length > 0 || Array.prototype.some.call(backend.inputDevices, supportsExtraButtons)
                )
                onClicked: root.pageStack.push(buttonPage)

                function supportsExtraButtons(device) {
                    return (device?.supportedButtons ?? 0) & ~(Qt.LeftButton | Qt.RightButton | Qt.MiddleButton);
                }
            }
        }
    }

    Kirigami.ScrollablePage {
        id: buttonPage
        visible: false

        MouseArea {
            // Deliberately using MouseArea on the page instead of a TapHandler on the button, so we can capture clicks anywhere
            id: buttonCapture
            property var lastButton: {}

            anchors.fill: parent
            enabled: newBinding.checked
            preventStealing: true
            acceptedButtons: Qt.AllButtons & ~(Qt.LeftButton | Qt.RightButton | Qt.MiddleButton)
            onClicked: {
                lastButton = buttonMappings.extraButtons.find(entry => Qt[entry.buttonName] === mouse.button)
                newBinding.visible = false
                newKeySequenceItem.visible = true
                newKeySequenceItem.startCapturing()
            }
        }

        ColumnLayout {
            Kirigami.FormLayout {
                id: buttonLayout
                twinFormLayouts: otherLayout
                Repeater {
                    id: buttonMappings
                    model: extraButtons?.filter(entry => backend.buttonMapping?.hasOwnProperty(entry.buttonName)) ?? []

                    readonly property var extraButtons: Array.from({length: 24}, (value, index) => ({
                        buttonName: "ExtraButton" + (index + 1),
                        button: Qt["ExtraButton" + (index + 1)],
                        label: i18ndc("kcmmouse", "@label for assigning an action to a numbered button", "Extra Button %1:", index + 1)
                    }))

                    delegate: KQuickControls.KeySequenceItem {
                        Kirigami.FormData.label: modelData.label

                        keySequence: backend.buttonMapping[modelData.buttonName]

                        modifierlessAllowed: true
                        modifierOnlyAllowed: true
                        multiKeyShortcutsAllowed: false
                        checkForConflictsAgainst: KQuickControls.ShortcutType.None

                        onCaptureFinished: {
                            const copy = backend.buttonMapping;
                            copy[modelData.buttonName] = keySequence
                            backend.buttonMapping = copy
                            root.changeSignal()
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
                actions: [
                    Kirigami.Action {
                        icon.name: "dialog-cancel"
                        text: i18ndc("kcmmouse", "@action:button", "Cancel")
                        onTriggered: {
                            newKeySequenceItem.visible = false;
                            newBinding.visible = true
                            newBinding.checked = false
                        }
                    }
                ]
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

                    modifierlessAllowed: true
                    modifierOnlyAllowed: true
                    multiKeyShortcutsAllowed: false
                    checkForConflictsAgainst: KQuickControls.ShortcutType.None

                    onCaptureFinished: {
                        visible = false
                        newBinding.visible = true
                        newBinding.checked = false
                        const copy = backend.buttonMapping;
                        copy[buttonCapture.lastButton.buttonName] = keySequence
                        backend.buttonMapping = copy
                        root.changeSignal()
                    }
                }


                Item {
                    Kirigami.FormData.isSection: true
                }

                QQC2.Button  {
                    onClicked: root.pageStack.pop()
                    text: i18ndc("kcmmouse", "@action:button", "Go back")
                    icon.name: "go-previous"
                }
            }
        }
    }
}
