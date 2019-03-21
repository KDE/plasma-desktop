/*
 * Copyright 2019 Kai Uwe Broulik <kde@privat.broulik.de>
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

import QtQuick 2.9
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.3 as QtControls
import org.kde.kirigami 2.4 as Kirigami
import org.kde.kcm 1.2 as KCM

KCM.SimpleKCM {

    title: i18n("Do not Disturb Times")

    Kirigami.FormLayout {
        width: parent.width

        Repeater {
            model: 7

            Rectangle {
                property int dayNumber: (Qt.locale().firstDayOfWeek + index) % 7
                Kirigami.FormData.label: Qt.locale().dayName(dayNumber)
                Kirigami.Theme.colorSet: Kirigami.Theme.View
                Kirigami.Theme.inherit: false
                Layout.fillWidth: true
                color: Kirigami.Theme.backgroundColor
                height: Kirigami.Units.gridUnit
                border {
                    width: 1
                    color: Kirigami.Theme.textColor
                }
            }
        }

        // FIXME fix formatting (no seconds and timezone), make separate component
        QtControls.SpinBox {
            enabled: dndTimeCheck.checked
            from: 0
            to: 23 * 60 + 59
            value: 22 * 60
            stepSize: 15
            textFromValue: function(value, locale) {
                return new Date(1,1,2019,Math.floor(value / 60),value%60,0).toLocaleTimeString(locale);
            }
        }

        QtControls.Label {
            text: i18nc("Enable between hh:mm and hh:mm", "and")
        }

        QtControls.SpinBox {
            enabled: dndTimeCheck.checked
            from: 0
            to: 23 * 60 + 59
            value: 6 * 60
            stepSize: 15
            textFromValue: function(value, locale) {
                return new Date(1,1,2019,Math.floor(value / 60),value%60,0).toLocaleTimeString(locale);
            }
        }
    }
}
