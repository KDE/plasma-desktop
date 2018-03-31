/*
 * Copyright 2018 Roman Gilg <subdiff@gmail.com>
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

    property size sizeHint: Qt.size(maincol.width, Math.round(1.1 * maincol.height))
    property size minimumSizeHint: Qt.size(maincol.width, Math.round(1.1 * maincol.height))
    signal changeSignal()

    property QtObject device: deviceModel[0]

    property bool loading: false

    function resetModel(index) {
        // not implemented
    }

    function syncValuesFromBackend() {
        loading = true

        leftHanded.load()
        accelSpeed.load()
        accelProfile.load()

        naturalScroll.load()

        loading = false
    }

    Controls.ScrollView {
        anchors.fill: parent

        Layouts.ColumnLayout {
            id: maincol
            spacing: units.largeSpacing

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
                                    id: leftHanded
                                    text: i18n("Left handed mode")

                                    function load() {
                                        if (!maincol.enabled) {
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
                                            enabled = device.supportsPointerAcceleration
                                            if (!enabled) {
                                                value = 0.1
                                                return
                                            }
                                            // transform libinput's pointer acceleration range [-1, 1] to slider range [1, 10]
                                            value = 4.5 * device.pointerAcceleration + 5.5
                                        }

                                        onValueChanged: {
                                            if (device != undefined && enabled && !root.loading) {
                                                // transform slider range [1, 10] to libinput's pointer acceleration range [-1, 1]
                                                device.pointerAcceleration = Math.round( (value - 5.5) / 4.5 * 100 ) / 100
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
                                        enabled = device.supportsPointerAccelerationProfileAdaptive

                                        if (!enabled) {
                                            itemAt(0).checked = false
                                            itemAt(1).checked = false
                                            return
                                        }

                                        itemAt(0).tooltiptext = i18n("Cursor moves the same distance as finger.")
                                        itemAt(1).tooltiptext = i18n("Cursor travel distance depends on movement speed of finger.")

                                        var toCheck = device.pointerAccelerationProfileAdaptive ? 1 : 0
                                        itemAt(toCheck).checked = true
                                    }

                                    onCurrentChanged: {
                                        if (enabled && !root.loading) {
                                            device.pointerAccelerationProfileFlat = itemAt(0).checked
                                            device.pointerAccelerationProfileAdaptive = itemAt(1).checked
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
                            text: i18n("Scrolling:")
                        }

                        Column {
                            leftPadding: units.smallSpacing
                            Column {
                                spacing: units.smallSpacing

                                Controls.CheckBox {
                                    id: naturalScroll
                                    text: i18n("Invert scroll direction")

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
