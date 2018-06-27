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
        accelSpeed.load()
        accelProfile.load()
        tapToClick.load()
        tapAndDrag.load()
        tapAndDragLock.load()
        multiTap.load()
        scrollMethod.load()
        naturalScroll.load()

        loading = false
    }

    Kirigami.FormLayout {
        id: formLayout

        // Device
        Controls.ComboBox {
            Kirigami.FormData.label: i18n("Device:")
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

        Kirigami.Separator {
        }

        // General settings
        Controls.CheckBox {
            Kirigami.FormData.label: i18n("General:")
            id: deviceEnabled
            text: i18n("Device enabled")

            hoverEnabled: true
            Controls.ToolTip {
                text: i18n("Accept input through this device.")
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
            text: i18n("Disable while typing")

            hoverEnabled: true
            Controls.ToolTip {
                text: i18n("Disable touchpad while typing to prevent accidental inputs.")
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
            text: i18n("Left handed mode")

            hoverEnabled: true
            Controls.ToolTip {
                text: i18n("Swap left and right buttons.")
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
            text: i18n("Press left and right buttons for middle click")

            hoverEnabled: true
            Controls.ToolTip {
                text: i18n("Clicking left and right button simultaneously sends middle button click.")
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
            }
        }

        Kirigami.Separator {
        }

        // Acceleration
        Controls.Slider {
            Kirigami.FormData.label: i18n("Pointer speed:")
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
            Kirigami.FormData.label: i18n("Acceleration profile:")
            Kirigami.FormData.buddyFor: accelProfileFlat
            id: accelProfile
            spacing: Kirigami.Units.smallSpacing

            function load() {
                enabled = touchpad.supportsPointerAccelerationProfileAdaptive

                if (!enabled) {
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
                text: i18n("Flat")

                hoverEnabled: true
                Controls.ToolTip {
                    text: i18n("Cursor moves the same distance as finger.")
                    visible: parent.hovered
                    delay: 1000
                }
                onCheckedChanged: accelProfile.syncCurrent()
            }

            Controls.RadioButton {
                id: accelProfileAdaptive
                text: i18n("Adaptive")

                hoverEnabled: true
                Controls.ToolTip {
                    text: i18n("Cursor travel distance depends on movement speed of finger.")
                    visible: parent.hovered
                    delay: 1000
                }
                onCheckedChanged: accelProfile.syncCurrent()
            }
        }

        Kirigami.Separator {
        }

        // Tapping
        Controls.CheckBox {
            Kirigami.FormData.label: i18n("Tapping:")
            id: tapToClick
            text: i18n("Tap-to-click")

            hoverEnabled: true
            Controls.ToolTip {
                text: i18n("Single tap is left button click.")
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
            text: i18n("Tap-and-drag")

            hoverEnabled: true
            Controls.ToolTip {
                text: i18n("Sliding over touchpad directly after tap drags.")
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
            text: i18n("Tap-and-drag lock")

            hoverEnabled: true
            Controls.ToolTip {
                text: i18n("Dragging continues after a short finger lift.")
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
            Kirigami.FormData.label: i18n("Two-finger tap:")
            Kirigami.FormData.buddyFor: multiTapRightClick
            id: multiTap

            spacing: Kirigami.Units.smallSpacing

            function load() {
                enabled = touchpad.supportsLmrTapButtonMap && tapToClick.checked
                if (touchpad.tapFingerCount > 2) {
                    multiTapRightClick.text = i18n("Right-click (three-finger tap to middle-click)")
                    multiTapRightClickToolTip.text = i18n("Tap with two fingers to right-click, tap with three fingers to middle-click.")

                    multiTapMiddleClick.text = i18n("Middle-click (three-finger tap right-click)")
                    multiTapMiddleClickToolTip.text = i18n("Tap with two fingers to middle-click, tap with three fingers to right-click.")
                } else {
                    multiTapRightClick.text = i18n("Righ-click")
                    multiTapRightClickToolTip.text = i18n("Tap with two fingers to right-click.")

                    multiTapMiddleClick.text = i18n("Middle-click")
                    multiTapMiddleClickToolTip.text = i18n("Tap with two fingers to middle-click.")
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

        Kirigami.Separator {
        }

        // Scrolling
        Layouts.ColumnLayout {
            Kirigami.FormData.label: i18n("Scrolling:")
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
                text: i18n("Two fingers")

                hoverEnabled: true
                Controls.ToolTip {
                    text: i18n("Slide with two fingers scrolls.")
                    visible: parent.hovered
                    delay: 1000
                }
            }

            Controls.RadioButton {
                id: scrollMethodTouchpadEdges
                text: i18n("Touchpad edges")

                hoverEnabled: true
                Controls.ToolTip {
                    text: i18n("Slide on the touchpad edges scrolls.")
                    visible: parent.hovered
                    delay: 1000
                }
                onCheckedChanged: scrollMethod.syncCurrent()
            }
        }

        Controls.CheckBox {
            id: naturalScroll
            text: i18n("Invert scroll direction (Natural scrolling)")

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
                text: i18n("Touchscreen like scrolling.")
                visible: parent.hovered
                delay: 1000
            }
        }
    } // END Kirigami.FormLayout
} // END Kirigami.ScrollablePage
