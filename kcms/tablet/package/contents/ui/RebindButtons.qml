/*
    SPDX-FileCopyrightText: 2022 Aleix Pol Gonzalez <aleixpol@kde.org>
    SPDX-FileCopyrightText: 2022 David Redondo <kde@david-redondo.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.15 as QQC2
import org.kde.kirigami 2.6 as Kirigami
import org.kde.plasma.tablet.kcm 1.1
import org.kde.kcm 1.3
import org.kde.kquickcontrols 2.0

Kirigami.FormLayout
{
    id: root
    required property string path
    required property string name
    TabletEvents {
        id: tabletEvents

        anchors.fill: parent

        onPadButtonsChanged: {
            if (!path.endsWith(root.path)) {
                return;
            }
            buttonsRepeater.model = buttonCount
        }
    }


    Repeater {
        id: buttonsRepeater
        model: tabletEvents.padButtons

        delegate: RowLayout {
            Layout.fillWidth: true
            QQC2.Label {
                id: buttonLabel
                text: i18nd("kcm_tablet", "Button %1:", modelData + 1)
            }

            Connections {
                target: tabletEvents
                function onPadButtonReceived(path, button, pressed) {
                    if (button !== modelData || !path.endsWith(root.path)) {
                        return;
                    }
                    buttonLabel.font.bold = pressed
                }
            }

            KeySequenceItem {
                id: seq
                keySequence: kcm.padButtonMapping(root.name, modelData)
                Connections {
                    target: kcm
                    function onSettingsRestored() {
                        seq.keySequence = kcm.padButtonMapping(root.name, modelData)
                    }
                }

                modifierlessAllowed: true
                multiKeyShortcutsAllowed: false
                checkForConflictsAgainst: ShortcutType.None

                onCaptureFinished: {
                    kcm.assignPadButtonMapping(root.name, modelData, keySequence)
                }
            }
        }
    }
}
