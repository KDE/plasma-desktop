
import QtQuick 2.15
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.3 as QQC2
import org.kde.kirigami 2.19 as Kirigami
import org.kde.plasma.tablet.kcm 1.1
import org.kde.kcmutils
import org.kde.kquickcontrols 2.0

Kirigami.FormLayout {
    required property var device

    Repeater {
        model: [
            {value: 0x14b, text: i18nd("kcm_tablet", "Pen button 1:")},
            {value: 0x14c, text: i18nd("kcm_tablet", "Pen button 2:")},
            {value: 0x149, text: i18nd("kcm_tablet", "Pen button 3:")}
        ] // BTN_STYLUS, BTN_STYLUS2, BTN_STYLUS3

        delegate: KeySequenceItem {
            id: seq
            Kirigami.FormData.label: (pressed ? "<b>" : "") + modelData.text + (pressed ? "</b>" : "")
            property bool pressed: false

            Connections {
                target: tabletEvents

                function onToolButtonReceived(hardware_serial_hi, hardware_serial_lo, button, pressed) {
                    if (button !== modelData.value) {
                        return;
                    }
                    seq.pressed = pressed
                }
            }

            keySequence: kcm.toolButtonMapping(form.device.name, modelData.value)
            Connections {
                target: kcm

                function onSettingsRestored() {
                    seq.keySequence = kcm.toolButtonMapping(form.device.name, modelData.value)
                }
            }

            showCancelButton: true
            modifierlessAllowed: true
            modifierOnlyAllowed: true
            multiKeyShortcutsAllowed: false
            checkForConflictsAgainst: ShortcutType.None

            onCaptureFinished: {
                kcm.assignToolButtonMapping(form.device.name, modelData.value, keySequence)
            }
        }
    }
}