/*
    SPDX-FileCopyrightText: 2018 Tomaz Canabrava <tcanabrava@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

import QtQuick 2.6
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.12 as QQC2
import org.kde.kcmutils as KCM
import org.kde.kirigami 2.20 as Kirigami

Kirigami.FormLayout {
    property var screenReaderInstalled : null

    QQC2.CheckBox {
        id: enableScreenReader
        text: i18n("Enable Screen Reader")

        KCM.SettingStateBinding {
            configObject: kcm.screenReaderSettings
            settingName: "Enabled"
        }

        visible: screenReaderInstalled
        checked: kcm.screenReaderSettings.enabled
        onToggled: kcm.screenReaderSettings.enabled = checked

        Accessible.role: Accessible.CheckBox
        Accessible.name: text
    }
    QQC2.Button {
        text: i18n("Launch Orca Screen Reader Configuration…")
        visible: screenReaderInstalled
        enabled: !kcm.screenReaderSettings.isImmutable("Enabled") && screenReaderInstalled
        onClicked: kcm.launchOrcaConfiguration()

        Accessible.role: Accessible.Button
        Accessible.name: text
        Accessible.description: i18n("Opens the settings for the Orca Screen Reader in a new window.")
    }
    QQC2.Label {
        text: kcm.orcaLaunchFeedback
        textFormat: Text.PlainText
    }

    onVisibleChanged: {
        if (visible === true && screenReaderInstalled === null) {
            screenReaderInstalled = kcm.orcaInstalled()
        }
    }
    Kirigami.PlaceholderMessage {
        icon.name: "preferences-desktop-text-to-speech"
        text: i18n("The Orca Screen Reader is not installed.")
        explanation: i18n("Please install it before trying to use this feature, then log out or reboot.")
        visible: !screenReaderInstalled

        Accessible.role: Accessible.StaticText
        Accessible.name: text
        Accessible.description: explanation
    }
}
