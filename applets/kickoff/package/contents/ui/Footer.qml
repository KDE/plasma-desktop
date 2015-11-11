/*
 *    Copyright (C) 2012  Gregor Taetzner <gregor@freenet.de>
 *
 *    This program is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License along
 *    with this program; if not, write to the Free Software Foundation, Inc.,
 *    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
import QtQuick 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.kquickcontrolsaddons 2.0

Item {
    id: footer

    height: statusLabel.height

    PlasmaComponents.Label {
            id: statusLabel
            //text: kickoff.footerText // FIXME: where does this come from? Is it still needed?
            anchors {
                verticalCenter: parent.verticalCenter
                left: parent.left
            }
    }

    PlasmaCore.SvgItem {
        id: brandingButton
        svg {
            PlasmaCore.Svg {
                imagePath: "widgets/branding"
            }
        }
        elementId: "brilliant"
        height: Math.min(implicitHeight, parent.height) //don't be bigger than the text or the logo size
        width: implicitWidth * (height / implicitHeight)

        anchors {
            verticalCenter: parent.verticalCenter
            left: statusLabel.right
            leftMargin: 4
        }
    }
}
