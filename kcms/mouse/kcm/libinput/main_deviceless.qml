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
    property size minimumSizeHint: Qt.size(formLayout.width, Math.round(1.3 * formLayout.height))
    
    signal changeSignal()

    property QtObject device: deviceModel[0]

    property bool loading: false

    function resetModel(index) {
        // not implemented
    }

    function syncValuesFromBackend() {
        loading = true

        leftHanded.load()
        middleEmulation.load()
        accelSpeed.load()
        accelProfile.load()
        naturalScroll.load()

        loading = false
    }

    Kirigami.FormLayout {
        id: formLayout

        // General
        Controls.CheckBox {
            Kirigami.FormData.label: i18nd("kcmmouse", "General:")
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
    }
}
