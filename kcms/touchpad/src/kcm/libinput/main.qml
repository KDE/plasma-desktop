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
import QtQuick.Controls 1.4 as Controls
import QtQuick.Layouts 1.3 as Layouts
import QtQuick.Controls.Styles 1.4 as Styles

import org.kde.kcm 1.1 as KCM
import org.kde.kirigami 2.4 as Kirigami
import org.kde.plasma.core 2.0 as PlasmaCore

import "components"

Kirigami.Page {
    id: root

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

        scrollmethod.load()
        naturalScroll.load()
//        TODO:
//        scrollbutton.load()

        loading = false
    }

    Kirigami.FormLayout {
        id: formLayout

        topPadding: root.topPadding
        leftPadding: root.leftPadding
        rightPadding: root.rightPadding
        bottomPadding: root.bottomPadding
        anchors {
            fill: parent
            topMargin: root.header ? root.header.height : 0
            bottomMargin: root.footer ? root.footer.height : 0
        }

        // Device
        Controls.ComboBox {
            Kirigami.FormData.label: i18n("Device:")
            id: deviceSelector
            enabled: touchpadCount > 1
            implicitWidth: units.gridUnit * 16
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

            ToolTip {
                text: i18n("Accept input through this device.")
            }
        }

        Controls.CheckBox {
            id: dwt
            text: i18n("Disable while typing")

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

            ToolTip {
                text: i18n("Disable touchpad while typing to prevent accidental inputs.")
            }
        }

        Controls.CheckBox {
            id: leftHanded
            text: i18n("Left handed mode")

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

            ToolTip {
                text: i18n("Swap left and right buttons.")
            }
        }

        Controls.CheckBox {
            id: middleEmulation
            text: i18n("Press left and right buttons for middle click")

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

            ToolTip {
                text: i18n("Clicking left and right button simultaneously sends middle button click.")
            }
        }

        Kirigami.Separator {
        }

        // Acceleration
        Controls.Slider {
            Kirigami.FormData.label: i18n("Pointer speed:")
            id: accelSpeed

            tickmarksEnabled: true

            minimumValue: 1
            maximumValue: 10
            stepSize: 1

            implicitWidth: units.gridUnit * 9

            function load() {
                enabled = touchpad.supportsPointerAcceleration
                if (!enabled) {
                    value = 0.1
                    return
                }
                // transform libinput's pointer acceleration range [-1, 1] to slider range [1, 10]
                value = 4.5 * touchpad.pointerAcceleration + 5.5
            }

            onValueChanged: {
                if (touchpad != undefined && enabled && !root.loading) {
                    // transform slider range [1, 10] to libinput's pointer acceleration range [-1, 1]
                    touchpad.pointerAcceleration = Math.round( (value - 5.5) / 4.5 * 100 ) / 100
                    root.changeSignal()
                }
            }
        }

        ExclGroupBox {
            Kirigami.FormData.label: i18n("Acceleration profile:")
            id: accelProfile
            model: [i18n("Flat"), i18n("Adaptive")]

            function load() {
                enabled = touchpad.supportsPointerAccelerationProfileAdaptive

                if (!enabled) {
                    itemAt(0).checked = false
                    itemAt(1).checked = false
                    return
                }

                itemAt(0).tooltiptext = i18n("Cursor moves the same distance as finger.")
                itemAt(1).tooltiptext = i18n("Cursor travel distance depends on movement speed of finger.")

                var toCheck = touchpad.pointerAccelerationProfileAdaptive ? 1 : 0
                itemAt(toCheck).checked = true
            }

            onCurrentChanged: {
                if (enabled && !root.loading) {
                    touchpad.pointerAccelerationProfileFlat = itemAt(0).checked
                    touchpad.pointerAccelerationProfileAdaptive = itemAt(1).checked
                    root.changeSignal()
                }
            }
        }

        Kirigami.Separator {
        }

        // Tapping
        Controls.CheckBox {
            Kirigami.FormData.label: i18n("Tapping:")
            id: tapToClick
            text: i18n("Tap-to-click")

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

            ToolTip {
                text: i18n("Single tap is left button click.")
            }
        }

        Controls.CheckBox {
            id: tapAndDrag
            text: i18n("Tap-and-drag")

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

            ToolTip {
                text: i18n("Sliding over touchpad directly after tap drags.")
            }
        }

        Controls.CheckBox {
            id: tapAndDragLock
            text: i18n("Tap-and-drag lock")

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

            ToolTip {
                text: i18n("Dragging continues after a short finger lift.")
            }
        }

        ExclGroupBox {
            Kirigami.FormData.label: i18n("Two-finger tap:")
            id: multiTap

            function load() {
                enabled = touchpad.supportsLmrTapButtonMap && tapToClick.checked
                if (touchpad.tapFingerCount > 2) {
                    model = [i18n("Right-click (three-finger tap to middle-click)"),
                             i18n("Middle-click (three-finger tap right-click)")]
                    itemAt(0).tooltiptext = i18n("Tap with two fingers to right-click, tap with three fingers to middle-click.")
                    itemAt(1).tooltiptext = i18n("Tap with two fingers to middle-click, tap with three fingers to right-click.")
                } else {
                    model = [i18n("Righ-click"), i18n("Middle-click")]
                    itemAt(0).tooltiptext = i18n("Tap with two fingers to right-click.")
                    itemAt(1).tooltiptext = i18n("Tap with two fingers to middle-click.")
                }

                if (!enabled) {
                    itemAt(0).checked = false
                    itemAt(1).checked = false
                    return
                }
                var toCheck = touchpad.lmrTapButtonMap ? 1 : 0
                itemAt(toCheck).checked = true
            }

            onCurrentChanged: {
                if (enabled && !root.loading) {
                    touchpad.lmrTapButtonMap = itemAt(1).checked
                    root.changeSignal()
                }
            }
        }

        Kirigami.Separator {
        }

        // Scrolling
        ExclGroupBox {
            Kirigami.FormData.label: i18n("Scrolling:")
            id: scrollmethod
            model: [i18n("Two fingers"), i18n("Touchpad edges")]

            function load() {
                itemAt(0).enabled = touchpad.supportsScrollTwoFinger
                itemAt(1).enabled = touchpad.supportsScrollEdge

                var toCheck = 0
                if (itemAt(0).enabled && touchpad.scrollTwoFinger) {
                    toCheck = 0
                } else if (itemAt(1).enabled && touchpad.scrollEdge) {
                    toCheck = 1
                }
                itemAt(0).tooltiptext = i18n("Slide with two fingers scrolls.")
                itemAt(1).tooltiptext = i18n("Slide on the touchpad edges scrolls.")

                itemAt(toCheck).checked = formLayout.enabled
            }

            onCurrentChanged: {
                if (enabled && !root.loading) {
                    touchpad.scrollTwoFinger = itemAt(0).checked
                    touchpad.scrollEdge = itemAt(1).checked
                    root.changeSignal()
                }
                loading = true
                naturalScroll.load()
                loading = false
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

            ToolTip {
                text: i18n("Touchscreen like scrolling.")
            }
        }
    } // END Kirigami.FormLayout
} // END Kirigami.ScrollablePage
