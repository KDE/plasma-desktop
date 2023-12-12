/*
    SPDX-FileCopyrightText: 2018 Roman Gilg <subdiff@gmail.com>
    SPDX-FileCopyrightText: 2018 Furkan Tokac <furkantokac34@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kquickcontrols

Kirigami.ApplicationItem {
    id: root

    pageStack.globalToolBar.style:  Kirigami.ApplicationHeaderStyle.None
    pageStack.columnView.columnResizeMode: Kirigami.ColumnView.SingleColumn
    pageStack.defaultColumnWidth: Kirigami.Units.gridUnit * 20

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
        } else {
            deviceSelector.model = [""]
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
        buttonMappings.load()

        loading = false
    }

    pageStack.initialPage: Kirigami.ScrollablePage {
        spacing: Kirigami.Units.smallSpacing

        Kirigami.FormLayout {
            id: formLayout
            enabled: deviceCount

            // Device
            QQC2.ComboBox {
                id: deviceSelector
                Kirigami.FormData.label: i18nd("kcmmouse", "Device:")
                enabled: deviceCount > 1
                Layout.fillWidth: true
                model: deviceModel
                textRole: "name"

                onCurrentIndexChanged: {
                    if (deviceCount) {
                        device = deviceModel[currentIndex]
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

            // General
            QQC2.CheckBox {
                id: deviceEnabled
                Kirigami.FormData.label: i18nd("kcmmouse", "General:")
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

                QQC2.ToolTip.delay: 1000
                QQC2.ToolTip.visible: hovered
                QQC2.ToolTip.text: i18nd("kcmmouse", "Accept input through this device.")
            }

            QQC2.CheckBox {
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

                QQC2.ToolTip.delay: 1000
                QQC2.ToolTip.visible: hovered
                QQC2.ToolTip.text: i18nd("kcmmouse", "Swap left and right buttons.")
            }

            QQC2.CheckBox {
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

                QQC2.ToolTip.delay: 1000
                QQC2.ToolTip.visible: hovered
                QQC2.ToolTip.text: i18nd("kcmmouse", "Clicking left and right button simultaneously sends middle button click.")
            }

            Item {
                Kirigami.FormData.isSection: false
            }

            // Acceleration
            QQC2.Slider {
                id: accelSpeed
                Kirigami.FormData.label: i18nd("kcmmouse", "Pointer speed:")
                Layout.fillWidth: true

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

            ColumnLayout {
                id: accelProfile
                spacing: Kirigami.Units.smallSpacing
                Kirigami.FormData.label: i18nd("kcmmouse", "Pointer acceleration:")
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

                QQC2.RadioButton {
                    id: accelProfileFlat
                    text: i18nd("kcmmouse", "None")

                    QQC2.ToolTip.delay: 1000
                    QQC2.ToolTip.visible: hovered
                    QQC2.ToolTip.text: i18nd("kcmmouse", "Cursor moves the same distance as the mouse movement.")
                    onCheckedChanged: accelProfile.syncCurrent()
                }

                QQC2.RadioButton {
                    id: accelProfileAdaptive
                    text: i18nd("kcmmouse", "Standard")

                    QQC2.ToolTip.delay: 1000
                    QQC2.ToolTip.visible: hovered
                    QQC2.ToolTip.text: i18nd("kcmmouse", "Cursor travel distance depends on the mouse movement speed.")
                    onCheckedChanged: accelProfile.syncCurrent()
                }
            }

            Item {
                Kirigami.FormData.isSection: false
            }

            // Scrolling
            QQC2.CheckBox {
                id: naturalScroll
                Kirigami.FormData.label: i18nd("kcmmouse", "Scrolling:")
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

                QQC2.ToolTip.delay: 1000
                QQC2.ToolTip.visible: hovered
                QQC2.ToolTip.text: i18nd("kcmmouse", "Touchscreen like scrolling.")
            }

            // Scroll Speed aka scroll Factor
            GridLayout {
                Kirigami.FormData.label: i18nd("kcm_touchpad", "Scrolling speed:")
                Kirigami.FormData.buddyFor: scrollFactor
                Layout.fillWidth: true

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
                QQC2.Label {
                    text: i18ndc("kcmmouse", "Slower Scroll", "Slower")
                }
                Item {
                    Layout.fillWidth: true
                }
                QQC2.Label {
                    text: i18ndc("kcmmouse", "Faster Scroll Speed", "Faster")
                }

            }

            Item {
                Kirigami.FormData.isSection: true
            }

            QQC2.Button  {
                text: i18ndc("kcmmouse", "@action:button", "Re-bind Additional Mouse Buttons…")
                visible: buttonMappings.model.length > 0 || Array.prototype.some.call(deviceModel, device => device.supportedButtons & ~(Qt.LeftButton | Qt.RightButton | Qt.MiddleButton))
                onClicked: root.pageStack.push(buttonPage)
            }
        }
    }

    Kirigami.ScrollablePage {
        id: buttonPage
        visible: false

        MouseArea {
            // Deliberately using MouseArea on the page instead of a TapHandler on the button, so we can capture clicks anywhere
            id: buttonCapture
            property var lastButton: {}

            anchors.fill: parent
            enabled: newBinding.checked
            preventStealing: true
            acceptedButtons: Qt.AllButtons & ~(Qt.LeftButton | Qt.RightButton | Qt.MiddleButton)
            onClicked: {
                lastButton = buttonMappings.extraButtons.find(entry => Qt[entry.buttonName] === mouse.button)
                newBinding.visible = false
                newKeySequenceItem.visible = true
                newKeySequenceItem.startCapturing()
            }
        }

       ColumnLayout {
            Kirigami.FormLayout {
                id: buttonLayout
                twinFormLayouts: otherLayout
                Repeater {
                    id: buttonMappings

                    readonly property var extraButtons: Array.from({length: 24}, (value, index) => ({
                        buttonName: "ExtraButton" + (index + 1),
                        button: Qt["ExtraButton" + (index + 1)],
                        label: i18ndc("kcmmouse", "@label for assigning an action to a numbered button", "Extra Button %1:", index + 1)
                    }))

                    function load() {
                        model = Qt.binding(() => extraButtons.filter(entry => backend.buttonMapping.hasOwnProperty(entry.buttonName)))
                    }

                    delegate: KeySequenceItem {
                        Kirigami.FormData.label: modelData.label

                        keySequence: backend.buttonMapping[modelData.buttonName]

                        modifierlessAllowed: true
                        multiKeyShortcutsAllowed: false
                        checkForConflictsAgainst: ShortcutType.None

                        onCaptureFinished: {
                            const copy = backend.buttonMapping;
                            copy[modelData.buttonName] = keySequence
                            backend.buttonMapping = copy
                            root.changeSignal()
                        }
                    }
                }
            }

            Kirigami.InlineMessage {
                id: explanationLabel
                Layout.fillWidth: true
                visible: newBinding.checked || newKeySequenceItem.visible
                text: newBinding.visible ? i18ndc("kcmmouse","@action:button", "Press the mouse button for which you want to add a key binding") :
                    i18ndc("kcmmouse","@action:button, %1 is the translation of 'Extra Button %1' from above", "Enter the new key combination for %1", buttonCapture.lastButton.label)
                actions: [
                    Kirigami.Action {
                        icon.name: "dialog-cancel"
                        text: i18ndc("kcmmouse", "@action:button", "Cancel")
                        onTriggered: {
                            newKeySequenceItem.visible = false;
                            newBinding.visible = true
                            newBinding.checked = false
                        }
                    }
                ]
            }

            Kirigami.FormLayout {
                id: otherLayout
                twinFormLayouts: buttonLayout

                QQC2.Button {
                    id: newBinding
                    checkable: true
                    text: checked ? i18ndc("kcmmouse", "@action:button", "Press a mouse button ") :
                        i18ndc("kcmmouse", "@action:button, Bind a mousebutton to keyboard key(s)", "Add Binding…")
                    icon.name: "list-add"
                }
                KeySequenceItem {
                    id: newKeySequenceItem
                    visible: false

                    modifierlessAllowed: true
                    multiKeyShortcutsAllowed: false
                    checkForConflictsAgainst: ShortcutType.None

                    onCaptureFinished: {
                        visible = false
                        newBinding.visible = true
                        newBinding.checked = false
                        const copy = backend.buttonMapping;
                        copy[buttonCapture.lastButton.buttonName] = keySequence
                        backend.buttonMapping = copy
                        root.changeSignal()
                    }
                }


                Item {
                    Kirigami.FormData.isSection: true
                }

                QQC2.Button  {
                    onClicked: root.pageStack.pop()
                    text: i18ndc("kcmmouse", "@action:button", "Go back")
                    icon.name: "go-previous"
                }
            }
        }
    }
}
