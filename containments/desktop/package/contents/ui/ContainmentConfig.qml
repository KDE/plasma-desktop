/*
 *   Copyright 2011-2013 Sebastian Kügler <sebas@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2 or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Library General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

import QtQuick 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.kquickcontrolsaddons 2.0 as KQuickControlsAddons
import "plasmapackage:/code/LayoutManager.js" as LayoutManager

Item {
    id: containmentConfig

    width: 400
    height: 320

    property bool open: false

    PlasmaComponents.ToolButton {
        id: cfgbutton

        width: root.iconSize
        height: width

        iconSource: "configure"
        onClicked: open = !open;
    }

    PlasmaCore.FrameSvgItem {
        id: configFrame
        anchors.fill: parent
        imagePath: "widgets/background"
        opacity: open ? 1 : 0
        Behavior on opacity { PropertyAnimation {} }

        Row {
            anchors.fill: parent
            anchors.margins: parent.margins.top

            spacing: configFrame.margins.top/2

            Column {
                width: parent.width / 2 - (configFrame.margins.top)
                spacing: configFrame.margins.top/2
                PlasmaComponents.Label {
                    width: parent.width
                    horizontalAlignment: Text.AlignRight
                    text: "Visual debugging:"
                }
                PlasmaComponents.Label {
                    width: parent.width
                    horizontalAlignment: Text.AlignRight
                    text: "Icon size:"
                }
                PlasmaComponents.Label {
                    width: parent.width
                    horizontalAlignment: Text.AlignRight
                    text: "Applethandle delay:"
                }
                PlasmaComponents.Label {
                    width: parent.width
                    horizontalAlignment: Text.AlignRight
                    text: "Halo move duration:"
                }
                PlasmaComponents.Label {
                    width: parent.width
                    horizontalAlignment: Text.AlignRight
                    text: "Halo opacity:"
                }
            }

            Column {
                spacing: configFrame.margins.top/2
                PlasmaComponents.CheckBox {
                    checked: root.debug
                    onCheckedChanged: {
                        print("Debug is now " + checked);
                        root.debug = checked;
                    }
                }
                PlasmaComponents.TextField {
                    text: root.iconSize
                    onTextChanged: {
                        print("Set icon size to " + text);
                        root.iconSize = text;
                    }
                }
                PlasmaComponents.TextField {
                    text: root.handleDelay
                    onTextChanged: {
                        print("Set handle delay to " + text);
                        root.handleDelay = text;
                    }
                }
                PlasmaComponents.TextField {
                    text: placeHolderPaint.moveDuration
                    onTextChanged: {
                        print("Set move duration to " + text);
                        placeHolderPaint.moveDuration = text;
                    }
                }
                PlasmaComponents.TextField {
                    text: root.haloOpacity
                    onTextChanged: {
                        print("Set halo opacity to " + text);
                        root.haloOpacity = text;
                    }
                }
            }
        }
    }
}
