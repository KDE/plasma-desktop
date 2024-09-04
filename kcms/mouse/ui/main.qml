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
import QtQuick.Layouts
import org.kde.kcmutils as KCMUtils

import org.kde.kirigami as Kirigami
import org.kde.kquickcontrols as KQuickControls

import org.kde.plasma.private.kcm_mouse as Mouse

KCMUtils.SimpleKCM {
    id: root

    // Would be nice to use a `required` qualifier, but KQuickManagedConfigModule doesn't
    // have an API to provide a mapping of initial properties for the root page.
    readonly property Mouse.InputBackend backend: KCMUtils.ConfigModule.inputBackend

    readonly property Mouse.InputDevice device: backend.inputDevices[KCMUtils.ConfigModule.currentDeviceIndex] ?? null

    function supportsExtraButtons(device: Mouse.InputDevice): bool {
        return (device?.supportedButtons ?? 0) & ~(Qt.LeftButton | Qt.RightButton | Qt.MiddleButton);
    }

    actions: Kirigami.Action  {
        icon.name: "input-mouse-click-left-symbolic"
        text: i18ndc("kcmmouse", "@action:button", "Configure Extra Buttons…")

        visible: {
            if (root.backend.isAnonymousInputDevice) {
                return false;
            }
            return root.backend.buttonMappingCount > 0
                || root.backend.inputDevices.some(root.supportsExtraButtons);
        }

        onTriggered: source => {
            root.KCMUtils.ConfigModule.push("bindings.qml");
        }
    }

    header: Header {
        saveLoadMessage: root.KCMUtils.ConfigModule.saveLoadMessage
        hotplugMessage: root.KCMUtils.ConfigModule.hotplugMessage
    }

    Kirigami.FormLayout {
        id: formLayout
        enabled: root.device !== null && root.backend.inputDevices.length > 0

        // Device
        QQC2.ComboBox {
            id: deviceSelector
            Kirigami.FormData.label: i18nd("kcmmouse", "Device:")
            visible: !root.backend.isAnonymousInputDevice
            enabled: count > 1
            Layout.fillWidth: true
            model: root.backend.inputDevices
            textRole: "name"

            Component.onCompleted: {
                currentIndex = Qt.binding(() => root.KCMUtils.ConfigModule.currentDeviceIndex);
            }

            onActivated: {
                root.KCMUtils.ConfigModule.currentDeviceIndex = currentIndex;
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
            visible: !root.backend.isAnonymousInputDevice
            enabled: root.device?.supportsDisableEvents ?? false
            checked: enabled && (root.device?.enabled ?? false)

            onToggled: {
                if (root.device) {
                    root.device.enabled = checked
                }
            }

            QQC2.ToolTip.delay: 1000
            QQC2.ToolTip.visible: hovered
            QQC2.ToolTip.text: i18nd("kcmmouse", "Accept input through this device.")
        }

        QQC2.CheckBox {
            id: leftHanded
            Kirigami.FormData.label: deviceEnabled.visible ? null : deviceEnabled.Kirigami.FormData.label
            text: i18nd("kcmmouse", "Left-handed mode")
            enabled: root.device?.supportsLeftHanded ?? false
            checked: enabled && (root.device?.leftHanded ?? false)

            onToggled: {
                if (root.device) {
                    root.device.leftHanded = checked
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
                text: i18ndc("kcmmouse", "@option:check", "Press left and right buttons for middle-click")
                enabled: root.device?.supportsMiddleEmulation ?? false
                checked: enabled && (root.device?.middleEmulation ?? false)

                onToggled: {
                    if (root.device) {
                        root.device.middleEmulation = checked
                    }
                }

                QQC2.ToolTip.delay: 1000
                QQC2.ToolTip.visible: hovered
                QQC2.ToolTip.text: i18ndc("kcmmouse", "@info:tooltip" ,"Pressing the left and right button simultaneously acts as middle-click.")
            }

            Kirigami.ContextualHelpButton {
                toolTipText: i18ndc("kcmmouse", "@info:tooltip from ContextualHelpButton", "Activating this setting increases click latency by 50ms. The extra delay is needed to correctly detect simultaneous left and right button presses.")
            }
        }

        Item {
            Kirigami.FormData.isSection: false
        }

        // Acceleration
        RowLayout {
            id: accelSpeed

            Kirigami.FormData.label: i18nd("kcmmouse", "Pointer speed:")
            Layout.fillWidth: true

            spacing: Kirigami.Units.smallSpacing

            function onAccelSpeedChanged(value: int): void {
                if (root.device && (value / 1000) !== root.device.pointerAcceleration) {
                    root.device.pointerAcceleration = value / 100
                }
            }

            QQC2.Slider {
                id: accelSpeedSlider
                Layout.fillWidth: true

                from: 1
                to: 11
                stepSize: 1
                enabled: root.device?.supportsPointerAcceleration ?? false

                // convert libinput pointer acceleration range [-1, 1] to slider range [1, 11]
                value: enabled && root.device ? Math.round(6 + root.device.pointerAcceleration / 0.2) : 0

                onMoved: {
                    if (root.device) {
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
                enabled: root.device?.supportsPointerAcceleration ?? false

                // if existing configuration or another application set a value with more than 2 decimals
                // we reduce the precision to 2
                value: enabled && root.device ? Math.round(root.device.pointerAcceleration * 100) : 0

                validator: DoubleValidator {
                    bottom: accelSpeedSpinbox.from
                    top: accelSpeedSpinbox.to
                }

                onValueModified: {
                    if (root.device) {
                        accelSpeed.onAccelSpeedChanged(value)
                        // Keyboard input breaks SpinBox value bindings with current Qt.
                        // Restore the binding so clicking "Reset" will update it correctly.
                        value = Qt.binding(() => accelSpeedSpinbox.enabled && root.device
                            ? Math.round(root.device.pointerAcceleration * 100)
                            : 0
                        );
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
            enabled: root.device?.supportsPointerAccelerationProfileAdaptive ?? false

            QQC2.ButtonGroup {
                buttons: [accelProfileFlat, accelProfileAdaptive]
                onClicked: {
                    if (root.device) {
                        root.device.pointerAccelerationProfileFlat = accelProfileFlat.checked
                        root.device.pointerAccelerationProfileAdaptive = accelProfileAdaptive.checked
                    }
                }
            }

            QQC2.RadioButton {
                id: accelProfileFlat
                text: i18nd("kcmmouse", "None")
                checked: accelProfile.enabled && (root.device?.pointerAccelerationProfileFlat ?? false)

                QQC2.ToolTip.delay: 1000
                QQC2.ToolTip.visible: hovered
                QQC2.ToolTip.text: i18nd("kcmmouse", "Cursor moves the same distance as the mouse movement.")
            }

            QQC2.RadioButton {
                id: accelProfileAdaptive
                text: i18nd("kcmmouse", "Standard")
                checked: accelProfile.enabled && (root.device?.pointerAccelerationProfileAdaptive ?? false)

                QQC2.ToolTip.delay: 1000
                QQC2.ToolTip.visible: hovered
                QQC2.ToolTip.text: i18nd("kcmmouse", "Cursor travel distance depends on the mouse movement speed.")
            }
        }

        Item {
            Kirigami.FormData.isSection: false
        }

        // Scroll Speed aka scroll Factor
        GridLayout {
            Kirigami.FormData.label: i18nd("kcm_touchpad", "Scrolling speed:")
            Kirigami.FormData.buddyFor: scrollFactor
            Layout.fillWidth: true

            visible: !root.backend.isAnonymousInputDevice
            columns: 3

            QQC2.Slider {
                id: scrollFactor
                Layout.fillWidth: true
                Layout.columnSpan: 3

                from: 0
                to: 14
                stepSize: 1
                enabled: root.device !== null

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
                    20,
                ]

                function indexOf(val: real): int {
                    const index = values.indexOf(val)
                    return index === -1 ? values.indexOf(1) : index
                }
                value: indexOf(root.device?.scrollFactor ?? 1)

                onMoved: {
                    if (root.device) {
                        root.device.scrollFactor = values[value]
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

        // Scrolling
        QQC2.CheckBox {
            id: naturalScroll
            Kirigami.FormData.label: i18nd("kcmmouse", "Scrolling:")
            text: i18nd("kcmmouse", "Invert scroll direction")
            enabled: root.device?.supportsNaturalScroll ?? false
            checked: enabled && (root.device?.naturalScroll ?? false)

            onToggled: {
                if (root.device) {
                    root.device.naturalScroll = checked
                }
            }

            QQC2.ToolTip.delay: 1000
            QQC2.ToolTip.visible: hovered
            QQC2.ToolTip.text: i18nd("kcmmouse", "Touchscreen-like scrolling.")
        }

        QQC2.CheckBox {
            id: scrollOnButtonDown
            text: i18nd("kcmmouse", "Press middle button and move mouse to scroll")
            enabled: root.device?.supportsScrollOnButtonDown ?? false
            checked: enabled && (root.device?.scrollOnButtonDown ?? false)

            onToggled: {
                if (root.device) {
                    root.device.scrollOnButtonDown = checked
                }
            }

            QQC2.ToolTip.delay: 1000
            QQC2.ToolTip.visible: hovered
            QQC2.ToolTip.text: i18nd("kcmmouse", "Scrolling with the mouse while the middle button is pressed.")
        }
    }
}
