/*
 *  Copyright 2013 Marco Martin <mart@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  2.010-1301, USA.
 */

import QtQuick 2.0
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.configuration 2.0
import org.kde.kquickcontrolsaddons 2.0 as KQuickControlsAddons

PlasmaComponents.Button {
    text: i18nd("plasma_shell_org.kde.plasma.desktop", "Screen Edge")
    checkable: true
    checked: mel.pressed

    KQuickControlsAddons.MouseEventListener {
        id: mel
        cursorShape: Qt.DragMoveCursor
        anchors.fill: parent
        property int lastX
        property int lastY
        property int startMouseX
        property int startMouseY
        onPressed: {
            dialogRoot.closeContextMenu();
            lastX = mouse.screenX
            lastY = mouse.screenY
            startMouseX = mouse.x
            startMouseY = mouse.y
        }
        onPositionChanged: {
            panel.screen = mouse.screen;

            var newLocation = panel.location;
            //If the mouse is in an internal rectangle, do nothing
            if ((mouse.screenX < panel.screenGeometry.x + panel.screenGeometry.width/3 ||
                mouse.screenX > panel.screenGeometry.x + panel.screenGeometry.width/3*2) ||
                (mouse.screenY < panel.screenGeometry.y + panel.screenGeometry.height/3 ||
                mouse.screenY > panel.screenGeometry.y + panel.screenGeometry.height/3*2))
            {
                var screenAspect = panel.screenGeometry.height / panel.screenGeometry.width;

                if (mouse.screenY < panel.screenGeometry.y+(mouse.screenX-panel.screenGeometry.x)*screenAspect) {
                    if (mouse.screenY < panel.screenGeometry.y + panel.screenGeometry.height-(mouse.screenX-panel.screenGeometry.x)*screenAspect) {
                        newLocation = PlasmaCore.Types.TopEdge;
                    } else {
                        newLocation = PlasmaCore.Types.RightEdge;
                    }

                } else {
                    if (mouse.screenY < panel.screenGeometry.y + panel.screenGeometry.height-(mouse.screenX-panel.screenGeometry.x)*screenAspect) {
                        newLocation = PlasmaCore.Types.LeftEdge;
                    } else {
                        newLocation = PlasmaCore.Types.BottomEdge;
                    }
                }
            }
            panel.location = newLocation

            switch (newLocation) {
            case PlasmaCore.Types.TopEdge:
                var y = Math.max(mouse.screenY - mapToItem(dialogRoot, 0, startMouseY).y, panel.height);
                configDialog.y = y;
                panel.distance = Math.max(y - panel.height - panel.screen.geometry.y, 0);
                break
            case PlasmaCore.Types.LeftEdge:
                var x = Math.max(mouse.screenX - mapToItem(dialogRoot, startMouseX, 0).x, panel.width);
                configDialog.x = x;
                panel.distance = Math.max(x - panel.width - panel.screen.geometry.x, 0);
                break;
            case PlasmaCore.Types.RightEdge:
                var x = Math.min(mouse.screenX - mapToItem(dialogRoot, startMouseX, 0).x, mouse.screen.geometry.x + mouse.screen.size.width - panel.width - configDialog.width);
                configDialog.x = x;
                panel.distance = Math.max(mouse.screen.size.width - (x - mouse.screen.geometry.x) - panel.width - configDialog.width, 0);
                break;
            case PlasmaCore.Types.BottomEdge:
            default:
                var y = Math.min(mouse.screenY - mapToItem(dialogRoot, 0, startMouseY).y, mouse.screen.geometry.y + mouse.screen.size.height - panel.height - configDialog.height);
                configDialog.y = y;
                panel.distance = Math.max(mouse.screen.size.height - (y - mouse.screen.geometry.y) - panel.height - configDialog.height, 0);
            }

            lastX = mouse.screenX
            lastY = mouse.screenY
        }
        onReleased: {
            panel.distance = 0
            panelResetAnimation.running = true
        }
    }
}
