/*
    SPDX-FileCopyrightText: 2018 Roman Gilg <subdiff@gmail.com>
    SPDX-FileCopyrightText: 2018 Furkan Tokac <furkantokac34@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.7
import QtQuick.Controls 2.0 as Controls
import QtQuick.Layouts 1.3 as Layouts

import org.kde.kcm 1.1 as KCM
import org.kde.kirigami 2.4 as Kirigami

import "components"

// TODO: Change ScrollablePage as KCM.SimpleKCM
// after rewrite the KCM in KConfigModule.
Kirigami.ScrollablePage {
    id: root
    
    spacing: Kirigami.Units.smallSpacing
    
    property size sizeHint: Qt.size(formLayout.width, Math.round(1.3 * formLayout.height))
    property size minimumSizeHint: Qt.size(formLayout.width/2, Math.round(1.3 * formLayout.height))

    property alias deviceIndex: deviceSelector.currentIndex
    signal changeSignal()

    property QtObject device
    property int deviceCount: backend.deviceCount

    property bool loading: false

    enabled: deviceCount > 0

    function resetModel(index) {
        deviceCount = backend.deviceCount
        formLayout.enabled = deviceCount
        deviceSelector.enabled = deviceCount > 1

        loading = true
        if (deviceCount) {
            device = deviceModel[index]
            deviceSelector.model = deviceModel
            deviceSelector.currentIndex = index
            console.log("Configuration of device '" +
                        (index + 1) + " : " + device.name + "' opened")
        } else {
            deviceSelector.model = [""]
            console.log("No device found")
        }
        loading = false
    }

    function syncValuesFromBackend() {
        loading = true

        deviceEnabled.load()
        leftHanded.load()
        middleEmulation.load()
        accelSpeed.load()
        accelProfile.load()
        naturalScroll.load()
        scrollFactor.load()

        loading = false
    }

    Kirigami.FormLayout {
        id: formLayout
        enabled: deviceCount

        // Device
        Controls.ComboBox {
            Kirigami.FormData.label: i18nd("kcmmouse", "Device:")
            id: deviceSelector
            enabled: deviceCount > 1
            Layouts.Layout.fillWidth: true
            model: deviceModel
            textRole: "name"

            onCurrentIndexChanged: {
                if (deviceCount) {
                    device = deviceModel[currentIndex]
                    if (!loading) {
                        changeSignal()
                    }
                    console.log("Configuration of device '" +
                                (currentIndex+1) + " : " + device.name + "' opened")
                }
                root.syncValuesFromBackend()
            }
        }

        Item {
            Kirigami.FormData.isSection: false
        }

        // General
        Controls.CheckBox {
            Kirigami.FormData.label: i18nd("kcmmouse", "General:")
            id: deviceEnabled
            text: i18nd("kcmmouse", "Device enabled")

            function load() {
                if (!formLayout.enabled) {
                    checked = false
                    return
                }
                enabled = device.supportsDisableEvents
                checked = enabled && device.enabled
            }

            onCheckedChanged: {
                if (enabled && !root.loading) {
                    device.enabled = checked
                    root.changeSignal()
                }
            }

            ToolTip {
                text: i18nd("kcmmouse", "Accept input through this device.")
            }
        }

        Controls.CheckBox {
            id: leftHanded
            text: i18nd("kcmmouse", "Left handed mode")

            function load() {
                if (!formLayout.enabled) {
                    checked = false
                    return
                }
                enabled = device.supportsLeftHanded
                checked = enabled && device.leftHanded
            }

            onCheckedChanged: {
                if (enabled && !root.loading) {
                    device.leftHanded = checked
                    root.changeSignal()
                }
            }

            ToolTip {
                text: i18nd("kcmmouse", "Swap left and right buttons.")
            }
        }

        Controls.CheckBox {
            id: middleEmulation
            text: i18nd("kcmmouse", "Press left and right buttons for middle-click")

            function load() {
                if (!formLayout.enabled) {
                    checked = false
                    return
                }
                enabled = device.supportsMiddleEmulation
                checked = enabled && device.middleEmulation
            }

            onCheckedChanged: {
                if (enabled && !root.loading) {
                    device.middleEmulation = checked
                    root.changeSignal()
                }
            }

            ToolTip {
                text: i18nd("kcmmouse", "Clicking left and right button simultaneously sends middle button click.")
            }
        }

        Item {
            Kirigami.FormData.isSection: false
        }

        // Acceleration
        Controls.Slider {
            Kirigami.FormData.label: i18nd("kcmmouse", "Pointer speed:")
            id: accelSpeed

            from: 1
            to: 11
            stepSize: 1

            function load() {
                enabled = device.supportsPointerAcceleration
                if (!enabled) {
                    value = 0.2
                    return
                }
                // transform libinput's pointer acceleration range [-1, 1] to slider range [1, 11]
                //value = 4.5 * device.pointerAcceleration + 5.5
                value = 6 + device.pointerAcceleration / 0.2
            }

            onValueChanged: {
                if (device != undefined && enabled && !root.loading) {
                    // transform slider range [1, 10] to libinput's pointer acceleration range [-1, 1]
                    // by *10 and /10, we ignore the floating points after 1 digit. This prevents from
                    // having a libinput value like 0.60000001
                    device.pointerAcceleration = Math.round(((value-6) * 0.2) * 10) / 10
                    root.changeSignal()
                }
            }
        }

        Layouts.ColumnLayout {
            id: accelProfile
            spacing: Kirigami.Units.smallSpacing
            Kirigami.FormData.label: i18nd("kcmmouse", "Acceleration profile:")
            Kirigami.FormData.buddyFor: accelProfileFlat

            function load() {
                enabled = device.supportsPointerAccelerationProfileAdaptive

                if (!enabled) {
                    accelProfileAdaptive.checked = false
                    accelProfileFlat.checked = false
                    return
                }

                if(device.pointerAccelerationProfileAdaptive) {
                    accelProfileAdaptive.checked = true
                    accelProfileFlat.checked = false
                } else {
                    accelProfileAdaptive.checked = false
                    accelProfileFlat.checked = true
                }
            }

            function syncCurrent() {
                if (enabled && !root.loading) {
                    device.pointerAccelerationProfileFlat = accelProfileFlat.checked
                    device.pointerAccelerationProfileAdaptive = accelProfileAdaptive.checked
                    root.changeSignal()
                }
            }

            Controls.RadioButton {
                id: accelProfileFlat
                text: i18nd("kcmmouse", "Flat")

                ToolTip {
                    text: i18nd("kcmmouse", "Cursor moves the same distance as the mouse movement.")
                }
                onCheckedChanged: accelProfile.syncCurrent()
            }

            Controls.RadioButton {
                id: accelProfileAdaptive
                text: i18nd("kcmmouse", "Adaptive")

                ToolTip {
                    text: i18nd("kcmmouse", "Cursor travel distance depends on the mouse movement speed.")
                }
                onCheckedChanged: accelProfile.syncCurrent()
            }
        }

        Item {
            Kirigami.FormData.isSection: false
        }

        // Scrolling
        Controls.CheckBox {
            Kirigami.FormData.label: i18nd("kcmmouse", "Scrolling:")
            id: naturalScroll
            text: i18nd("kcmmouse", "Invert scroll direction")

            function load() {
                enabled = device.supportsNaturalScroll
                checked = enabled && device.naturalScroll
            }

            onCheckedChanged: {
                if (enabled && !root.loading) {
                    device.naturalScroll = checked
                    root.changeSignal()
                }
            }

            ToolTip {
                text: i18nd("kcmmouse", "Touchscreen like scrolling.")
            }
        }

        // Scroll Speed aka scroll Factor
        Layouts.GridLayout {
            Kirigami.FormData.label: i18nd("kcm_touchpad", "Scrolling speed:")
            Kirigami.FormData.buddyFor: scrollFactor

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
                    let index = values.indexOf(device.scrollFactor)
                    if (index === -1) {
                        index = values.indexOf(1);
                    }
                    value = index
                }

                onMoved: {
                    device.scrollFactor = values[value]
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
    }
}
