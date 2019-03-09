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
import QtQuick.Controls 2.0 as QQC2
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.configuration 2.0
import org.kde.kquickcontrolsaddons 2.0 as KQuickControlsAddons

PlasmaComponents.Button {
    readonly property string textLabel: panel.location === PlasmaCore.Types.LeftEdge || panel.location === PlasmaCore.Types.RightEdge ? i18nd("plasma_shell_org.kde.plasma.desktop", "Width") : i18nd("plasma_shell_org.kde.plasma.desktop", "Height")
    text: panelResizeHintTimer.running ? panel.thickness : textLabel

    readonly property string sizeIcon: panel.location === PlasmaCore.Types.LeftEdge || panel.location === PlasmaCore.Types.RightEdge ? "resizecol" : "resizerow"
    iconSource: sizeIcon

    checkable: true
    checked: mel.pressed

    Timer {
        id: panelResizeHintTimer
        interval: 1000
    }

    Connections {
        target: panel
        onThicknessChanged: panelResizeHintTimer.restart()
    }

    QQC2.ToolTip {
        id: tooltip
        visible: false
        delay: 0
        timeout: 10000
        contentItem: PlasmaComponents.Label {
            anchors.fill: parent
            text: i18nd("plasma_shell_org.kde.plasma.desktop", "Click and drag the button to resize the panel.")
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            wrapMode: Text.WordWrap
            color: tooltip.palette.toolTipText
        }

        KQuickControlsAddons.MouseEventListener {
            anchors.fill: parent
            onPressed: tooltip.visible = false
        }
    }

    KQuickControlsAddons.MouseEventListener {
        id: mel
        anchors.fill: parent
        cursorShape: panel.location === PlasmaCore.Types.LeftEdge || panel.location === PlasmaCore.Types.RightEdge ? Qt.SizeHorCursor : Qt.SizeVerCursor

        property int startMouseX
        property int startMouseY
        onPressed: {
            dialogRoot.closeContextMenu();
            startMouseX = mouse.x
            startMouseY = mouse.y
            tooltip.visible = true
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

        property int wheelDelta: 0
        onWheelMoved: {
            wheelDelta += wheel.delta;
            var deltaThickness = 0;
            // Magic number 120 for common "one click"
            // See: http://qt-project.org/doc/qt-5/qml-qtquick-wheelevent.html#angleDelta-prop
            while (wheelDelta >= 120) {
                wheelDelta -= 120;
                deltaThickness += 1;
            }
            while (wheelDelta <= -120) {
                wheelDelta += 120;
                deltaThickness -= 1;
            }
            deltaThickness = deltaThickness * 2;
            var newThickness = Math.max(units.gridUnit, panel.thickness + deltaThickness);
            if (panel.location === PlasmaCore.Types.LeftEdge || panel.location === PlasmaCore.Types.RightEdge) {
                newThickness = Math.min(newThickness, panel.screenToFollow.geometry.width/2);
            } else {
                newThickness = Math.min(newThickness, panel.screenToFollow.geometry.height/2);
            }
            if (newThickness % 2 != 0) {
                newThickness -= 1;
            }
            panel.thickness = newThickness;
            panelResetAnimation.running = true;
        }
    }
}
