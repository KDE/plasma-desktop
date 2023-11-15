/*
    SPDX-FileCopyrightText: 2020 Ivan Čukić <ivan.cukic at kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Controls

import org.kde.kirigami as Kirigami
import org.kde.plasma.plasmoid
import org.kde.plasma.core as PlasmaCore
import org.kde.kcmutils as KCM

KCM.SimpleKCM {
    property alias cfg_showActivityIcon: radioCurrentActivityIcon.checked
    property alias cfg_showActivityName: checkShowActivityName.checked

    Kirigami.FormLayout {
        RadioButton {
            id: radioCurrentActivityIcon

            Kirigami.FormData.label: i18n("Icon:")

            text: i18n("Show the current activity icon")
        }

        RadioButton {
            id: radioGenericActivityIcon
            checked: !radioCurrentActivityIcon.checked
            text: i18n("Show the generic activity icon")
        }

        Item { Kirigami.FormData.isSection: true }

        CheckBox {
            id: checkShowActivityName

            enabled: Plasmoid.formFactor !== PlasmaCore.Types.Vertical

            Kirigami.FormData.label: i18n("Title:")

            text: i18n("Show the current activity name")
        }
    }
}
