/*
 *   Copyright 2020 Ivan Čukić <ivan.cukic at kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2 or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
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
    property bool disableSetting: plasmoid.formFactor === PlasmaCore.Types.Vertical

    Item { Kirigami.FormData.isSection: true }

    RadioButton {
        id: radioCurrentActivityIcon

        Kirigami.FormData.label: i18n("Icon:")

        enabled: !disableSetting
        text: i18n("Show the current activity icon")
    }

    RadioButton {
        id: radioGenericActivityIcon
        enabled: !disableSetting
        checked: !radioCurrentActivityIcon.checked
        text: i18n("Show the generic activity icon")
    }

    Item { Kirigami.FormData.isSection: true }

    CheckBox {
        id: checkShowActivityName

        Kirigami.FormData.label: i18n("Title:")

        text: i18n("Show the current activity name")
    }
}
