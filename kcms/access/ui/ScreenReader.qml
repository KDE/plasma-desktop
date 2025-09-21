/*
    SPDX-FileCopyrightText: 2018 Tomaz Canabrava <tcanabrava@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kcmutils as KCMUtils
import org.kde.kirigami as Kirigami

Item {
    Kirigami.PlaceholderMessage {
        anchors.centerIn: parent
        icon.name: "preferences-desktop-text-to-speech"
        text: i18nc("@info Placeholder message title", "The Orca Screen Reader is not installed")
        explanation: i18nc("@info Placeholder message explanation", "Please install it, then close and reopen this window")
        visible: !kcm.screenReaderInstalled
    }
    Kirigami.FormLayout {
        QQC2.CheckBox {
            id: startScreenReaderOnStartup
            text: i18nc("@option:check", "Start Screen Reader on Startup")

            KCMUtils.SettingStateBinding {
                configObject: kcm.screenReaderSettings
                // keep the settingName to be backwards compatible
                settingName: "Enabled"
            }

            visible: kcm.screenReaderInstalled
            enabled: !kcm.screenReaderSettings.isImmutable("Enabled") && kcm.screenReaderInstalled
            checked: kcm.screenReaderSettings.enabled
            onToggled: kcm.screenReaderSettings.enabled = checked
        }
        QQC2.CheckBox {
            id: enableScreenReader
            text: i18nc("@option:check", "Enable Screen Reader")
            visible: kcm.screenReaderInstalled
            checked: kcm.screenReaderEnabled
            onToggled: kcm.screenReaderEnabled = checked
        }
        QQC2.Button {
            text: i18nc("@action:button", "Launch Orca Screen Reader Configuration…")
            visible: kcm.screenReaderInstalled
            enabled: !kcm.screenReaderSettings.isImmutable("Enabled") && kcm.screenReaderInstalled
            onClicked: kcm.launchOrcaConfiguration()
        }
        QQC2.Label {
            text: kcm.orcaLaunchFeedback
            textFormat: Text.PlainText
        }
    }
}
