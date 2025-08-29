/*
    SPDX-FileCopyrightText: 2017 Roman Gilg <subdiff@gmail.com>
    SPDX-FileCopyrightText: 2018 Furkan Tokac <furkantokac34@gmail.com>
    SPDX-FileCopyrightText: 2025 Jakob Petsovits <jpetso@petsovits.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kcmutils as KCM
import org.kde.kirigami as Kirigami

import org.kde.touchpad.kcm

import org.kde.plasma.private.kcm_touchpad as Touchpad

KCM.SimpleKCM {
    id: root

    spacing: Kirigami.Units.smallSpacing

    property alias deviceIndex: deviceSelector.currentIndex
    property Touchpad.InputDevice device: backend.inputDevices[deviceIndex] ?? null
    signal changeSignal()

    LayoutMirroring.enabled: Qt.application.layoutDirection === Qt.RightToLeft
    LayoutMirroring.childrenInherit: true

    headerPaddingEnabled: false // Let the InlineMessage touch the edges
    header: Kirigami.InlineMessage {
        id: inlineMessage
        position: Kirigami.InlineMessage.Position.Header
    }

    Connections {
        target: KCMTouchpad

        function onShowMessage(message, type) {

            if (!backend.inputDevices?.length) {
                return
            }

            if (message.length !== 0) {
                inlineMessage.text = message
                inlineMessage.type = type
                inlineMessage.visible = true
            } else {
                inlineMessage.visible = false
            }
        }
    }

    Kirigami.PlaceholderMessage {
        icon.name: "input-touchpad"
        text: i18nd("kcm_touchpad", "No touchpad found")
        explanation: i18nd("kcm_touchpad", "Connect an external touchpad");
        anchors.centerIn: parent
        visible: !backend.inputDevices?.length
        width: parent.width - (Kirigami.Units.largeSpacing * 4)
    }


    Kirigami.FormLayout {
        id: formLayout

        visible: backend.inputDevices?.length > 0
        enabled: visible

        // Device
        QQC2.ComboBox {
            Kirigami.FormData.label: i18nd("kcm_touchpad", "Device:")
            id: deviceSelector

            visible: count > 1
            Layout.fillWidth: true
            model: backend.inputDevices
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

        // General settings
        QQC2.CheckBox {
            id: deviceEnabled
            Kirigami.FormData.label: i18nd("kcm_touchpad", "General:")
            text: i18nd("kcm_touchpad", "Device enabled")
            enabled: root.device?.supportsDisableEvents ?? false
            checked: root.device && (!root.device.supportsDisableEvents || root.device.enabled)

            onToggled: {
                if (root.device) {
                    root.device.enabled = checked
                    root.changeSignal()
                }
            }
        }

        QQC2.CheckBox {
            id: disableEventsOnExternalMouse
            text: i18ndc("kcm_touchpad", "@option:check", "Disable while mouse is connected")
            leftPadding: Application.layoutDirection === Qt.LeftToRight ?
                deviceEnabled.contentItem.leftPadding : disableEventsOnExternalMouse.padding
            rightPadding: Application.layoutDirection === Qt.RightToLeft ?
                deviceEnabled.contentItem.rightPadding : disableEventsOnExternalMouse.padding
            enabled: root.device?.supportsDisableEventsOnExternalMouse ?? false
            checked: enabled && (root.device?.disableEventsOnExternalMouse ?? false)

            onToggled: {
                if (root.device) {
                    root.device.disableEventsOnExternalMouse = checked
                    root.changeSignal()
                }
            }
        }

        ColumnLayout {
            Kirigami.FormData.buddyFor: dwt
            spacing: 0

            QQC2.CheckBox {
                id: dwt
                text: i18nd("kcm_touchpad", "Disable while typing")
                Accessible.description: dwtInfoLabel.text
                leftPadding: Application.layoutDirection === Qt.LeftToRight ?
                    deviceEnabled.contentItem.leftPadding : disableEventsOnExternalMouse.padding
                rightPadding: Application.layoutDirection === Qt.RightToLeft ?
                    deviceEnabled.contentItem.rightPadding : disableEventsOnExternalMouse.padding
                enabled: root.device?.supportsDisableWhileTyping ?? false
                checked: enabled && (root.device?.disableWhileTyping ?? false)

                onToggled: {
                    if (root.device) {
                        root.device.disableWhileTyping = checked
                        root.changeSignal()
                    }
                }
            }
            QQC2.Label {
                id: dwtInfoLabel
                Layout.fillWidth: true
                leftPadding: Application.layoutDirection === Qt.LeftToRight ?
                    dwt.leftPadding + dwt.contentItem.leftPadding : dwt.padding
                rightPadding: Application.layoutDirection === Qt.RightToLeft ?
                    dwt.rightPadding + dwt.contentItem.rightPadding : dwt.padding
                text: i18ndc("kcm_touchpad", "@label 'this' refers to the 'disable touchpad while typing' feature", "This can interfere with video games.")
                textFormat: Text.PlainText
                elide: Text.ElideRight
                font: Kirigami.Theme.smallFont
                enabled: dwt.enabled
            }
        }

        QQC2.CheckBox {
            id: leftHanded
            text: i18nd("kcm_touchpad", "Left-handed mode")
            enabled: root.device?.supportsLeftHanded ?? false
            checked: enabled && (root.device?.leftHanded ?? false)

            onToggled: {
                if (root.device) {
                    root.device.leftHanded = checked
                    root.changeSignal()
                }
            }
        }

        RowLayout {
            spacing: Kirigami.Units.smallSpacing
            QQC2.CheckBox {
                id: middleEmulation
                text: i18ndc("kcm_touchpad", "@option:check", "Press left and right buttons to middle-click")
                enabled: root.device?.supportsMiddleEmulation ?? false
                checked: enabled && (root.device?.middleEmulation ?? false)

                onToggled: {
                    if (root.device) {
                        root.device.middleEmulation = checked
                        root.changeSignal()
                    }
                }
            }

            Kirigami.ContextualHelpButton {
                toolTipText: i18ndc("kcm_touchpad", "@info:tooltip from ContextualHelpButton", "Activating this setting increases click latency by 50ms. The extra delay is needed to correctly detect simultaneous left and right button presses.")
            }
        }

        Item {
            Kirigami.FormData.isSection: false
        }

        // Acceleration
        RowLayout {
            Kirigami.FormData.label: i18nd("kcm_touchpad", "Pointer speed:")
            id: accelSpeed
            Layout.fillWidth: true

            function onAccelSpeedChanged(val) {
                if ((val / 100) !== root.device.pointerAcceleration) {
                    root.device.pointerAcceleration = val / 100
                    root.changeSignal()
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
                value: enabled ? Math.round(6 + root.device.pointerAcceleration / 0.2) : 0

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

                Layout.minimumWidth: Kirigami.Units.gridUnit * 4

                from: -100
                to: 100
                stepSize: 1
                editable: true
                enabled: root.device?.supportsPointerAcceleration ?? false

                // if existing configuration or another application set a value with more than 2 decimals
                // we reduce the precision to 2
                value: enabled ? Math.round(root.device.pointerAcceleration * 100) : 0

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

        RowLayout {
            Kirigami.FormData.buddyFor: accelProfileEnabled
            spacing: Kirigami.Units.smallSpacing

            QQC2.CheckBox {
                id: accelProfileEnabled
                text: i18nd("kcm_touchpad", "Enable pointer acceleration")
                enabled: root.device?.supportsPointerAccelerationProfileAdaptive ?? false
                visible: enabled
                checked: enabled && !(root.device?.pointerAccelerationProfileFlat ?? false)

                onToggled: {
                    if (root.device) {
                        root.device.pointerAccelerationProfileFlat = !checked
                        root.device.pointerAccelerationProfileAdaptive = checked
                        root.changeSignal()
                    }
                }
            }

            Kirigami.ContextualHelpButton {
                toolTipText: i18ndc("kcm_touchpad", "@info:tooltip from ContextualHelpButton", "When enabled, pointer travel distance increases with faster movement speed.")
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

            visible: root.device?.supportsScrollFactor ?? false
            columns: 3

            QQC2.Slider {
                id: scrollFactor
                Layout.fillWidth: true
                Layout.columnSpan: 3

                from: 0
                to: 14
                stepSize: 1
                enabled: root.device

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
                value: indexOf(root.device?.scrollFactor ?? 1)

                onMoved: {
                    if (root.device) {
                        root.device.scrollFactor = values[value]
                        root.changeSignal()
                    }
                }
            }

            //row 2
            QQC2.Label {
                text: i18ndc("kcm_touchpad", "Slower Scroll", "Slower")
                textFormat: Text.PlainText
            }
            Item {
                Layout.fillWidth: true
            }
            QQC2.Label {
                text: i18ndc("kcm_touchpad", "Faster Scroll Speed", "Faster")
                textFormat: Text.PlainText
            }
        }

        // Scrolling
        ColumnLayout {
            id: scrollMethod
            Kirigami.FormData.label: i18nd("kcm_touchpad", "Scrolling:")
            Kirigami.FormData.buddyFor: scrollMethodTwoFingers
            visible: scrollMethodTwoFingers.enabled || scrollMethodTouchpadEdges.enabled

            spacing: Kirigami.Units.smallSpacing

            QQC2.ButtonGroup {
                buttons: [scrollMethodTwoFingers, scrollMethodTouchpadEdges]
                onClicked: {
                    if (root.device) {
                        root.device.scrollTwoFinger = scrollMethodTwoFingers.checked
                        root.device.scrollEdge = scrollMethodTouchpadEdges.checked
                        root.changeSignal()
                    }
                }
            }

            QQC2.RadioButton {
                id: scrollMethodTwoFingers
                text: i18nd("kcm_touchpad", "Two fingers")
                enabled: root.device?.supportsScrollTwoFinger ?? false
                checked: root.device?.scrollTwoFinger ?? false
            }

            QQC2.RadioButton {
                id: scrollMethodTouchpadEdges
                text: i18nd("kcm_touchpad", "Touchpad edges")
                enabled: root.device?.supportsScrollEdge ?? false
                checked: root.device?.scrollEdge ?? false
            }
        }

        QQC2.CheckBox {
            id: naturalScroll
            text: i18nd("kcm_touchpad", "Invert scroll direction (Natural scrolling)")
            enabled: root.device?.supportsNaturalScroll ?? false
            checked: enabled && (root.device?.naturalScroll ?? false)

            onToggled: {
                if (root.device) {
                    root.device.naturalScroll = checked
                    root.changeSignal()
                }
            }
        }

        QQC2.CheckBox {
            id: disableHorizontalScrolling
            text: i18nd("kcm_touchpad", "Disable horizontal scrolling")
            visible: root.device?.supportsHorizontalScrolling ?? false
            enabled: root.device?.supportsHorizontalScrolling ?? false
            checked: enabled && !(root.device?.horizontalScrolling ?? true)

            onToggled: {
                if (root.device) {
                    root.device.horizontalScrolling = !checked
                    root.changeSignal()
                }
            }
        }

        Item {
            Kirigami.FormData.isSection: false
        }

        // Tapping
        QQC2.CheckBox {
            id: tapToClick
            Kirigami.FormData.label: i18ndc("kcm_touchpad", "@label for checkbox, tap-to-click", "Tapping:")
            text: i18ndc("kcm_touchpad", "@option:check", "Tap to click")
            enabled: root.device?.tapFingerCount > 0
            checked: enabled && (root.device?.tapToClick ?? false)

            onToggled: {
                if (root.device) {
                    root.device.tapToClick = checked
                    root.changeSignal()
                }
            }
        }

        RowLayout {
            spacing: Kirigami.Units.smallSpacing
            QQC2.CheckBox {
                id: tapAndDrag
                text: i18nd("kcm_touchpad", "Tap-and-drag")
                enabled: root.device?.tapFingerCount > 0 && tapToClick.checked
                checked: enabled && (root.device?.tapAndDrag ?? false)

                onToggled: {
                    if (root.device) {
                        root.device.tapAndDrag = checked
                        root.changeSignal()
                    }
                }
            }

            Kirigami.ContextualHelpButton {
                toolTipText: i18ndc("kcm_touchpad", "@info:tooltip from ContextualHelpButton", "Tap, then tap and immediately slide finger over the touchpad to drag. Lift finger to drop.")
            }
        }


        QQC2.CheckBox {
            id: tapAndDragLock
            text: i18nd("kcm_touchpad", "Allow briefly lifting finger during tap-and-drag")
            enabled: root.device?.tapFingerCount > 0 && tapAndDrag.checked
            checked: enabled && (root.device?.tapDragLock ?? false)

            onToggled: {
                if (root.device) {
                    root.device.tapDragLock = checked
                    root.changeSignal()
                }
            }
        }

        ColumnLayout {
            id: multiTap
            Kirigami.FormData.label: (root.device?.tapFingerCount > 2
                ? i18nd("kcm_touchpad", "Tap to right-click:")
                : i18nd("kcm_touchpad", "Two-finger tap:") // right-click is only supported by one of the options
            )
            Kirigami.FormData.buddyFor: multiTapRightClick
            visible: root.device?.supportsLmrTapButtonMap
            enabled: root.device?.supportsLmrTapButtonMap && tapToClick.checked

            // spacing only on top of radio buttons, not between radio and help text label
            spacing: 0

            QQC2.ButtonGroup {
                buttons: [multiTapRightClick, multiTapMiddleClick]
                onClicked: {
                    if (root.device) {
                        root.device.lmrTapButtonMap = multiTapMiddleClick.checked
                        root.changeSignal()
                    }
                }
            }

            QQC2.RadioButton {
                id: multiTapRightClick
                text: (root.device?.tapFingerCount > 2
                    ? i18ndc("kcm_touchpad", "@option:radio touchpad tap to right-click", "Tap with two fingers")
                    : i18nd("kcm_touchpad", "Right-click")
                )
                Accessible.description: i18nd("kcm_touchpad", "Right-click by tapping with two fingers")
                checked: multiTap.enabled && !(root.device?.lmrTapButtonMap ?? false)
            }

            QQC2.Label {
                Layout.fillWidth: true
                visible: root.device?.tapFingerCount > 2
                leftPadding: Application.layoutDirection === Qt.LeftToRight ?
                    multiTapRightClick.contentItem.leftPadding : multiTapRightClick.padding
                rightPadding: Application.layoutDirection === Qt.RightToLeft ?
                    multiTapRightClick.contentItem.rightPadding : multiTapRightClick.padding
                text: i18ndc("kcm_touchpad", "@info shown below radio button", "Middle-click by tapping with three fingers.")
                textFormat: Text.PlainText
                elide: Text.ElideRight
                font: Kirigami.Theme.smallFont
            }

            QQC2.RadioButton {
                id: multiTapMiddleClick
                text: (root.device?.tapFingerCount > 2
                    ? i18ndc("kcm_touchpad", "@option:radio touchpad tap to right-click", "Tap with three fingers")
                    : i18nd("kcm_touchpad", "Middle-click")
                )
                Accessible.description: root.device?.tapFingerCount > 2
                    ? i18nd("kcm_touchpad", "Right-click by tapping with three fingers")
                    : i18nd("kcm_touchpad", "Middle-click")
                topPadding: Kirigami.Units.smallSpacing // in lieu of multiTap.spacing
                checked: multiTap.enabled && (root.device?.lmrTapButtonMap ?? false)
            }

            QQC2.Label {
                Layout.fillWidth: true
                visible: root.device?.tapFingerCount > 2
                leftPadding: Application.layoutDirection === Qt.LeftToRight ?
                    multiTapMiddleClick.contentItem.leftPadding : multiTapMiddleClick.padding
                rightPadding: Application.layoutDirection === Qt.RightToLeft ?
                    multiTapMiddleClick.contentItem.rightPadding : multiTapMiddleClick.padding
                text: i18ndc("kcm_touchpad", "@info shown below radio button", "Middle-click by tapping with two fingers.")
                textFormat: Text.PlainText
                elide: Text.ElideRight
                font: Kirigami.Theme.smallFont
            }
        }

        Item {
            Kirigami.FormData.isSection: false
        }

        ColumnLayout {
            id: rightClickMethod
            Kirigami.FormData.label: i18ndc("kcm_touchpad", "@label for radiobutton group, configure right-click with touch-pad integrated button (pressing into the touchpad)", "Press to right-click:")
            Kirigami.FormData.buddyFor: rightClickMethodAreas
            enabled: (root.device?.supportsClickMethodAreas && root.device?.supportsClickMethodClickfinger) ?? false
            visible: (root.device?.supportsClickMethodAreas || root.device?.supportsClickMethodClickfinger) ?? false

            // spacing only on top of radio buttons, not between radio and help text label
            spacing: 0

            QQC2.ButtonGroup {
                buttons: [rightClickMethodAreas, rightClickMethodClickfinger]
                onClicked: {
                    if (root.device) {
                        root.device.clickMethodAreas = rightClickMethodAreas.checked && rightClickMethodAreas.visible
                        root.device.clickMethodClickfinger = rightClickMethodClickfinger.checked && rightClickMethodClickfinger.visible
                        root.changeSignal()
                    }
                }
            }

            QQC2.RadioButton {
                id: rightClickMethodAreas
                text: i18ndc("kcm_touchpad", "@option:radio touchpad integrated right-click", "Press bottom-right corner")
                enabled: root.device?.supportsClickMethodAreas ?? false
                checked: enabled && (root.device?.clickMethodAreas ?? false)
                Accessible.description: rightClickMethodAreasInfoLabel.visible ? rightClickMethodAreasInfoLabel.text : ""
            }

            QQC2.Label {
                id: rightClickMethodAreasInfoLabel
                Layout.fillWidth: true
                visible: !middleClickMethod.visible
                leftPadding: Application.layoutDirection === Qt.LeftToRight ?
                    rightClickMethodAreas.contentItem.leftPadding : rightClickMethodAreas.padding
                rightPadding: Application.layoutDirection === Qt.RightToLeft ?
                    rightClickMethodAreas.contentItem.rightPadding : rightClickMethodAreas.padding
                text: middleEmulation.checked
                    ? i18ndc("kcm_touchpad", "@info shown below radio button", "Middle-click by pressing both bottom corners.")
                    : i18ndc("kcm_touchpad", "@info shown below radio button", "Middle-click by pressing bottom center.")
                textFormat: Text.PlainText
                elide: Text.ElideRight
                font: Kirigami.Theme.smallFont
            }

            QQC2.RadioButton {
                id: rightClickMethodClickfinger
                text: i18ndc("kcm_touchpad", "@option:radio touchpad integrated right-click", "Press touchpad with two fingers")
                Accessible.description: rightClickMethodClickfingerInfoLabel.visible ? rightClickMethodClickfingerInfoLabel.text : ""
                topPadding: Kirigami.Units.smallSpacing // in lieu of rightClickMethod.spacing
                enabled: root.device?.supportsClickMethodClickfinger ?? false
                checked: enabled && (root.device?.clickMethodClickfinger ?? false)
            }

            QQC2.Label {
                id: rightClickMethodClickfingerInfoLabel
                Layout.fillWidth: true
                visible: !middleClickMethod.visible
                leftPadding: Application.layoutDirection === Qt.LeftToRight ?
                    rightClickMethodClickfinger.contentItem.leftPadding : rightClickMethodClickfinger.padding
                rightPadding: Application.layoutDirection === Qt.RightToLeft ?
                    rightClickMethodClickfinger.contentItem.rightPadding : rightClickMethodClickfinger.padding
                text: i18ndc("kcm_touchpad", "@info shown below radio button", "Middle-click by pressing with three fingers.")
                textFormat: Text.PlainText
                elide: Text.ElideRight
                font: Kirigami.Theme.smallFont
            }
        }

        ColumnLayout {
            id: middleClickMethod
            Kirigami.FormData.label: i18ndc("kcm_touchpad", "@label for radiobutton group, configure middle-click with touch-pad integrated button (pressing into the touchpad)", "Press to middle-click:")
            Kirigami.FormData.buddyFor: middleSoftwareEmulation
            enabled: root.device?.supportsMiddleEmulation ?? false

            spacing: Kirigami.Units.smallSpacing
            visible: noMiddleSoftwareEmulation.visible ||
                     middleSoftwareEmulation.visible ||
                     clickfingerMiddleInfoBox.visible

            QQC2.ButtonGroup {
                buttons: [noMiddleSoftwareEmulation, middleSoftwareEmulation]
                onClicked: {
                    if (root.device) {
                        root.device.middleEmulation = middleSoftwareEmulation.checked && middleSoftwareEmulation.visible
                        root.changeSignal()
                    }
                }
            }

            QQC2.RadioButton {
                id: noMiddleSoftwareEmulation
                text: i18ndc("kcm_touchpad", "@option:radio touchpad integrated middle-click", "Press bottom middle edge")
                visible: rightClickMethodAreas.checked
                checked: middleClickMethod.enabled && !(root.device?.middleEmulation ?? false)
            }

            QQC2.RadioButton {
                id: middleSoftwareEmulation
                text: i18ndc("kcm_touchpad", "@option:radio touchpad integrated middle-click", "Press bottom left and bottom right corners")
                visible: rightClickMethodAreas.checked
                checked: middleClickMethod.enabled && (root.device?.middleEmulation ?? false)
            }

            QQC2.CheckBox {
                id: clickfingerMiddleInfoBox
                text: i18ndc("kcm_touchpad", "@option:check touchpad integrated middle-click", "Press touchpad with three fingers")
                checked: true
                enabled: false
                visible: rightClickMethodClickfinger.checked
            }
        }
    }
}
