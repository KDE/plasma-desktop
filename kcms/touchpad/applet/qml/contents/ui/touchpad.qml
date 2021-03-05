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
import org.kde.plasma.components 2.0 as PlasmaComponents

import org.kde.plasma.plasmoid 2.0

Item {
    id: root

    readonly property bool hasTouchpad: typeof dataSource.data.touchpad !== "undefined" && dataSource.data.touchpad.workingTouchpadFound
    readonly property bool touchpadEnabled: hasTouchpad ? dataSource.data.touchpad.enabled : false
    readonly property bool hasMouse: hasTouchpad ? dataSource.data.touchpad.mousePluggedIn : false

    Plasmoid.preferredRepresentation: Plasmoid.compactRepresentation
    Plasmoid.icon: touchpadEnabled ? "input-touchpad-on" : "input-touchpad-off"
    Plasmoid.status: {
        if (confirmDialog.status !== PlasmaComponents.DialogStatus.Closed) {
            return PlasmaCore.Types.AcceptingInputStatus;
        } else if (hasTouchpad) {
            return PlasmaCore.Types.ActiveStatus;
        } else {
            return PlasmaCore.Types.HiddenStatus;
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
    Plasmoid.onActivated: action_toggle()

    Component.onCompleted: {
        // the "text" argument for setAction is mandatory but we overwrite
        // it by a binding right away anyway
        plasmoid.setAction("toggle", "");

        var action = plasmoid.action("toggle");
        action.text = Qt.binding(function() {
            return root.touchpadEnabled ? i18n("Disable touchpad") : i18n("Enable touchpad");
        });
        action.visible = Qt.binding(function() {
            return root.hasTouchpad;
        });
    }

    PlasmaCore.DataSource {
        id: dataSource
        engine: "touchpad"
        connectedSources: dataSource.sources
    }

    Plasmoid.compactRepresentation: PlasmaCore.ToolTipArea {
        id: toolTip

        Layout.minimumWidth: PlasmaCore.Units.iconSizes.small
        Layout.minimumHeight: Layout.minimumWidth

        mainText: plasmoid.title
        subText: plasmoid.toolTipSubText

        active: confirmDialog.status === PlasmaComponents.DialogStatus.Closed

        Connections {
            target: confirmDialog
            function onStatusChanged() {
                if (confirmDialog.status === PlasmaComponents.DialogStatus.Open) {
                    toolTip.hideToolTip()
                }
            }
        }

        MouseArea {
            anchors.fill: parent
            enabled: root.hasTouchpad
            onClicked: root.action_toggle()
        }

        PlasmaCore.IconItem {
            anchors.fill: parent
            source: plasmoid.icon
            active: parent.containsMouse
            enabled: root.hasTouchpad
        }
    }

    // This is only accessible from System Tray, when hidden in the popup
    // and you click the list item text instead of the icon
    Plasmoid.fullRepresentation: Item {

        PlasmaComponents.Button {
            readonly property QtObject action: plasmoid.action("toggle")
            anchors.centerIn: parent
            text: action.text
            enabled: action.visible
            onClicked: action.trigger()
        }

    }

    function action_toggle() {
        if (!root.hasTouchpad) {
            return;
        }

        if (!root.hasMouse && root.touchpadEnabled) {
            confirmDialog.open()
            return;
        }

        execOp("toggle")
    }

    function execOp(op) {
        var service = dataSource.serviceForSource("touchpad")
        service.startOperationCall(service.operationDescription(op))
    }

    PlasmaComponents.QueryDialog {
        id: confirmDialog
        visualParent: plasmoid.compactRepresentationItem
        hideOnWindowDeactivate: true
        titleText: i18n("Touchpad")
        titleIcon: "dialog-warning"
        message: i18n("No mouse was detected.\nAre you sure you want to disable the touchpad?")
        acceptButtonText: i18n("Disable")
        onAccepted: execOp("disable")
    }
}
