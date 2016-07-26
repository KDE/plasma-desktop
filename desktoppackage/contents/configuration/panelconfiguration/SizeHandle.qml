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
    text: panel.location == PlasmaCore.Types.LeftEdge || panel.location == PlasmaCore.Types.RightEdge ? i18nd("plasma_shell_org.kde.plasma.desktop", "Width") : i18nd("plasma_shell_org.kde.plasma.desktop", "Height")

    checkable: true
    checked: mel.pressed

    KQuickControlsAddons.MouseEventListener {
        id: mel
        anchors.fill: parent
        cursorShape: panel.location == PlasmaCore.Types.LeftEdge || panel.location == PlasmaCore.Types.RightEdge ? Qt.SizeHorCursor : Qt.SizeVerCursor

        property int startMouseX
        property int startMouseY
        onPressed: {
            dialogRoot.closeContextMenu();
            startMouseX = mouse.x
            startMouseY = mouse.y
        }
        onPositionChanged: {
            switch (panel.location) {
            case PlasmaCore.Types.TopEdge:
                var y = Math.min(panel.screenToFollow.geometry.y + panel.screenToFollow.geometry.height/2, mouse.screenY - mapToItem(dialogRoot, 0, startMouseY).y);
                var thickness = Math.max(units.gridUnit, y - panel.y);

                if (thickness % 2 != 0) {
                    if (mouse.y > startMouseY) {
                        thickness += 1;
                        y -= 1;
                    } else {
                        thickness -= 1;
                        y += 1;
                    }
                }

                configDialog.y = y;
                panel.thickness = thickness;
                break;
            case PlasmaCore.Types.LeftEdge:
                var x = Math.min(panel.screenToFollow.geometry.x + panel.screenToFollow.geometry.width/2, mouse.screenX - mapToItem(dialogRoot, startMouseX, 0).x);
                var thickness = Math.max(units.gridUnit, x - panel.x);

                if (thickness % 2 != 0) {
                    if (mouse.x > startMouseX) {
                        thickness += 1;
                        x += 1;
                    } else {
                        thickness -= 1;
                        x -= 1;
                    }
                }

                configDialog.x = x;
                panel.thickness = thickness;
                break;
            case PlasmaCore.Types.RightEdge:
                var x = Math.max(panel.screenToFollow.geometry.x + panel.screenToFollow.geometry.width/2, mouse.screenX - mapToItem(dialogRoot, startMouseX, 0).x);
                var thickness = Math.max(units.gridUnit, panel.screenToFollow.geometry.x + panel.screenToFollow.geometry.width - (x + configDialog.width));

                if (thickness % 2 != 0) {
                    if (mouse.x > startMouseX) {
                        thickness -= 1;
                        x += 1;
                    } else {
                        thickness += 1;
                        x -= 1;
                    }
                }

                configDialog.x = x;
                panel.thickness = thickness;
                break;
            case PlasmaCore.Types.BottomEdge:
            default:
                var y = Math.max(panel.screenToFollow.geometry.y + panel.screenToFollow.geometry.height/2, mouse.screenY - mapToItem(dialogRoot, 0, startMouseY).y);
                var thickness = Math.max(units.gridUnit, panel.screenToFollow.geometry.y + panel.screenToFollow.geometry.height - (y + configDialog.height));

                if (thickness % 2 != 0) {
                    if (mouse.y > startMouseY) {
                        thickness -= 1;
                        y += 1;
                    } else {
                        thickness += 1;
                        y -= 1;
                    }
                }

                configDialog.y = y;
                panel.thickness = thickness;
            }
        }
    }
}
