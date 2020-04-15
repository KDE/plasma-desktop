/*
 * Copyright 2017 Roman Gilg <subdiff@gmail.com>
 * Copyright 2018 Furkan Tokac <furkantokac34@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

import QtQuick 2.7
import QtQuick.Controls 2.0 as Controls
import QtQuick.Layouts 1.3 as Layouts

import org.kde.kcm 1.1 as KCM
import org.kde.kirigami 2.4 as Kirigami

// TODO: Change ScrollablePage as KCM.SimpleKCM
// after rewrite the KCM in KConfigModule.
Kirigami.ScrollablePage {
    id: root

    spacing: Kirigami.Units.smallSpacing

    property size minimumSizeHint: Qt.size(formLayout.width/2, deviceSelector.height)

    property alias deviceIndex: deviceSelector.currentIndex
    signal changeSignal()

    property QtObject touchpad
    property int touchpadCount: backend.touchpadCount

    property bool loading: false

    enabled: touchpadCount > 0

    function resetModel(index) {
        touchpadCount = backend.touchpadCount
        formLayout.enabled = touchpadCount
        deviceSelector.enabled = touchpadCount > 1

        loading = true
        if (touchpadCount) {
            touchpad = deviceModel[index]
            deviceSelector.model = deviceModel
            deviceSelector.currentIndex = index
            console.log("Touchpad configuration of device '" +
                        (index + 1) + " : " + touchpad.name + "' opened")
        } else {
            deviceSelector.model = [""]
            console.log("No touchpad found")
        }
        loading = false
    }

    function syncValuesFromBackend() {
        loading = true

        deviceEnabled.load()
        dwt.load()
        leftHanded.load()
        middleEmulation.load()
        accelSpeed.load()
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

    Kirigami.FormLayout {
        id: formLayout

        // Device
        Controls.ComboBox {
            Kirigami.FormData.label: i18nd("kcm_touchpad", "Device:")
            id: deviceSelector

            enabled: touchpadCount > 1
            Layouts.Layout.fillWidth: true
            model: deviceModel
            textRole: "name"

            onCurrentIndexChanged: {
                if (touchpadCount) {
                    touchpad = deviceModel[currentIndex]
                    if (!loading) {
                        changeSignal()
                    }
                    console.log("Touchpad configuration of device '" +
                                (currentIndex+1) + " : " + touchpad.name + "' opened")
                }
                root.syncValuesFromBackend()
            }
        }

        Item {
            Kirigami.FormData.isSection: false
        }

        // General settings
        Controls.CheckBox {
            Kirigami.FormData.label: i18nd("kcm_touchpad", "General:")
            id: deviceEnabled
            text: i18nd("kcm_touchpad", "Device enabled")

            hoverEnabled: true
            Controls.ToolTip {
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

        Controls.CheckBox {
            id: dwt
            text: i18nd("kcm_touchpad", "Disable while typing")

            hoverEnabled: true
            Controls.ToolTip {
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

        Controls.CheckBox {
            id: leftHanded
            text: i18nd("kcm_touchpad", "Left handed mode")

            hoverEnabled: true
            Controls.ToolTip {
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

        Controls.CheckBox {
            id: middleEmulation
            text: i18nd("kcm_touchpad", "Press left and right buttons for middle click")

            hoverEnabled: true
            Controls.ToolTip {
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

        Item {
            Kirigami.FormData.isSection: false
        }

        // Acceleration
        Controls.Slider {
            Kirigami.FormData.label: i18nd("kcm_touchpad", "Pointer speed:")
            id: accelSpeed

            from: 1
            to: 11
            stepSize: 1

            function load() {
                enabled = touchpad.supportsPointerAcceleration
                if (!enabled) {
                    value = 0.1
                    return
                }
                // transform libinput's pointer acceleration range [-1, 1] to slider range [1, 11]
                value = 6 + touchpad.pointerAcceleration / 0.2
            }

            onValueChanged: {
                if (touchpad != undefined && enabled && !root.loading) {
                    // transform slider range [1, 11] to libinput's pointer acceleration range [-1, 1]
                    // by *10 and /10, we ignore the floating points after 1 digit. This prevents from
                    // having a libinput value like 0.60000001
                    touchpad.pointerAcceleration = Math.round(((value-6) * 0.2) * 10) / 10
                    root.changeSignal()
                }
            }
        }

        Layouts.ColumnLayout {
            Kirigami.FormData.label: i18nd("kcm_touchpad", "Acceleration profile:")
            Kirigami.FormData.buddyFor: accelProfileFlat
            id: accelProfile
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

            Controls.RadioButton {
                id: accelProfileFlat
                text: i18nd("kcm_touchpad", "Flat")

                hoverEnabled: true
                Controls.ToolTip {
                    text: i18nd("kcm_touchpad", "Cursor moves the same distance as finger.")
                    visible: parent.hovered
                    delay: 1000
                }
                onCheckedChanged: accelProfile.syncCurrent()
            }

            Controls.RadioButton {
                id: accelProfileAdaptive
                text: i18nd("kcm_touchpad", "Adaptive")

                hoverEnabled: true
                Controls.ToolTip {
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
        Controls.CheckBox {
            Kirigami.FormData.label: i18nd("kcm_touchpad", "Tapping:")
            id: tapToClick
            text: i18nd("kcm_touchpad", "Tap-to-click")

            hoverEnabled: true
            Controls.ToolTip {
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

        Controls.CheckBox {
            id: tapAndDrag
            text: i18nd("kcm_touchpad", "Tap-and-drag")

            hoverEnabled: true
            Controls.ToolTip {
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

        Controls.CheckBox {
            id: tapAndDragLock
            text: i18nd("kcm_touchpad", "Tap-and-drag lock")

            hoverEnabled: true
            Controls.ToolTip {
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

        Layouts.ColumnLayout {
            Kirigami.FormData.label: i18nd("kcm_touchpad", "Two-finger tap:")
            Kirigami.FormData.buddyFor: multiTapRightClick
            id: multiTap

            spacing: Kirigami.Units.smallSpacing

            function load() {
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

                if(touchpad.lmrTapButtonMap) {
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

            Controls.RadioButton {
                id: multiTapRightClick
                // text: is handled dynamically on load.

                hoverEnabled: true
                Controls.ToolTip {
                    id: multiTapRightClickToolTip
                    visible: parent.hovered
                    delay: 1000
                    // text: is handled dynamically on load.
                }
                onCheckedChanged: multiTap.syncCurrent()
            }

            Controls.RadioButton {
                id: multiTapMiddleClick
                // text: is handled dynamically on load.

                hoverEnabled: true
                Controls.ToolTip {
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
        Layouts.ColumnLayout {
            Kirigami.FormData.label: i18nd("kcm_touchpad", "Scrolling:")
            Kirigami.FormData.buddyFor: scrollMethodTwoFingers
            id: scrollMethod

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

            Controls.RadioButton {
                id: scrollMethodTwoFingers
                text: i18nd("kcm_touchpad", "Two fingers")

                hoverEnabled: true
                Controls.ToolTip {
                    text: i18nd("kcm_touchpad", "Slide with two fingers scrolls.")
                    visible: parent.hovered
                    delay: 1000
                }
            }

            Controls.RadioButton {
                id: scrollMethodTouchpadEdges
                text: i18nd("kcm_touchpad", "Touchpad edges")

                hoverEnabled: true
                Controls.ToolTip {
                    text: i18nd("kcm_touchpad", "Slide on the touchpad edges scrolls.")
                    visible: parent.hovered
                    delay: 1000
                }
                onCheckedChanged: scrollMethod.syncCurrent()
            }
        }

        Controls.CheckBox {
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
            Controls.ToolTip {
                text: i18nd("kcm_touchpad", "Touchscreen like scrolling.")
                visible: parent.hovered
                delay: 1000
            }
        }

        Controls.CheckBox {
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
            Controls.ToolTip {
                text: i18nd("kcm_touchpad", "Disable horizontal scrolling")
                visible: parent.hovered
                delay: 1000
            }
        }

        // Scroll Speed aka scroll Factor
        Layouts.GridLayout {
            Kirigami.FormData.label: i18nd("kcm_touchpad", "Scrolling speed:")
            Kirigami.FormData.buddyFor: scrollFactor
            visible: touchpad.supportsScrollFactor

            columns: 3

            Controls.Slider {
                id: scrollFactor

                from: 0
                to: 14
                stepSize: 1

                property variant values : [
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

                Layouts.Layout.columnSpan: 3

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
            Controls.Label {
                text: i18nc("Slower Scroll", "Slower")
            }
            Item {
                Layouts.Layout.fillWidth: true
            }
            Controls.Label {
                text: i18nc("Faster Scroll Speed", "Faster")
            }
        }

        Item {
            Kirigami.FormData.isSection: false
        }

        Layouts.ColumnLayout {
            Kirigami.FormData.label: i18nd("kcm_touchpad", "Right-click:")
            Kirigami.FormData.buddyFor: rightClickMethodAreas
            id: rightClickMethod
            enabled: touchpad.supportsClickMethodAreas && touchpad.supportsClickMethodClickfinger
            
            spacing: Kirigami.Units.smallSpacing

            function load() {
                visible = (touchpad.supportedButtons & Qt.LeftButton)

                if (!visible) {
                    rightClickMethodAreas.checked = false
                    rightClickMethodClickfinger.checked = false
                    return;
                }

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
                    touchpad.clickMethodAreas = rightClickMethodAreas.checked
                    touchpad.clickMethodClickfinger = rightClickMethodClickfinger.checked
                    root.changeSignal()
                }
                loading = true
                middleClickMethod.load()
                loading = false
            }

            Controls.RadioButton {
                id: rightClickMethodAreas
                text: i18nd("kcm_touchpad", "Press bottom-right corner")

                hoverEnabled: true
                Controls.ToolTip {
                    text: i18nd("kcm_touchpad", "Software enabled buttons will be added to bottom portion of your touchpad.")
                    visible: parent.hovered
                    delay: 1000
                }
            }

            Controls.RadioButton {
                id: rightClickMethodClickfinger
                text: i18nd("kcm_touchpad", "Press anywhere with two fingers")

                hoverEnabled: true
                Controls.ToolTip {
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

        Layouts.ColumnLayout {
            Kirigami.FormData.label: i18nd("kcm_touchpad", "Middle-click: ")
            Kirigami.FormData.buddyFor: middleSoftwareEmulation
            id: middleClickMethod
            
            spacing: Kirigami.Units.smallSpacing

            function load() {
                visible = rightClickMethod.visible

                if (!visible) {
                    enabled = false
                    return;
                }

                enabled = touchpad.supportsMiddleEmulation
                if (enabled && touchpad.middleEmulation) {
                    middleSoftwareEmulation.checked = true
                } else {
                    noMiddleSoftwareEmulation.checked = true
                }
            }

            function syncCurrent() {
                if (enabled && !root.loading) {
                    touchpad.middleEmulation = middleSoftwareEmulation.checked
                    root.changeSignal()
                }
                loading = true
                middleEmulation.load()
                loading = false
            }

            Controls.RadioButton {
                id: noMiddleSoftwareEmulation
                text: i18nd("kcm_touchpad", "Press bottom-middle")
                visible: rightClickMethodAreas.checked
                hoverEnabled: true
                Controls.ToolTip {
                    text: i18nd("kcm_touchpad", "Software enabled middle-button will be added to bottom portion of your touchpad.")
                    visible: parent.hovered
                    delay: 1000
                }
            }

            Controls.RadioButton {
                id: middleSoftwareEmulation
                text: i18nd("kcm_touchpad", "Press bottom left and bottom right corners simultaneously")
                visible: rightClickMethodAreas.checked
                hoverEnabled: true
                Controls.ToolTip {
                    text: i18nd("kcm_touchpad", "Clicking left and right button simultaneously sends middle button click.")
                    visible: parent.hovered
                    delay: 1000
                }
                onCheckedChanged: middleClickMethod.syncCurrent()
            }

            Controls.CheckBox {
                id: clickfingerMiddleInfoBox
                text: i18nd("kcm_touchpad", "Press anywhere with three fingers")
                checked: true
                enabled: false
                visible: rightClickMethodClickfinger.checked
                hoverEnabled: true
                Controls.ToolTip {
                    text: i18nd("kcm_touchpad", "Press anywhere with three fingers.")
                    visible: parent.hovered
                    delay: 1000
                }
            }
        }
    } // END Kirigami.FormLayout
} // END Kirigami.ScrollablePage
