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
    KQuickControlsAddons.MouseEventListener {
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
                configDialog.y = Math.min(panel.screen.geometry.y + panel.screen.geometry.height/2, mouse.screenY - mapToItem(dialogRoot, 0, startMouseY).y);
                panel.thickness = Math.max(units.gridUnit, configDialog.y - panel.y);
                break;
            case PlasmaCore.Types.LeftEdge:
                configDialog.x = Math.min(panel.screen.geometry.x + panel.screen.geometry.width/2, mouse.screenX - mapToItem(dialogRoot, startMouseX, 0).x);
                panel.thickness = Math.max(units.gridUnit, configDialog.x - panel.x);
                break;
            case PlasmaCore.Types.RightEdge:
                configDialog.x = Math.max(panel.screen.geometry.x + panel.screen.geometry.width/2, mouse.screenX - mapToItem(dialogRoot, startMouseX, 0).x);
                panel.thickness = Math.max(units.gridUnit, panel.screen.geometry.x + panel.screen.geometry.width - (configDialog.x + configDialog.width));
                break;
            case PlasmaCore.Types.BottomEdge:
            default:
                configDialog.y = Math.max(panel.screen.geometry.y + panel.screen.geometry.height/2, mouse.screenY - mapToItem(dialogRoot, 0, startMouseY).y);
                panel.thickness = Math.max(units.gridUnit, panel.screen.geometry.y + panel.screen.geometry.height - (configDialog.y + configDialog.height));
            }
        }
    }
}
