// -*- coding: iso-8859-1 -*-
/*
 *   Copyright 2013 Alexander Mezin <mezin.alexander@gmail.com>
 *   Copyright 2016 Kai Uwe Broulik <kde@privat.broulik.de>
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
import QtQuick.Layouts 1.1

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.extras 2.0 as PlasmaExtras

import org.kde.plasma.plasmoid 2.0

Item {
    id: root

    // Don't de-duplicate `touchpadEnabled` expression using `hasTouchpad`
    // property, QML doesn't work that way. Order of signals propagation is
    // not specified, so if/when data source disconnects, touchpadEnabled
    // might get re-evaluated while hasTouchpad is still true.
    readonly property bool hasTouchpad: typeof dataSource.data.touchpad !== "undefined" && dataSource.data.touchpad.workingTouchpadFound
    readonly property bool touchpadEnabled: typeof dataSource.data.touchpad !== "undefined" && dataSource.data.touchpad.workingTouchpadFound
        && dataSource.data.touchpad.enabled

    Plasmoid.preferredRepresentation: Plasmoid.compactRepresentation
    Plasmoid.icon: touchpadEnabled ? "input-touchpad-on" : "input-touchpad-off"
    // Touchpad being enabled is normal; only show the applet when it's disabled
    Plasmoid.status: {
        if (!hasTouchpad) {
            return PlasmaCore.Types.HiddenStatus;
        } else if (touchpadEnabled) {
            return PlasmaCore.Types.PassiveStatus;
        } else {
            return PlasmaCore.Types.ActiveStatus;
        }
    }

    Plasmoid.toolTipSubText: {
        if (!hasTouchpad) {
            return i18n("No touchpad was found");
        } else if (touchpadEnabled) {
            return i18n("Touchpad is enabled");
        } else {
            return i18n("Touchpad is disabled");
        }
    }

    PlasmaCore.DataSource {
        id: dataSource
        engine: "touchpad"
        connectedSources: dataSource.sources
    }

    Plasmoid.compactRepresentation: PlasmaCore.IconItem {
        implicitWidth: PlasmaCore.Units.iconSizes.small
        implicitHeight: PlasmaCore.Units.iconSizes.small

        source: Plasmoid.icon
        active: mousearea.containsMouse

        PlasmaCore.ToolTipArea {
            mainText: Plasmoid.title
            subText: Plasmoid.toolTipSubText
        }

        MouseArea {
            id: mousearea

            anchors.fill: parent
            onClicked: {
                Plasmoid.expanded = !Plasmoid.expanded;
            }
        }
    }

    // This is only accessible from System Tray, when hidden in the popup
    // and you click the list item text instead of the icon
    Plasmoid.fullRepresentation: Item {

        PlasmaExtras.PlaceholderMessage {
            anchors.centerIn: parent
            width: parent.width - (PlasmaCore.Units.largeSpacing * 8)
            text: Plasmoid.toolTipSubText
            iconName: Plasmoid.icon
        }
    }
}
