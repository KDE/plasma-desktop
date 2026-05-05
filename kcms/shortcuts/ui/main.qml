/*
    SPDX-FileCopyrightText: 2020 David Redondo <david@david-redondo.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kcmutils as KCMUtils


KCMUtils.SimpleKCM {
    id: root

    Kirigami.Form {
        Kirigami.FormGroup {
            Kirigami.FormAction {
                action: Kirigami.Action {
                    text: i18n("Keyboard Shortcuts")
                    icon.name: "preferences-desktop-keyboard-shortcut"
                }
            }

            Kirigami.FormSeparator {}

            Kirigami.FormAction {
                action: Kirigami.Action {
                    text: i18n("Touchpad Gestures")
                    icon.name:"preferences-desktop-touchpad"
                    onTriggered: kcm.push("TouchpadGestures.qml")
                }
            }

            Kirigami.FormSeparator {}

            Kirigami.FormAction {
                action: Kirigami.Action {
                    text: i18n("Touchscreen Gestures")
                    icon.name: "preferences-desktop-gestures-touch"
                }
            }

            Kirigami.FormSeparator {}

            Kirigami.FormAction {
                action: Kirigami.Action {
                    text: i18n("Screen Edge Gestures")
                    icon.name: "preferences-desktop-gestures-screenedges"
                }
            }

            Kirigami.FormSeparator {}

            Kirigami.FormAction {
                action: Kirigami.Action {
                    text: i18n("Line Shape Gestures")
                    icon.name:"preferences-desktop-mouse"
                }
            }

            Kirigami.FormSeparator {}

            Kirigami.FormAction {
                action: Kirigami.Action {
                    text: i18n("Scroll Gestures")
                    icon.name:"preferences-scroll"
                }
            }
        }
    }
}
