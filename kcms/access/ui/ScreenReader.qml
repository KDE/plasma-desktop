/*
    SPDX-FileCopyrightText: 2018 Tomaz Canabrava <tcanabrava@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

import QtQuick 2.6
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.12 as QQC2
import org.kde.kcmutils as KCM
import org.kde.kirigami 2.4 as Kirigami

Kirigami.FormLayout {
    property var screenReaderInstalled : null

    QQC2.CheckBox {
        id: enableScreenReader
        text: i18n("Enable Screen Reader")

        KCM.SettingStateBinding {
            configObject: kcm.screenReaderSettings
            settingName: "Enabled"
        }

        checked: kcm.screenReaderSettings.enabled
        onToggled: kcm.screenReaderSettings.enabled = checked
    }
    QQC2.Button {
        text: i18n("Launch Orca Screen Reader Configuration…")

        enabled: !kcm.screenReaderSettings.isImmutable("Enabled") && screenReaderInstalled

        onClicked: kcm.launchOrcaConfiguration()
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
    Kirigami.InlineMessage {
        text: i18n("You may have to log out or reboot for the screen reader to work properly.")
        visible: screenReaderInstalled && enableScreenReader.checked
        type: Kirigami.MessageType.Warning
        Layout.fillWidth: true
    }
    Kirigami.InlineMessage {
        text: i18n("The Orca Screen Reader is not installed. Please install it before trying to use this feature, then log out or reboot.")
        visible: !screenReaderInstalled
        type: Kirigami.MessageType.Warning
        Layout.fillWidth: true
    }
}
