/***************************************************************************
 *   Copyright (C) 2013 by Eike Hein <hein@kde.org>                        *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

import QtQuick 2.0
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.12
import org.kde.kirigami 2.4 as Kirigami

import org.kde.plasma.core 2.0 as PlasmaCore

Item {
    width: childrenRect.width
    height: childrenRect.height

    readonly property bool plasmaPaAvailable: Qt.createComponent("PulseAudio.qml").status === Component.Ready

    property bool plasmoidVertical: (plasmoid.formFactor === PlasmaCore.Types.Vertical)

    property alias cfg_showToolTips: showToolTips.checked
    property alias cfg_highlightWindows: highlightWindows.checked
    property bool cfg_indicateAudioStreams
    property alias cfg_iconSize: iconSize.value
    property alias cfg_maxStripes: maxStripes.value
    property alias cfg_forceStripes: forceStripes.checked

    Kirigami.FormLayout {
        anchors.left: parent.left
        anchors.right: parent.right

        CheckBox {
            id: showToolTips
            Kirigami.FormData.label: i18n ("General:")
            text: i18n("Show tooltips")
        }
        
        RowLayout {
            // HACK: Workaround for Kirigami bug 434625
            // due to which a simple Layout.leftMargin on CheckBox doesn't work
            Item { implicitWidth: Kirigami.Units.gridUnit }
            CheckBox {
                id: highlightWindows
                text: i18n("Highlight windows when hovering over task tooltips")
                enabled: showToolTips.checked
            }
        }

        CheckBox {
            id: indicateAudioStreams
            text: i18n("Mark applications that play audio")
            checked: cfg_indicateAudioStreams && plasmaPaAvailable
            onCheckedChanged: cfg_indicateAudioStreams = checked
            enabled: plasmaPaAvailable
        }

        Item {
            Kirigami.FormData.isSection: true
            visible: plasmoidVertical
        }

        Slider {
            id: iconSize
            visible: plasmoidVertical
            Kirigami.FormData.label: i18n("Icon size:")
            Layout.fillWidth: true
            from: 0
            to: 5
            stepSize: 1
        }

        RowLayout {
            visible: plasmoidVertical
            Layout.fillWidth: true

            Label {
                text: i18n("Small")
                Layout.alignment: Qt.AlignLeft
            }

            Item {
                Layout.fillWidth: true
            }

            Label {
                text: i18n("Large")
                Layout.alignment: Qt.AlignRight
            }
        }

        Item {
            Kirigami.FormData.isSection: true
        }

        SpinBox {
            id: maxStripes
            Kirigami.FormData.label: plasmoidVertical ? i18n("Maximum columns:") : i18n("Maximum rows:")
            from: 1
            }

        CheckBox {
            id: forceStripes
            text: plasmoidVertical ? i18n("Always arrange tasks in rows of as many columns") : i18n("Always arrange tasks in columns of as many rows")
            enabled: maxStripes.value > 1
        }
    }
}
