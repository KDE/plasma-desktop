/*
 * Copyright 2018 Tomaz Canabrava <tcanabrava@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
        text: i18n("Launch Orca Screen Reader Configuration...")

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
