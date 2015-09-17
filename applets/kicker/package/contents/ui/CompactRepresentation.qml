/***************************************************************************
 *   Copyright (C) 2013-2014 by Eike Hein <hein@kde.org>                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

import QtQuick 2.0
import QtQuick.Layouts 1.1

import org.kde.plasma.core 2.0 as PlasmaCore

Item {
    id: root

    property bool vertical: (plasmoid.formFactor == PlasmaCore.Types.Vertical)
    property QtObject dashWindow: null

    PlasmaCore.IconItem {
        id: buttonIcon

        anchors.fill: parent

        visible: !(plasmoid.configuration.useCustomButtonImage && plasmoid.configuration.customButtonImage)

        source: plasmoid.configuration.icon
        active: mouseArea.containsMouse

        onWidthChanged: {
            if (vertical && visible) {
                root.Layout.minimumWidth = units.iconSizes.small;
                root.Layout.minimumHeight = parent.width;
            }
        }

        onHeightChanged: {
            if (!vertical && visible) {
                root.Layout.minimumWidth = parent.height;
                root.Layout.minimumHeight = units.iconSizes.small;
            }
        }

        onVisibleChanged: {
            if (visible) {
                if (vertical) {
                    root.Layout.minimumWidth = units.iconSizes.small;
                    root.Layout.minimumHeight = units.iconSizes.small;
                } else {
                    root.Layout.minimumWidth = parent.height;
                    root.Layout.minimumHeight = units.iconSizes.small;
                }
            }
        }
    }

    Image {
        id: buttonImage

        width: vertical ? parent.width : undefined
        height: vertical ? undefined : parent.height

        onPaintedWidthChanged: {
            if (!vertical && visible) {
                root.Layout.minimumWidth = paintedWidth;
                root.Layout.minimumHeight = units.iconSizes.small;
            }
        }

        onPaintedHeightChanged: {
            if (vertical && visible) {
                root.Layout.minimumWidth = units.iconSizes.small;
                root.Layout.minimumHeight = paintedHeight;
            }
        }

        onVisibleChanged: {
            if (visible) {
                if (vertical) {
                    root.Layout.minimumWidth = units.iconSizes.small;
                    root.Layout.minimumHeight = paintedHeight;
                } else {
                    root.Layout.minimumWidth = paintedWidth;
                    root.Layout.minimumHeight = units.iconSizes.small;
                }
            }
        }

        visible: plasmoid.configuration.useCustomButtonImage && plasmoid.configuration.customButtonImage
        source: plasmoid.configuration.customButtonImage
        fillMode: Image.PreserveAspectFit
        smooth: true
    }

    MouseArea
    {
        id: mouseArea
        property bool wasExpanded: false;

        anchors.fill: parent

        hoverEnabled: true

        onPressed: {
            if (!isDash) {
                wasExpanded = plasmoid.expanded
            }
        }

        onClicked: {
            if (isDash) {
                dashWindow.toggle();
                buttonIcon.active = false;
                justOpenedTimer.start();
            } else {
                plasmoid.expanded = !wasExpanded;
            }
        }
    }

    Component.onCompleted: {
        if (isDash) {
            dashWindow = Qt.createQmlObject("DashboardRepresentation {}", root);
            plasmoid.activated.connect(function() {
                dashWindow.toggle()
                justOpenedTimer.start()
            })
        }
    }
}
