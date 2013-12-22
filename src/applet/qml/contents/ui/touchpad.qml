// -*- coding: iso-8859-1 -*-
/*
 *   Copyright 2013 Alexander Mezin <mezin.alexander@gmail.com>
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

import QtQuick 1.1
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.plasma.components 0.1 as PlasmaComponents

Item {
    id: root

    Component.onCompleted: {
        plasmoid.aspectRatioMode = Square
    }

    PlasmaCore.DataSource {
        id: dataSource
        engine: "touchpad"
        connectedSources: dataSource.sources
        onNewData: {
            if (data.enabled === null) {
                return
            }

            if (data.enabled) {
                //Hide plasmoid from notification area after short delay
                delayedStatusUpdate.restart()
            } else {
                plasmoid.status = ActiveStatus
            }

            icon.elementId = data.enabled ? "touchpad_enabled"
                                          : "touchpad_disabled"

            plasmoid.setAction("toggle", data.enabled ? i18n("Disable touchpad")
                                                      : i18n("Enable touchpad"))
        }
    }

    property bool hasTouchpad: typeof dataSource.data.touchpad != 'undefined'
    property bool enabled: hasTouchpad ? dataSource.data.touchpad.enabled
                                       : false
    property bool mouse: hasTouchpad ? dataSource.data.touchpad.mousePluggedIn
                                     : false

    Timer {
        id: delayedStatusUpdate
        interval: 1000
        running: true
        onTriggered: {
            if (!hasTouchpad) {
                //Setting this in Component.onCompleted didn't work
                plasmoid.status = PassiveStatus
                return
            }

            plasmoid.status = enabled ? PassiveStatus : ActiveStatus
        }
    }

    PlasmaCore.SvgItem {
        id: icon
        anchors.fill: parent
        svg: PlasmaCore.Svg {
            multipleImages: true
            imagePath: "icons/touchpad"
        }
        visible: hasTouchpad
    }

    PlasmaCore.ToolTip {
        target: root
        mainText: {
            if (!hasTouchpad) {
                return i18n("Touchpad is not found");
            }

            return enabled ? i18n("Touchpad is enabled")
                           : i18n("Touchpad is disabled")
        }
        image: enabled ? "input-touchpad" : "dialog-cancel"
    }

    MouseArea {
        anchors.fill: parent
        onClicked: action_toggle()
        enabled: hasTouchpad
    }

    function action_toggle() {
        if (!mouse && enabled) {
            confirmDialog.open()
            return
        }
        toggle()
    }

    function toggle() {
        var service = dataSource.serviceForSource("touchpad")
        service.startOperationCall(service.operationDescription("toggle"))
    }

    PlasmaComponents.QueryDialog {
        id: confirmDialog
        visualParent: root
        titleText: i18n("Touchpad")
        titleIcon: "dialog-warning"
        message: i18n("No mouses were detected.\nAre you sure you want to disable touchpad?")
        acceptButtonText: i18n("Disable")
        onAccepted: {
            if (enabled) {
                toggle()
            }
        }
    }
}
