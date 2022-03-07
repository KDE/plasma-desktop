/*
    SPDX-FileCopyrightText: 2020 Ivan Čukić <ivan.cukic at kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.0
import QtQuick.Controls 2.5

import org.kde.kirigami 2.5 as Kirigami
import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore

Kirigami.FormLayout {
    anchors.left: parent.left
    anchors.right: parent.right

    property alias cfg_showActivityIcon: radioCurrentActivityIcon.checked
    property alias cfg_showActivityName: checkShowActivityName.checked

    Item { Kirigami.FormData.isSection: true }

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
