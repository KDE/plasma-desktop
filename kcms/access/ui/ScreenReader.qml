/*
    SPDX-FileCopyrightText: 2018 Tomaz Canabrava <tcanabrava@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kcmutils as KCMUtils
import org.kde.kirigami as Kirigami


Loader {
    active: kcm.orcaInstalled()
    Kirigami.PlaceholderMessage {
        anchors.centerIn: parent
        width: Math.min(implicitWidth, parent.width)
        height: Math.min(implicitHeight, parent.height)
        visible: !parent.active
        icon.name: "preferences-desktop-text-to-speech"
        text: i18nc("@info Placeholder message title", "The Orca Screen Reader is not installed")
        explanation: i18nc("@info Placeholder message explanation", "Please install it, then close and reopen this window")
    }
    sourceComponent: Kirigami.Form {
        Kirigami.FormGroup {
            Kirigami.FormEntry {
                contentItem: QQC2.CheckBox {
                    id: enableScreenReader
                    text: i18nc("@option:check", "Enable Screen Reader")

                    KCMUtils.SettingHighlighter {
                        highlight: !kcm.screenReaderIsDefaults
                    }

                    checked: kcm.screenReaderEnabled
                    onToggled: kcm.screenReaderEnabled = checked
                }
            }
            Kirigami.FormAction {
                enabled: action.enabled
                subtitle: kcm.orcaLaunchFeedback
                action: QQC2.Action {
                    text: i18nc("@action:button", "Launch Orca Screen Reader Configuration…")
                    onTriggered: kcm.launchOrcaConfiguration()
                }
            }
        }
    }
}
