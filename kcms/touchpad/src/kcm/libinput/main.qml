/*
 * Copyright 2017 Roman Gilg <subdiff@gmail.com>
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

import org.kde.plasma.core 2.0 as PlasmaCore

import "components"

Item {
    id: root

    property size sizeHint: Qt.size(maincol.width, maincol.height)
    property size minimumSizeHint: Qt.size(maincol.width/2, deviceSelector.height)
    property alias deviceIndex: deviceSelector.currentIndex
    signal changeSignal()

    property QtObject touchpad
    property int touchpadCount: backend.touchpadCount

    property bool loading: false

    function resetModel(index) {
        touchpadCount = backend.touchpadCount
        maincol.enabled = touchpadCount
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

    Controls.ScrollView {
        anchors.fill: parent

        Layouts.ColumnLayout {
            id: maincol
            enabled: touchpadCount
            spacing: units.largeSpacing

            Layouts.RowLayout {
                spacing: units.largeSpacing
                Layouts.Layout.leftMargin: 0.1 * parent.width
                Layouts.Layout.rightMargin: 0.1 * parent.width

                Controls.Label {
                    text: i18n("Device:")
                }

                Controls.ComboBox {
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
            }

            Row {
                spacing: units.largeSpacing * 2

                Column {
                    spacing: units.smallSpacing * 2

                    Column {
                        leftPadding: units.smallSpacing
                        Column {
                            spacing: units.smallSpacing
                            Controls.Label {
                                text: i18n("General settings:")
                            }

                            Column {
                                leftPadding: units.smallSpacing
                                Column {
                                    spacing: units.smallSpacing
                                    Controls.CheckBox {
                                        id: deviceEnabled
                                        text: i18n("Device enabled")

                                        function load() {
                                            if (!maincol.enabled) {
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
                                            if (!maincol.enabled) {
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
                                            if (!maincol.enabled) {
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
                                        text: i18n("Emulate middle button")

                                        function load() {
                                            if (!maincol.enabled) {
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
                                }
                            }
                        }
                    }

                    Column {
                        leftPadding: units.smallSpacing
                        Column {
                            spacing: units.smallSpacing
                            Controls.Label {
                                text: i18n("Acceleration:")
                            }

                            Column {
                                leftPadding: units.smallSpacing
                                Column {
                                    spacing: units.smallSpacing * 2

                                    Row {
                                        Controls.Slider {
                                            id: accelSpeed
                                            anchors.verticalCenter: parent.verticalCenter

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
                                    }

                                    ExclGroupBox {
                                        id: accelProfile
                                        label: i18n("Acceleration Profile:")
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
                                }
                            }
                        }
                    }

                    Column {
                        leftPadding: units.smallSpacing
                        Column {
                            spacing: units.smallSpacing
                            Controls.Label {
                                text: i18n("Tapping:")
                            }

                            Column {
                                leftPadding: units.smallSpacing
                                Column {
                                    spacing: units.smallSpacing
                                    Controls.CheckBox {
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
                                }
                            }
                        }
                    }
                }

                Column {
                    spacing: units.smallSpacing * 2

                    Column {
                        leftPadding: units.smallSpacing
                    Column {
                        leftPadding: units.smallSpacing
                        ExclGroupBox {
                            id: multiTap
                            label: i18n("Multi tapping:")

                            function load() {
                                enabled = touchpad.supportsLmrTapButtonMap && tapToClick.checked
                                if (touchpad.tapFingerCount > 2) {
                                    model = [i18n("Two-tap right, three middle"), i18n("Two-tap middle, three right")]
                                    itemAt(0).tooltiptext = i18n("Tap with two fingers triggers a right click, tap with three fingers a middle click.")
                                    itemAt(1).tooltiptext = i18n("Tap with two fingers triggers a middle click, tap with three fingers a right click.")
                                } else {
                                    model = [i18n("Two-tap right click"), i18n("Two-tap middle click")]
                                    itemAt(0).tooltiptext = i18n("Tap with two fingers triggers a right click.")
                                    itemAt(1).tooltiptext = i18n("Tap with two fingers triggers a middle click.")
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
                    }
                    }

                    Column {
                        leftPadding: units.smallSpacing
                        Column {
                            spacing: units.smallSpacing
                            Controls.Label {
                                text: i18n("Scrolling:")
                            }

                            Column {
                                leftPadding: units.smallSpacing
                                Column {
                                    spacing: units.smallSpacing
                                    ExclGroupBox {
                                        id: scrollmethod
                                        label: i18n("Scroll method:")
                                        model: [i18n("Two fingers"), i18n("Touchpad edges"), /*i18n("On Button down"),*/ i18n("No scroll")]

                                        property bool isNoScroll: true

                                        function load() {
                                            itemAt(0).enabled = touchpad.supportsScrollTwoFinger
                                            itemAt(1).enabled = touchpad.supportsScrollEdge
        //                                    TODO:
        //                                    itemAt(2).enabled = touchpad.supportsScrollOnButtonDown

                                            var toCheck = 2
                                            if (itemAt(0).enabled && touchpad.scrollTwoFinger) {
                                                toCheck = 0
                                            } else if (itemAt(1).enabled && touchpad.scrollEdge) {
                                                toCheck = 1
        //                                    } else if (itemAt(2).enabled && touchpad.scrollOnButtonDown) {
        //                                        toCheck = 2
                                            }
                                            itemAt(0).tooltiptext = i18n("Slide with two fingers scrolls.")
                                            itemAt(1).tooltiptext = i18n("Slide on the touchpad edges scrolls.")
                                            itemAt(2).tooltiptext = i18n("All forms of touchpad scrolling are deactivated.")

                                            isNoScroll = (toCheck == 2)
                                            itemAt(toCheck).checked = maincol.enabled
                                        }

                                        onCurrentChanged: {
                                            if (enabled && !root.loading) {
                                                touchpad.scrollTwoFinger = itemAt(0).checked
                                                touchpad.scrollEdge = itemAt(1).checked
        //                                        touchpad.scrollOnButtonDown = itemAt(2).checked
                                                root.changeSignal()
                                            }
                                            isNoScroll = itemAt(2).checked
                                            loading = true
                                            naturalScroll.load()
                                            loading = false
                                        }
                                    }

                                    Controls.CheckBox {
                                        id: naturalScroll
                                        text: i18n("Invert scroll direction")

                                        function load() {
                                            enabled = touchpad.supportsNaturalScroll && !scrollmethod.isNoScroll
                                            checked = enabled && touchpad.naturalScroll
                                        }

                                        onCheckedChanged: {
                                            if (enabled && !root.loading) {
                                                touchpad.tapDragLock = checked
                                                root.changeSignal()
                                            }
                                        }

                                        ToolTip {
                                            text: i18n("Touchscreen like scrolling.")
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
