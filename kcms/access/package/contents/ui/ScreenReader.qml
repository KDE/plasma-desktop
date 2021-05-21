/*
    SPDX-FileCopyrightText: 2018 Tomaz Canabrava <tcanabrava@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

import QtQuick 2.6
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.12 as QQC2
import org.kde.kcm 1.3 as KCM
import org.kde.kirigami 2.3 as Kirigami

Kirigami.FormLayout {
    property var screenReaderInstalled : null

    QQC2.CheckBox {
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
    }
    QQC2.Label {
        Layout.fillWidth: true
        wrapMode: Text.Wrap
        text: screenReaderInstalled
            ? i18n("Please note that you may have to log out or reboot once to allow the screen reader to work properly.")
            : i18n("It appears that the Orca Screen Reader is not installed. Please install it before trying to use this feature, and then log out or reboot")
    }

    onVisibleChanged: {
        if (visible === true && screenReaderInstalled === null) {
            screenReaderInstalled = kcm.orcaInstalled()
        }
    }
}
