/*
    SPDX-FileCopyrightText: 2020 Ivan Čukić <ivan.cukic at kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Controls as QQC2

import org.kde.kirigami as Kirigami
import org.kde.plasma.plasmoid
import org.kde.plasma.core as PlasmaCore
import org.kde.kcmutils as KCM

KCM.SimpleKCM {
    property alias cfg_showActivityIcon: radioCurrentActivityIcon.checked
    property alias cfg_showActivityName: checkShowActivityName.checked

    Kirigami.Form {
        Kirigami.FormGroup {
            QQC2.ButtonGroup { id: iconGroup }

            Kirigami.FormEntry {
                title: i18nc("@title:group prefix for radiobutton group", "Icon:")
                contentItem: QQC2.RadioButton {
                    id: radioCurrentActivityIcon
                    text: i18nc("@option:radio", "Show the current activity icon")
                }
            }

            Kirigami.FormEntry {
                contentItem: QQC2.RadioButton {
                    id: radioGenericActivityIcon
                    checked: !radioCurrentActivityIcon.checked
                    text: i18nc("@option:radio", "Show the generic activity icon")
                }
            }
        }

        Kirigami.FormGroup {
            enabled: Plasmoid.formFactor !== PlasmaCore.Types.Vertical
            Kirigami.FormEntry {
                title: i18nc("@label prefix for checkbox", "Title:")
                contentItem: QQC2.CheckBox {
                    id: checkShowActivityName
                    text: i18nc("@option:check title", "Show the current activity name")
                }
            }
        }
    }
}
