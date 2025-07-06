/*
    SPDX-FileCopyrightText: 2024 Joshua Goins <joshua.goins@kdab.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2

import org.kde.kirigami as Kirigami
import org.kde.plasma.tablet.kcm as KCM
import org.kde.kcmutils
import org.kde.kquickcontrols

Kirigami.Dialog {
    id: actionDialog

    property KCM.inputSequence inputSequence
    property string name
    property bool supportsPenButton
    property bool supportsRelativeEvents

    signal gotInputSequence(sequence: KCM.inputSequence)

    title: supportsRelativeEvents ?
        i18ndc("kcm_tablet", "@title Select the action for the tablet dial", "Select Dial/Ring Action")
        : i18ndc("kcm_tablet", "@title Select the action for the tablet button", "Select Button Action")
    modal: true

    maximumWidth: Kirigami.Units.gridUnit * 20
    maximumHeight: Kirigami.Units.gridUnit * 20

    topPadding: 0
    leftPadding: Kirigami.Units.largeSpacing
    rightPadding: Kirigami.Units.largeSpacing
    bottomPadding: 0

    standardButtons: QQC2.Dialog.Ok | QQC2.Dialog.Cancel
    showCloseButton: false

    onAccepted: gotInputSequence(inputSequence)

    function refreshDialogData(): void {
        switch (inputSequence.type) {
            case KCM.InputSequence.Disabled:
                disabledRadio.checked = true;
                break;
            case KCM.InputSequence.Keyboard:
                keyboardRadio.checked = true;
                seq.keySequence = inputSequence.keySequence();
                break;
            case KCM.InputSequence.RelativeKeyboard:
                relativeKeyboardRadio.checked = true;
                upSeq.keySequence = inputSequence.upKeySequence();
                downSeq.keySequence = inputSequence.downKeySequence();
                thresholdSlider.value = inputSequence.threshold();
                break;
            case KCM.InputSequence.Mouse:
                mouseRadio.checked = true;
                clickCombo.currentIndex = clickCombo.indexOfValue(inputSequence.mouseButton());

                const modifiers = actionDialog.inputSequence.keyboardModifiers();
                const checkFlag = (flag) => (modifiers & flag) === flag;
                ctrlCheckbox.checked = checkFlag(Qt.ControlModifier);
                altCheckbox.checked = checkFlag(Qt.AltModifier);
                metaCheckbox.checked = checkFlag(Qt.MetaModifier);
                shiftCheckbox.checked = checkFlag(Qt.ShiftModifier);
                break;
            case KCM.InputSequence.Pen:
                penRadio.checked = true;
                break;
            case KCM.InputSequence.Scroll:
                scrollRadio.checked = true;
                break;
            case KCM.InputSequence.ApplicationDefined:
                applicationRadio.checked = true;
                break;
        }
    }

    ColumnLayout {
        spacing: Kirigami.Units.smallSpacing

        QQC2.Label {
            text: actionDialog.supportsRelativeEvents ?
                i18ndc("kcm_tablet", "@info %1 is the 'pad button %1' or 'pen button 1/2/3' strings", "Choose what action will be taken when rotating %1:", actionDialog.name)
                : i18ndc("kcm_tablet", "@info %1 is the 'pad button %1' or 'pen button 1/2/3' strings", "Choose what action will be taken when pressing %1:", actionDialog.name)
            wrapMode: Text.WordWrap

            Layout.fillWidth: true
        }

        QQC2.ButtonGroup { id: radioGroup }

        QQC2.RadioButton {
            id: applicationRadio

            readonly property int index: 0

            icon.name: "applications-all-symbolic"
            text: i18ndc("kcm_tablet", "@option:radio Set this action to an application-defined type", "Defer to application")

            QQC2.ButtonGroup.group: radioGroup

            onToggled: {
                inputSequence.type = KCM.InputSequence.ApplicationDefined;
                refreshDialogData();
            }
        }
        QQC2.RadioButton {
            id: keyboardRadio

            readonly property int index: 1

            icon.name: "input-keyboard-symbolic"
            text: i18ndc("kcm_tablet", "@option:radio Set this action to a keyboard type", "Send keyboard key")
            visible: !actionDialog.supportsRelativeEvents

            QQC2.ButtonGroup.group: radioGroup

            onToggled: {
                inputSequence.type = KCM.InputSequence.Keyboard;
                refreshDialogData();
            }
        }
        QQC2.RadioButton {
            id: relativeKeyboardRadio

            readonly property int index: 2

            icon.name: "input-keyboard-symbolic"
            text: i18ndc("kcm_tablet", "@option:radio Set this action to a up/down keyboard type", "Send keyboard keys")
            visible: actionDialog.supportsRelativeEvents

            QQC2.ButtonGroup.group: radioGroup

            onToggled: {
                inputSequence.type = KCM.InputSequence.RelativeKeyboard;
                refreshDialogData();
            }
        }
        QQC2.RadioButton {
            id: mouseRadio

            readonly property int index: 3

            text: i18ndc("kcm_tablet", "@option:radio Set this action to a mouse type", "Send mouse button click")
            icon.name: "input-mouse-symbolic"
            visible: !actionDialog.supportsRelativeEvents

            QQC2.ButtonGroup.group: radioGroup

            onToggled: {
                inputSequence.type = KCM.InputSequence.Mouse;
                refreshDialogData();
            }
        }
        QQC2.RadioButton {
            id: penRadio

            readonly property int index: 4

            text: i18ndc("kcm_tablet", "@option:radio Set this action to a pen button type", "Send pen button")
            icon.name: "tool_pen-symbolic"
            visible: actionDialog.supportsPenButton
            enabled: visible

            QQC2.ButtonGroup.group: radioGroup

            onToggled: {
                inputSequence.type = KCM.InputSequence.Pen;
                refreshDialogData();
            }
        }
        QQC2.RadioButton {
            id: scrollRadio

            readonly property int index: 5

            text: i18ndc("kcm_tablet", "@option:radio Set this action to a scroll wheel type type", "Act as scroll wheel")
            icon.name: "input-mouse-click-middle"
            visible: actionDialog.supportsRelativeEvents
            enabled: visible

            QQC2.ButtonGroup.group: radioGroup

            onToggled: {
                inputSequence.type = KCM.InputSequence.Scroll;
                refreshDialogData();
            }
        }
        QQC2.RadioButton {
            id: disabledRadio

            readonly property int index: 6

            icon.name: "action-unavailable-symbolic"
            text: i18ndc("kcm_tablet", "@option:radio Disable this action", "Do nothing")

            QQC2.ButtonGroup.group: radioGroup

            onToggled: {
                inputSequence.type = KCM.InputSequence.Disabled;
                refreshDialogData();
            }
        }

        QQC2.MenuSeparator {
            Layout.fillWidth: true
        }

        StackLayout {
            currentIndex: radioGroup.checkedButton?.index ?? 0

            QQC2.Label {
                id: applicationView

                text: i18ndc("kcm_tablet", "@info", "This tablet button is forwarded to the focused application, which can decide how to handle it." )

                wrapMode: Text.WordWrap

                Layout.fillWidth: true
            }
            KeySequenceItem {
                id: seq

                showCancelButton: true
                modifierlessAllowed: true
                modifierOnlyAllowed: true
                multiKeyShortcutsAllowed: false
                checkForConflictsAgainst: ShortcutType.None

                onCaptureFinished: actionDialog.inputSequence.setKeySequence(keySequence)
            }
            Kirigami.FormLayout {
                id: relativeKeyboardView

                KeySequenceItem {
                    id: upSeq

                    showCancelButton: true
                    modifierlessAllowed: true
                    modifierOnlyAllowed: true
                    multiKeyShortcutsAllowed: false
                    checkForConflictsAgainst: ShortcutType.None

                    onCaptureFinished: actionDialog.inputSequence.setUpKeySequence(keySequence)

                    Kirigami.FormData.label: i18nd("kcm_tablet Keybind to send when dial/ring is turned upwards", "Up:")
                }

                KeySequenceItem {
                    id: downSeq

                    showCancelButton: true
                    modifierlessAllowed: true
                    modifierOnlyAllowed: true
                    multiKeyShortcutsAllowed: false
                    checkForConflictsAgainst: ShortcutType.None

                    onCaptureFinished: actionDialog.inputSequence.setDownKeySequence(keySequence)

                    Kirigami.FormData.label: i18nd("kcm_tablet Keybind to send when dial/ring is turned downwards", "Down:")
                }

                QQC2.Slider {
                    id: thresholdSlider

                    from: 5400 // or about 45 degrees
                    to: 120 // or about 1 degrees
                    stepSize: 120
                    snapMode: QQC2.Slider.SnapOnRelease

                    onMoved: actionDialog.inputSequence.setThreshold(value)

                    Kirigami.FormData.label: i18nd("kcm_tablet Speed for how often this dial/ring should emit key events", "Speed:")
                    Layout.fillWidth: true
                }
            }
            ColumnLayout {
                id: mouseView

                spacing: Kirigami.Units.smallSpacing

                QQC2.ComboBox {
                    id: clickCombo

                    textRole: "text"
                    valueRole: "value"
                    model: [
                        { value: Qt.LeftButton, text: i18ndc("kcm_tablet", "@action:inmenu Left mouse click", "Left-click") },
                        { value: Qt.MiddleButton, text: i18ndc("kcm_tablet", "@action:inmenu Middle mouse click", "Middle-click") },
                        { value: Qt.RightButton, text: i18ndc("kcm_tablet", "@action:inmenu Right mouse click", "Right-click") }
                    ]

                    onActivated: actionDialog.inputSequence.setMouseButton(currentValue)
                }

                RowLayout {
                    spacing: Kirigami.Units.largeSpacing

                    component ModifierCheckBox: QQC2.CheckBox {
                        required property /*Qt::KeyboardModifier*/ int modifier

                        onToggled: {
                            let modifiers = actionDialog.inputSequence.keyboardModifiers();
                            if (checked) {
                                modifiers |= modifier;
                            } else {
                                modifiers &= ~modifier;
                            }

                            actionDialog.inputSequence.setKeyboardModifiers(modifiers);
                        }
                    }

                    ModifierCheckBox {
                        id: ctrlCheckbox

                        text: i18ndc("kcm_tablet", "@option:check The ctrl modifier on the keyboard", "Ctrl")
                        modifier: Qt.ControlModifier
                    }

                    ModifierCheckBox {
                        id: altCheckbox

                        text: i18ndc("kcm_tablet", "@option:check The alt modifier on the keyboard", "Alt")
                        modifier: Qt.AltModifier
                    }

                    ModifierCheckBox {
                        id: metaCheckbox

                        text: i18ndc("kcm_tablet", "@option:check The meta modifier on the keyboard", "Meta")
                        modifier: Qt.MetaModifier
                    }

                    ModifierCheckBox {
                        id: shiftCheckbox

                        text: i18ndc("kcm_tablet", "@option:check The shift modifier on the keyboard", "Shift")
                        modifier: Qt.ShiftModifier
                    }
                }
            }
            ColumnLayout {
                id: penButtonView

                spacing: Kirigami.Units.smallSpacing

                QQC2.ComboBox {
                    id: penButtonCombo

                    textRole: "text"
                    valueRole: "value"
                    model: [
                        { value: 0, text: i18ndc("kcm_tablet", "@action:inmenu Stylus button", "Button 1") },
                        { value: 1, text: i18ndc("kcm_tablet", "@action:inmenu Stylus button", "Button 2") },
                        { value: 2, text: i18ndc("kcm_tablet", "@action:inmenu Stylus button", "Button 3") }
                    ]

                    onActivated: actionDialog.inputSequence.setPenButton(currentValue)
                }
            }
            QQC2.Label {
                id: scrollView

                text: i18ndc("kcm_tablet", "@info", "Act like a scroll wheel on a mouse.")

                wrapMode: Text.WordWrap

                Layout.fillWidth: true
            }
            QQC2.Label {
                id: disabledView

                text: i18ndc("kcm_tablet", "@info", "This button is disabled and will not do anything.")

                wrapMode: Text.WordWrap

                Layout.fillWidth: true
            }
        }
    }
}
