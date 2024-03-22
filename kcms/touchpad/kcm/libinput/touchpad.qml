/*
    SPDX-FileCopyrightText: 2017 Roman Gilg <subdiff@gmail.com>
    SPDX-FileCopyrightText: 2018 Furkan Tokac <furkantokac34@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kcmutils as KCM
import org.kde.kirigami as Kirigami

import org.kde.touchpad.kcm

KCM.SimpleKCM {
    id: root

    spacing: Kirigami.Units.smallSpacing

    property alias deviceIndex: deviceSelector.currentIndex
    signal changeSignal()

    property QtObject touchpad
    property int touchpadCount: backend.touchpadCount

    property bool loading: false

    function resetModel(index) {
        touchpadCount = backend.touchpadCount
        formLayout.enabled = touchpadCount
        deviceSelector.enabled = touchpadCount > 1

        loading = true
        if (touchpadCount) {
            touchpad = deviceModel[index]
            deviceSelector.model = deviceModel
            deviceSelector.currentIndex = index
        } else {
            deviceSelector.model = [""]
        }
        loading = false
    }

    function syncValuesFromBackend() {
        loading = true

        deviceEnabled.load()
        dwt.load()
        leftHanded.load()
        middleEmulation.load()
        accelSpeedSpinbox.load()
        accelSpeedSlider.load()
        accelProfile.load()
        tapToClick.load()
        tapAndDrag.load()
        tapAndDragLock.load()
        multiTap.load()
        scrollMethod.load()
        naturalScroll.load()
        scrollFactor.load()
        rightClickMethod.load()
        middleClickMethod.load()
        disableHorizontalScrolling.load()

        loading = false
    }

    header: Kirigami.InlineMessage {
        id: inlineMessage
    }

    Connections {
        target: TouchpadConfig

        function onShowMessage(message, type) {

            if (touchpadCount === 0) {
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
        explanation: i18n("Connect an external touchpad");
        anchors.centerIn: parent
        visible: touchpadCount === 0
        width: parent.width - (Kirigami.Units.largeSpacing * 4)
    }


    Kirigami.FormLayout {
        id: formLayout

        visible: touchpadCount > 0

        // Device
        QQC2.ComboBox {
            Kirigami.FormData.label: i18nd("kcm_touchpad", "Device:")
            id: deviceSelector

            visible: touchpadCount > 1
            Layout.fillWidth: true
            model: deviceModel
            textRole: "name"

            onCurrentIndexChanged: {
                if (touchpadCount) {
                    touchpad = deviceModel[currentIndex]
                    if (!loading) {
                        changeSignal()
                    }
                }
                root.syncValuesFromBackend()
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

            hoverEnabled: true
            QQC2.ToolTip {
                text: i18nd("kcm_touchpad", "Accept input through this device.")
                visible: parent.hovered
                delay: 1000
            }

            function load() {
                if (!formLayout.enabled) {
                    checked = false
                    return
                }
                enabled = touchpad.supportsDisableEvents
                checked = enabled && touchpad.enabled
            }

            onCheckedChanged: {
                if (enabled && !root.loading) {
                    touchpad.enabled = checked
                    root.changeSignal()
                }
            }
        }

        QQC2.CheckBox {
            id: dwt
            text: i18nd("kcm_touchpad", "Disable while typing")

            hoverEnabled: true
            QQC2.ToolTip {
                text: i18nd("kcm_touchpad", "Disable touchpad while typing to prevent accidental inputs.")
                visible: parent.hovered
                delay: 1000
            }

            function load() {
                if (!formLayout.enabled) {
                    checked = false
                    return
                }
                enabled = touchpad.supportsDisableWhileTyping
                checked = enabled && touchpad.disableWhileTyping
            }

            onCheckedChanged: {
                if (enabled && !root.loading) {
                    touchpad.disableWhileTyping = checked
                    root.changeSignal()
                }
            }
        }

        QQC2.CheckBox {
            id: leftHanded
            text: i18nd("kcm_touchpad", "Left handed mode")

            hoverEnabled: true
            QQC2.ToolTip {
                text: i18nd("kcm_touchpad", "Swap left and right buttons.")
                visible: parent.hovered
                delay: 1000
            }

            function load() {
                if (!formLayout.enabled) {
                    checked = false
                    return
                }
                enabled = touchpad.supportsLeftHanded
                checked = enabled && touchpad.leftHanded
            }

            onCheckedChanged: {
                if (enabled && !root.loading) {
                    touchpad.leftHanded = checked
                    root.changeSignal()
                }
            }
        }

        RowLayout {
            spacing: Kirigami.Units.smallSpacing
            QQC2.CheckBox {
                id: middleEmulation
                text: i18nd("kcm_touchpad", "Press left and right buttons for middle click")

                hoverEnabled: true
                QQC2.ToolTip {
                    text: i18nd("kcm_touchpad", "Clicking left and right button simultaneously sends middle button click.")
                    visible: parent.hovered
                    delay: 1000
                }

                function load() {
                    if (!formLayout.enabled) {
                        checked = false
                        return
                    }
                    enabled = touchpad.supportsMiddleEmulation
                    checked = enabled && touchpad.middleEmulation
                }

                onCheckedChanged: {
                    if (enabled && !root.loading) {
                        touchpad.middleEmulation = checked
                        root.changeSignal()
                    }
                    loading = true
                    middleClickMethod.load()
                    loading = false
                }
            }

            Kirigami.ContextualHelpButton {
                toolTipText: i18nd("kcm_touchpad", "Activating this setting increases mouse click latency by 50ms. The extra delay is needed to correctly detect simultaneous left and right mouse clicks.")
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
                // check slider
                if (val !== accelSpeedSlider.accelSpeedValue) {
                    accelSpeedSlider.accelSpeedValue = val
                    accelSpeedSlider.value = Math.round(6 + (val / 100) / 0.2)
                }

                // check spinbox
                if (val !== accelSpeedSpinbox.value) {
                    accelSpeedSpinbox.value = val
                }

                // check libinput accelspeed
                if ((val / 100) !== touchpad.pointerAcceleration) {
                    touchpad.pointerAcceleration = val / 100
                    root.changeSignal()
                }
            }

            QQC2.Slider {
                id: accelSpeedSlider
                Layout.fillWidth: true

                from: 1
                to: 11
                stepSize: 1
                property int accelSpeedValue: 0 // [-100, 100]

                function load() {
                    enabled = touchpad.supportsPointerAcceleration
                    if (!enabled) {
                        return
                    }

                    accelSpeedValue = Math.round(touchpad.pointerAcceleration * 100)

                    // convert libinput pointer acceleration range [-1, 1] to slider range [1, 11]
                    value = Math.round(6 + touchpad.pointerAcceleration / 0.2)
                }

                onValueChanged: {
                    if (touchpad != undefined && enabled && !root.loading) {
                        // convert slider range [1, 11] to accelSpeedValue range [-100, 100]
                        accelSpeedValue = Math.round(((value - 6) * 0.2) * 100)

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

                validator: DoubleValidator {
                    bottom: accelSpeedSpinbox.from
                    top: accelSpeedSpinbox.to
                }

                function load() {
                    enabled = touchpad.supportsPointerAcceleration
                    if (!enabled) {
                        return
                    }

                    // if existing configuration or another application set a value with more than 2 decimals
                    // we reduce the precision to 2
                    value = Math.round(touchpad.pointerAcceleration * 100)
                }

                onValueChanged: {
                    if (touchpad != undefined && enabled && !root.loading) {
                        accelSpeed.onAccelSpeedChanged(value)
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
            Kirigami.FormData.label: i18nd("kcm_touchpad", "Pointer acceleration:")
            Kirigami.FormData.buddyFor: accelProfileFlat
            spacing: Kirigami.Units.smallSpacing

            function load() {
                enabled = touchpad.supportsPointerAccelerationProfileAdaptive

                if (!enabled) {
                    accelProfile.visible = false
                    accelProfileFlat.checked = false
                    accelProfileAdaptive.checked = false
                    return
                }

                if(touchpad.pointerAccelerationProfileAdaptive) {
                    accelProfileAdaptive.checked = true
                } else {
                    accelProfileFlat.checked = true
                }
            }

            function syncCurrent() {
                if (enabled && !root.loading) {
                    touchpad.pointerAccelerationProfileFlat = accelProfileFlat.checked
                    touchpad.pointerAccelerationProfileAdaptive = accelProfileAdaptive.checked
                    root.changeSignal()
                }
            }

            QQC2.RadioButton {
                id: accelProfileFlat
                text: i18nd("kcm_touchpad", "None")

                hoverEnabled: true
                QQC2.ToolTip {
                    text: i18nd("kcm_touchpad", "Cursor moves the same distance as finger.")
                    visible: parent.hovered
                    delay: 1000
                }
                onCheckedChanged: accelProfile.syncCurrent()
            }

            QQC2.RadioButton {
                id: accelProfileAdaptive
                text: i18nd("kcm_touchpad", "Standard")

                hoverEnabled: true
                QQC2.ToolTip {
                    text: i18nd("kcm_touchpad", "Cursor travel distance depends on movement speed of finger.")
                    visible: parent.hovered
                    delay: 1000
                }
                onCheckedChanged: accelProfile.syncCurrent()
            }
        }

        Item {
            Kirigami.FormData.isSection: false
        }

        // Tapping
        QQC2.CheckBox {
            id: tapToClick
            Kirigami.FormData.label: i18nd("kcm_touchpad", "Tapping:")
            text: i18nd("kcm_touchpad", "Tap-to-click")

            hoverEnabled: true
            QQC2.ToolTip {
                text: i18nd("kcm_touchpad", "Single tap is left button click.")
                visible: parent.hovered
                delay: 1000
            }

            function load() {
                enabled = touchpad.tapFingerCount > 0
                checked = enabled && touchpad.tapToClick
            }

            function updateDependents() {
                loading = true
                tapAndDrag.load()
                tapAndDragLock.load()
                multiTap.load()
                loading = false
            }

            onCheckedChanged: {
                if (enabled && !root.loading) {
                    touchpad.tapToClick = checked
                    updateDependents()
                    root.changeSignal()
                }
            }
        }

        QQC2.CheckBox {
            id: tapAndDrag
            text: i18nd("kcm_touchpad", "Tap-and-drag")

            hoverEnabled: true
            QQC2.ToolTip {
                text: i18nd("kcm_touchpad", "Sliding over touchpad directly after tap drags.")
                visible: parent.hovered
                delay: 1000
            }

            function load() {
                enabled = touchpad.tapFingerCount > 0 && tapToClick.checked
                checked = enabled && touchpad.tapAndDrag
            }

            function updateDependents() {
                loading = true
                tapAndDragLock.load()
                loading = false
            }

            onCheckedChanged: {
                if (enabled && !root.loading) {
                    touchpad.tapAndDrag = checked
                    updateDependents()
                    root.changeSignal()
                }
            }
        }

        QQC2.CheckBox {
            id: tapAndDragLock
            text: i18nd("kcm_touchpad", "Tap-and-drag lock")

            hoverEnabled: true
            QQC2.ToolTip {
                text: i18nd("kcm_touchpad", "Dragging continues after a short finger lift.")
                visible: parent.hovered
                delay: 1000
            }

            function load() {
                enabled = touchpad.tapFingerCount > 0 && tapAndDrag.checked
                checked = enabled && touchpad.tapDragLock
            }

            onCheckedChanged: {
                if (enabled && !root.loading) {
                    touchpad.tapDragLock = checked
                    root.changeSignal()
                }
            }
        }

        ColumnLayout {
            id: multiTap
            Kirigami.FormData.label: i18nd("kcm_touchpad", "Two-finger tap:")
            Kirigami.FormData.buddyFor: multiTapRightClick

            spacing: Kirigami.Units.smallSpacing
            // hide initially
            visible: false

            function load() {
                visible = touchpad.supportsLmrTapButtonMap
                enabled = touchpad.supportsLmrTapButtonMap && tapToClick.checked
                if (touchpad.tapFingerCount > 2) {
                    multiTapRightClick.text = i18nd("kcm_touchpad", "Right-click (three-finger tap to middle-click)")
                    multiTapRightClickToolTip.text = i18nd("kcm_touchpad", "Tap with two fingers to right-click, tap with three fingers to middle-click.")

                    multiTapMiddleClick.text = i18nd("kcm_touchpad", "Middle-click (three-finger tap right-click)")
                    multiTapMiddleClickToolTip.text = i18nd("kcm_touchpad", "Tap with two fingers to middle-click, tap with three fingers to right-click.")
                } else {
                    multiTapRightClick.text = i18nd("kcm_touchpad", "Right-click")
                    multiTapRightClickToolTip.text = i18nd("kcm_touchpad", "Tap with two fingers to right-click.")

                    multiTapMiddleClick.text = i18nd("kcm_touchpad", "Middle-click")
                    multiTapMiddleClickToolTip.text = i18nd("kcm_touchpad", "Tap with two fingers to middle-click.")
                }

                if (!enabled) {
                    multiTapRightClick.checked = false
                    multiTapMiddleClick.checked = false
                    return
                }

                if (touchpad.lmrTapButtonMap) {
                    multiTapMiddleClick.checked = true
                } else {
                    multiTapRightClick.checked = true
                }
            }

            function syncCurrent() {
                if (enabled && !root.loading) {
                    touchpad.lmrTapButtonMap = multiTapMiddleClick.checked
                    root.changeSignal()
                }
            }

            QQC2.RadioButton {
                id: multiTapRightClick
                // text: is handled dynamically on load.

                hoverEnabled: true
                QQC2.ToolTip {
                    id: multiTapRightClickToolTip
                    visible: parent.hovered
                    delay: 1000
                    // text: is handled dynamically on load.
                }
                onCheckedChanged: multiTap.syncCurrent()
            }

            QQC2.RadioButton {
                id: multiTapMiddleClick
                // text: is handled dynamically on load.

                hoverEnabled: true
                QQC2.ToolTip {
                    id: multiTapMiddleClickToolTip
                    visible: parent.hovered
                    delay: 1000
                    // text: is handled dynamically on load.
                }
                onCheckedChanged: multiTap.syncCurrent()
            }
        }

        Item {
            Kirigami.FormData.isSection: false
        }

        // Scrolling
        ColumnLayout {
            id: scrollMethod
            Kirigami.FormData.label: i18nd("kcm_touchpad", "Scrolling:")
            Kirigami.FormData.buddyFor: scrollMethodTwoFingers

            spacing: Kirigami.Units.smallSpacing

            function load() {
                scrollMethodTwoFingers.enabled = touchpad.supportsScrollTwoFinger
                scrollMethodTouchpadEdges.enabled = touchpad.supportsScrollEdge

                if(scrollMethodTouchpadEdges.enabled && touchpad.scrollEdge) {
                    scrollMethodTouchpadEdges.checked = formLayout.enabled
                } else {
                    scrollMethodTwoFingers.checked = formLayout.enabled
                }
            }

            function syncCurrent() {
                if (enabled && !root.loading) {
                    touchpad.scrollTwoFinger = scrollMethodTwoFingers.checked
                    touchpad.scrollEdge = scrollMethodTouchpadEdges.checked
                    root.changeSignal()
                }
                loading = true
                naturalScroll.load()
                loading = false
            }

            QQC2.RadioButton {
                id: scrollMethodTwoFingers
                text: i18nd("kcm_touchpad", "Two fingers")

                hoverEnabled: true
                QQC2.ToolTip {
                    text: i18nd("kcm_touchpad", "Slide with two fingers scrolls.")
                    visible: parent.hovered
                    delay: 1000
                }
            }

            QQC2.RadioButton {
                id: scrollMethodTouchpadEdges
                text: i18nd("kcm_touchpad", "Touchpad edges")

                hoverEnabled: true
                QQC2.ToolTip {
                    text: i18nd("kcm_touchpad", "Slide on the touchpad edges scrolls.")
                    visible: parent.hovered
                    delay: 1000
                }
                onCheckedChanged: scrollMethod.syncCurrent()
            }
        }

        QQC2.CheckBox {
            id: naturalScroll
            text: i18nd("kcm_touchpad", "Invert scroll direction (Natural scrolling)")

            function load() {
                enabled = touchpad.supportsNaturalScroll
                checked = enabled && touchpad.naturalScroll
            }

            onCheckedChanged: {
                if (enabled && !root.loading) {
                    touchpad.naturalScroll = checked
                    root.changeSignal()
                }
            }

            hoverEnabled: true
            QQC2.ToolTip {
                text: i18nd("kcm_touchpad", "Touchscreen like scrolling.")
                visible: parent.hovered
                delay: 1000
            }
        }

        QQC2.CheckBox {
            id: disableHorizontalScrolling
            text: i18nd("kcm_touchpad", "Disable horizontal scrolling")

            function load() {
                visible = touchpad.supportsHorizontalScrolling
                enabled = touchpad.supportsHorizontalScrolling
                checked = enabled && !touchpad.horizontalScrolling
            }

            onCheckedChanged: {
                if (enabled && !root.loading) {
                    touchpad.horizontalScrolling = !checked
                    root.changeSignal()
                }
            }

            hoverEnabled: true
            QQC2.ToolTip {
                text: i18nd("kcm_touchpad", "Disable horizontal scrolling")
                visible: parent.hovered
                delay: 1000
            }
        }

        // Scroll Speed aka scroll Factor
        GridLayout {
            Kirigami.FormData.label: i18nd("kcm_touchpad", "Scrolling speed:")
            Kirigami.FormData.buddyFor: scrollFactor
            Layout.fillWidth: true
            visible: touchpad.supportsScrollFactor

            columns: 3

            QQC2.Slider {
                id: scrollFactor
                Layout.fillWidth: true

                from: 0
                to: 14
                stepSize: 1

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

                Layout.columnSpan: 3

                function load() {
                    let index = values.indexOf(touchpad.scrollFactor)
                    if (index === -1) {
                        index = values.indexOf(1);
                    }
                    value = index
                }

                onMoved: {
                    touchpad.scrollFactor = values[value]
                    root.changeSignal()
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

        Item {
            Kirigami.FormData.isSection: false
        }

        ColumnLayout {
            id: rightClickMethod
            Kirigami.FormData.label: i18nd("kcm_touchpad", "Right-click:")
            Kirigami.FormData.buddyFor: rightClickMethodAreas

            spacing: Kirigami.Units.smallSpacing
            // hide initially
            visible: false

            function load() {
                enabled = touchpad.supportsClickMethodAreas && touchpad.supportsClickMethodClickfinger
                visible = touchpad.supportsClickMethodAreas || touchpad.supportsClickMethodClickfinger

                rightClickMethodAreas.enabled = touchpad.supportsClickMethodAreas
                rightClickMethodClickfinger.enabled = touchpad.supportsClickMethodClickfinger

                if (rightClickMethodAreas.enabled && touchpad.clickMethodAreas) {
                    rightClickMethodAreas.checked = true
                } else if (rightClickMethodClickfinger.enabled && touchpad.clickMethodClickfinger) {
                    rightClickMethodClickfinger.checked = true
                }
            }

            function syncCurrent() {
                if (enabled && !root.loading) {
                    touchpad.clickMethodAreas = rightClickMethodAreas.checked && rightClickMethodAreas.visible
                    touchpad.clickMethodClickfinger = rightClickMethodClickfinger.checked && rightClickMethodClickfinger.visible
                    root.changeSignal()
                }
                loading = true
                middleClickMethod.load()
                loading = false
            }

            QQC2.RadioButton {
                id: rightClickMethodAreas
                text: i18nd("kcm_touchpad", "Press bottom-right corner")

                hoverEnabled: true
                QQC2.ToolTip {
                    text: i18nd("kcm_touchpad", "Software enabled buttons will be added to bottom portion of your touchpad.")
                    visible: parent.hovered
                    delay: 1000
                }
            }

            QQC2.RadioButton {
                id: rightClickMethodClickfinger
                text: i18nd("kcm_touchpad", "Press anywhere with two fingers")

                hoverEnabled: true
                QQC2.ToolTip {
                    text: i18nd("kcm_touchpad", "Tap with two finger to enable right click.")
                    visible: parent.hovered
                    delay: 1000
                }
                onCheckedChanged: rightClickMethod.syncCurrent()
            }
        }

        Item {
            Kirigami.FormData.isSection: false
        }

        ColumnLayout {
            id: middleClickMethod
            Kirigami.FormData.label: i18nd("kcm_touchpad", "Middle-click: ")
            Kirigami.FormData.buddyFor: middleSoftwareEmulation

            spacing: Kirigami.Units.smallSpacing
            visible: noMiddleSoftwareEmulation.visible ||
                     middleSoftwareEmulation.visible ||
                     clickfingerMiddleInfoBox.visible

            function load() {
                enabled = touchpad.supportsMiddleEmulation
                if (enabled && touchpad.middleEmulation) {
                    middleSoftwareEmulation.checked = true
                } else {
                    noMiddleSoftwareEmulation.checked = true
                }
            }

            function syncCurrent() {
                if (enabled && !root.loading) {
                    touchpad.middleEmulation = middleSoftwareEmulation.checked && middleSoftwareEmulation.visible
                    root.changeSignal()
                }
                loading = true
                middleEmulation.load()
                loading = false
            }

            QQC2.RadioButton {
                id: noMiddleSoftwareEmulation
                text: i18nd("kcm_touchpad", "Press bottom-middle")
                visible: rightClickMethodAreas.checked
                hoverEnabled: true
                QQC2.ToolTip {
                    text: i18nd("kcm_touchpad", "Software enabled middle-button will be added to bottom portion of your touchpad.")
                    visible: parent.hovered
                    delay: 1000
                }
            }

            QQC2.RadioButton {
                id: middleSoftwareEmulation
                text: i18nd("kcm_touchpad", "Press bottom left and bottom right corners simultaneously")
                visible: rightClickMethodAreas.checked
                hoverEnabled: true
                QQC2.ToolTip {
                    text: i18nd("kcm_touchpad", "Clicking left and right button simultaneously sends middle button click.")
                    visible: parent.hovered
                    delay: 1000
                }
                onCheckedChanged: middleClickMethod.syncCurrent()
            }

            QQC2.CheckBox {
                id: clickfingerMiddleInfoBox
                text: i18nd("kcm_touchpad", "Press anywhere with three fingers")
                checked: true
                enabled: false
                visible: rightClickMethodClickfinger.checked
                hoverEnabled: true
                QQC2.ToolTip {
                    text: i18nd("kcm_touchpad", "Press anywhere with three fingers.")
                    visible: parent.hovered
                    delay: 1000
                }
            }
        }
    }
}
