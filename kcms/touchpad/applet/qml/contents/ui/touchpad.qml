// -*- coding: iso-8859-1 -*-
/*
 *   SPDX-FileCopyrightText: 2013 Alexander Mezin <mezin.alexander@gmail.com>
 *   SPDX-FileCopyrightText: 2016 Kai Uwe Broulik <kde@privat.broulik.de>
 *
 *   SPDX-License-Identifier: LGPL-2.0-or-later
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

        source: plasmoid.icon
        active: mousearea.containsMouse

        PlasmaCore.ToolTipArea {
            mainText: plasmoid.title
            subText: plasmoid.toolTipSubText
        }

        MouseArea {
            id: mousearea

            anchors.fill: parent
            onClicked: {
                plasmoid.expanded = !plasmoid.expanded;
            }
        }
    }

    // This is only accessible from System Tray, when hidden in the popup
    // and you click the list item text instead of the icon
    Plasmoid.fullRepresentation: Item {

        PlasmaExtras.PlaceholderMessage {
            anchors.centerIn: parent
            width: parent.width - (PlasmaCore.Units.largeSpacing * 8)
            text: plasmoid.toolTipSubText
            iconName: plasmoid.icon
        }
    }
}
