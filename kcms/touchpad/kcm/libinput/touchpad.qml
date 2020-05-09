/*
 * Copyright 2017 Roman Gilg <subdiff@gmail.com>
 * Copyright 2018 Furkan Tokac <furkantokac34@gmail.com>
 * Copyright 2020 Nate Graham <nate@kde.org>
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
import QtQuick.Controls 2.5 as QQC2
import QtQuick.Layouts 1.3

import org.kde.kcm 1.1 as KCM
import org.kde.kirigami 2.4 as Kirigami

// TODO: Change ScrollablePage as KCM.SimpleKCM
// after rewrite the KCM in KConfigModule.
Kirigami.ScrollablePage {
    id: root

    property size minimumSizeHint: Qt.size(formLayout.width/2, deviceSelector.height)

    property alias deviceIndex: deviceSelector.currentIndex

    signal changeSignal()

    property QtObject touchpad
    property int touchpadCount: backend.touchpadCount

    property bool loading: false

    enabled: touchpadCount > 0

    function resetModel(index) {
        touchpadCount = backend.touchpadCount

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
    }


    Kirigami.FormLayout {
        id: formLayout

        enabled: touchpadCount > 0

        // Device
        QQC2.ComboBox {
            id: deviceSelector

            Kirigami.FormData.label: i18nd("kcm_touchpad", "Device:")
            Layout.fillWidth: true

            enabled: touchpadCount > 1

            model: deviceModel
            textRole: "name"

            onAccepted: {
                touchpad = deviceModel[currentIndex]
                changeSignal()
            }
        }

        Item {
            Kirigami.FormData.isSection: false
        }

        // General settings
        QQC2.CheckBox {
            Kirigami.FormData.label: i18nd("kcm_touchpad", "General:")

            enabled: touchpad.supportsDisableEvents

            checked: touchpad.enabled
            onToggled: root.changeSignal()

            text: i18nd("kcm_touchpad", "Device enabled")

            hoverEnabled: true
            QQC2.ToolTip {
                text: i18nd("kcm_touchpad", "Accept input through this device.")
                visible: parent.hovered
                delay: 1000
            }
        }

        QQC2.CheckBox {
            enabled: touchpad.supportsDisableWhileTyping

            checked: touchpad.disableWhileTyping
            onToggled: root.changeSignal()

            text: i18nd("kcm_touchpad", "Disable while typing")

            hoverEnabled: true
            QQC2.ToolTip {
                text: i18nd("kcm_touchpad", "Disable touchpad while typing to prevent accidental inputs.")
                visible: parent.hovered
                delay: 1000
            }
        }

        QQC2.CheckBox {
            enabled: touchpad.supportsLeftHanded

            checked: touchpad.leftHanded
            onToggled: root.changeSignal()

            text: i18nd("kcm_touchpad", "Left handed mode")

            hoverEnabled: true
            QQC2.ToolTip {
                text: i18nd("kcm_touchpad", "Swap left and right buttons.")
                visible: parent.hovered
                delay: 1000
            }
        }

        QQC2.CheckBox {
            enabled: touchpad.supportsMiddleEmulation

            checked: touchpad.middleEmulation
            onToggled: root.changeSignal()

            text: i18nd("kcm_touchpad", "Press left and right buttons for middle click")

            hoverEnabled: true
            QQC2.ToolTip {
                text: i18nd("kcm_touchpad", "Clicking left and right button simultaneously sends middle button click.")
                visible: parent.hovered
                delay: 1000
            }
        }

        Item {
            Kirigami.FormData.isSection: false
        }

        // Acceleration
        QQC2.Slider {
            Kirigami.FormData.label: i18nd("kcm_touchpad", "Pointer speed:")

            enabled: touchpad.supportsPointerAcceleration

            from: 1
            to: 11

            value: 6 + touchpad.pointerAcceleration / 0.2
            onMoved: {
                // transform slider range [1, 11] to libinput's pointer acceleration range [-1, 1]
                // by *10 and /10, we ignore the floating points after 1 digit. This prevents from
                // having a libinput value like 0.60000001
                touchpad.pointerAcceleration = Math.round(((value-6) * 0.2) * 10) / 10
                root.changeSignal()
            }
        }

        ColumnLayout {
            Kirigami.FormData.label: i18nd("kcm_touchpad", "Acceleration profile:")
            Kirigami.FormData.buddyFor: accelProfileFlat

            enabled: touchpad.supportsPointerAccelerationProfileAdaptive

            QQC2.RadioButton {
                id: accelProfileFlat

                checked: touchpad.pointerAccelerationProfileFlat
                onToggled: root.changeSignal()

                text: i18nd("kcm_touchpad", "Flat")

                hoverEnabled: true
                QQC2.ToolTip {
                    text: i18nd("kcm_touchpad", "Cursor moves the same distance as finger.")
                    visible: parent.hovered
                    delay: 1000
                }
            }

            QQC2.RadioButton {
                id: accelProfileAdaptive

                checked: touchpad.pointerAccelerationProfileAdaptive
                onToggled: root.changeSignal()

                text: i18nd("kcm_touchpad", "Adaptive")

                hoverEnabled: true
                QQC2.ToolTip {
                    text: i18nd("kcm_touchpad", "Cursor travel distance depends on movement speed of finger.")
                    visible: parent.hovered
                    delay: 1000
                }
            }
        }

        Item {
            Kirigami.FormData.isSection: false
        }

        // Tapping
        QQC2.CheckBox {
            id: tapToClick

            Kirigami.FormData.label: i18nd("kcm_touchpad", "Tapping:")

            enabled: touchpad.tapFingerCount > 0

            checked: touchpad.tapToClick
            onToggled: root.changeSignal()

            text: i18nd("kcm_touchpad", "Tap-to-click")

            hoverEnabled: true
            QQC2.ToolTip {
                text: i18nd("kcm_touchpad", "Single tap is left button click.")
                visible: parent.hovered
                delay: 1000
            }
        }

        QQC2.CheckBox {
            id: tapAndDrag

            enabled: touchpad.tapFingerCount > 0 && tapToClick.checked

            checked: touchpad.tapAndDrag
            onToggled: root.changeSignal()

            text: i18nd("kcm_touchpad", "Tap-and-drag")

            hoverEnabled: true
            QQC2.ToolTip {
                text: i18nd("kcm_touchpad", "Sliding over touchpad directly after tap drags.")
                visible: parent.hovered
                delay: 1000
            }

        }

        QQC2.CheckBox {
            id: tapAndDragLock

            enabled: touchpad.tapFingerCount > 0 && tapAndDrag.checked

            checked: touchpad.tapDragLock
            onToggled: root.changeSignal()

            text: i18nd("kcm_touchpad", "Tap-and-drag lock")

            hoverEnabled: true
            QQC2.ToolTip {
                text: i18nd("kcm_touchpad", "Dragging continues after a short finger lift.")
                visible: parent.hovered
                delay: 1000
            }
        }

        ColumnLayout {
            id: multiTap

            Kirigami.FormData.label: i18nd("kcm_touchpad", "Two-finger tap:")
            Kirigami.FormData.buddyFor: multiTapRightClick

            enabled: touchpad.supportsLmrTapButtonMap && tapToClick.checked

            QQC2.RadioButton {
                id: multiTapRightClick

                checked: !touchpad.lmrTapButtonMap
                onToggled: root.changeSignal()

                text: touchpad.tapFingerCount > 2 ? i18nd("kcm_touchpad", "Right-click (three-finger tap to middle-click)") : i18nd("kcm_touchpad", "Right-click")

                hoverEnabled: true
                QQC2.ToolTip {
                    id: multiTapRightClickToolTip
                    text: touchpad.tapFingerCount > 2 ? i18nd("kcm_touchpad", "Tap with two fingers to right-click, tap with three fingers to middle-click.") : i18nd("kcm_touchpad", "Tap with two fingers to right-click.")
                    visible: parent.hovered
                    delay: 1000
                }
            }

            QQC2.RadioButton {
                id: multiTapMiddleClick

                checked: touchpad.lmrTapButtonMap
                onToggled: root.changeSignal()

                text: touchpad.tapFingerCount > 2 ? i18nd("kcm_touchpad", "Middle-click (three-finger tap right-click)") : i18nd("kcm_touchpad", "Middle-click")

                hoverEnabled: true
                QQC2.ToolTip {
                    id: multiTapMiddleClickToolTip
                    text: touchpad.tapFingerCount > 2 ? i18nd("kcm_touchpad", "Tap with two fingers to middle-click, tap with three fingers to right-click.") : i18nd("kcm_touchpad", "Tap with two fingers to middle-click.")
                    visible: parent.hovered
                    delay: 1000
                }
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

            QQC2.RadioButton {
                id: scrollMethodTwoFingers

                enabled: touchpad.supportsScrollTwoFinger

                checked: touchpad.scrollTwoFinger
                onToggled: root.changeSignal()

                text: i18nd("kcm_touchpad", "Two fingers")

                hoverEnabled: true
                QQC2.ToolTip {
                    text: i18nd("kcm_touchpad", "Slide with two fingers scrolls.")
                    visible: parent.hovered
                    delay: 1000
                }
            }

            QQC2.RadioButton {
                enabled: touchpad.supportsScrollEdge

                checked: touchpad.scrollEdge
                onToggled: root.changeSignal()

                text: i18nd("kcm_touchpad", "Touchpad edges")

                hoverEnabled: true
                QQC2.ToolTip {
                    text: i18nd("kcm_touchpad", "Slide on the touchpad edges scrolls.")
                    visible: parent.hovered
                    delay: 1000
                }
            }
        }

        QQC2.CheckBox {
            enabled: touchpad.supportsNaturalScroll

            checked: touchpad.naturalScroll
            onToggled: root.changeSignal()

            text: i18nd("kcm_touchpad", "Invert scroll direction (Natural scrolling)")

            hoverEnabled: true
            QQC2.ToolTip {
                text: i18nd("kcm_touchpad", "Touchscreen like scrolling.")
                visible: parent.hovered
                delay: 1000
            }
        }

        QQC2.CheckBox {
            enabled: touchpad.supportsHorizontalScrolling

            checked: !touchpad.horizontalScrolling
            onToggled: root.changeSignal()

            text: i18nd("kcm_touchpad", "Disable horizontal scrolling")

            hoverEnabled: true
            QQC2.ToolTip {
                text: i18nd("kcm_touchpad", "Disable horizontal scrolling")
                visible: parent.hovered
                delay: 1000
            }
        }

        // FIXME
        // Scroll Speed aka scroll Factor
        GridLayout {
            Kirigami.FormData.label: i18nd("kcm_touchpad", "Scrolling speed:")
            Kirigami.FormData.buddyFor: scrollFactor

            visible: touchpad.supportsScrollFactor

            columns: 3

            QQC2.Slider {
                id: scrollFactor

                from: 0
                to: 14

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
                text: i18nc("Slower Scroll", "Slower")
            }
            Item {
                Layout.fillWidth: true
            }
            QQC2.Label {
                text: i18nc("Faster Scroll Speed", "Faster")
            }
        }

        Item {
            Kirigami.FormData.isSection: false
        }

        ColumnLayout {
            id: rightClickMethod

            Kirigami.FormData.label: i18nd("kcm_touchpad", "Right-click:")
            Kirigami.FormData.buddyFor: rightClickMethodAreas

            visible: touchpad.supportedButtons & Qt.LeftButton
            enabled: touchpad.supportsClickMethodAreas && touchpad.supportsClickMethodClickfinger

            QQC2.RadioButton {
                id: rightClickMethodAreas

                enabled: touchpad.supportsClickMethodAreas

                checked: touchpad.clickMethodAreas
                onToggled: root.changeSignal()

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

                enabled: touchpad.supportsClickMethodClickfinger

                checked: touchpad.clickMethodClickfinger
                onToggled: root.changeSignal()

                text: i18nd("kcm_touchpad", "Press anywhere with two fingers")

                hoverEnabled: true
                QQC2.ToolTip {
                    text: i18nd("kcm_touchpad", "Tap with two finger to enable right click.")
                    visible: parent.hovered
                    delay: 1000
                }
            }
        }

        Item {
            Kirigami.FormData.isSection: false
        }

        ColumnLayout {
            Kirigami.FormData.label: i18nd("kcm_touchpad", "Middle-click: ")
            Kirigami.FormData.buddyFor: middleSoftwareEmulation

            enabled: touchpad.supportsMiddleEmulation

            QQC2.RadioButton {
                visible: rightClickMethodAreas.checked

                checked: !touchpad.middleEmulation
                onToggled: root.changeSignal()

                text: i18nd("kcm_touchpad", "Press bottom-middle")

                hoverEnabled: true
                QQC2.ToolTip {
                    text: i18nd("kcm_touchpad", "Software enabled middle-button will be added to bottom portion of your touchpad.")
                    visible: parent.hovered
                    delay: 1000
                }
            }

            QQC2.RadioButton {
                id: middleSoftwareEmulation

                visible: rightClickMethodAreas.checked

                checked: touchpad.middleEmulation
                onToggled: root.changeSignal()

                text: i18nd("kcm_touchpad", "Press bottom left and bottom right corners simultaneously")

                hoverEnabled: true
                QQC2.ToolTip {
                    text: i18nd("kcm_touchpad", "Clicking left and right button simultaneously sends middle button click.")
                    visible: parent.hovered
                    delay: 1000
                }
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
    } // END Kirigami.FormLayout
} // END Kirigami.ScrollablePage
