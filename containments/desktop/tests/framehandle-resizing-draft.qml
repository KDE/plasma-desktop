/*
 *   Copyright 2013 Sebastian Kügler <sebas@kde.org>
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

import QtQuick 1.1
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.kquickcontrolsaddons 0.1 as KQuickControlsAddons

Rectangle {

    property bool showAppletHandle: mouseListener.containsMouse

    width: 600
    height: 600
    color: "black"
    anchors.fill: parent

    property int w: 30

    Rectangle {
        id: applet
        color: "blue"
        x: 100
        y: 60
        width: 400
        height: 400

        KQuickControlsAddons.MouseEventListener {
            /* The MouseEventListener
             *
             * this is the parent of the applet and handle,
             * it's wider than it's alloted position, however,
             * and unclipped. It carries the whole area that
             * is hoverable.
             */
            id: mouseListener
            anchors { left: parent.left; top: parent.top; bottom: parent.bottom; }
            width: parent.width+w;
            hoverEnabled: true

            onContainsMouseChanged: print("Mouse is " + containsMouse);

            PlasmaCore.FrameSvgItem {
                id: backgroundFrame
                /* The applet+handle frame
                 *
                 * This is the growing frame containing the applet
                 * and handle.
                 *
                 */
                imagePath: "widgets/background"
                z: parent.z+1
                anchors { left: parent.left; top: parent.top; bottom: parent.bottom; }
                width: showAppletHandle ? parent.width: parent.width-w;
                Behavior on width { PropertyAnimation {}}

                // DragArea goes here

                // Handle itself goes here
            }
            Rectangle {
                /* The Applet
                 *
                 * Sits on top of the frame, but doesn't resize with it,
                 * only with the whole allotted area.
                 *
                 * As it's a child of the MouseEventListener, the applet
                 * still receives all input.
                 */
                id: yellow
                color: "green"
                z: parent.z+1
                x: 0 + backgroundFrame.margins.left
                y: 0 + backgroundFrame.margins.top
                width: applet.width - (backgroundFrame.margins.left + backgroundFrame.margins.right)
                height: applet.width - (backgroundFrame.margins.top + backgroundFrame.margins.bottom)
                MouseArea {
                    anchors.fill: parent
                    onClicked: print("Applet clicked.");
                }
            }
        }
    }
    onShowAppletHandleChanged: print("Active? " + showAppletHandle);
}
