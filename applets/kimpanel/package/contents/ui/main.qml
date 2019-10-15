/*
 *  Copyright 2014 Weng Xuetian <wengxt@gmail.com>
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
import QtQuick.Layouts 1.1
import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.kquickcontrolsaddons 2.0

Item {
    id: root

    property bool vertical: plasmoid.formFactor === PlasmaCore.Types.Vertical

    LayoutMirroring.enabled: !vertical && Qt.application.layoutDirection === Qt.RightToLeft
    LayoutMirroring.childrenInherit: true

    Layout.minimumWidth: vertical ? units.iconSizes.small : items.implicitWidth
    Layout.minimumHeight: !vertical ? units.iconSizes.small : items.implicitHeight
    Layout.preferredHeight: Layout.minimumHeight
    Plasmoid.preferredRepresentation: Plasmoid.fullRepresentation

    InputPanel { }
    Text {
        id: textMetric
        visible: false
        // translated but not used, we just need length/height
        text: i18n("Arbitrary String Which Says Something")
    }

    Plasmoid.fullRepresentation: Item {
        id: dialogItem
        Layout.minimumWidth: units.gridUnit * 12
        Layout.minimumHeight: units.gridUnit * 12

        ListView {
            id: view
            anchors.fill: parent
            focus: true

            model: dataSource.data["statusbar"]["LayoutList"]

            delegate: Item {
                id: listdelegate
                height: textMetric.paintedHeight * 2

                anchors {
                    left: parent.left
                    right: parent.right
                }

                PlasmaCore.IconItem {
                    id: icon
                    source: "checkmark"
                    height: parent.height
                    width: height
                    visible: index == dataSource.data["statusbar"]["CurrentLayoutIndex"]
                }

                PlasmaComponents.Label {
                    text: model.description
                    anchors {
                        verticalCenter: parent.verticalCenter
                        left: icon.right
                        right: parent.right
                        leftMargin: 10
                        rightMargin: 10
                    }

                    verticalAlignment: Text.AlignVCenter
                    elide: Text.ElideRight
                }

                MouseArea {
                    height: parent.height + 15
                    anchors { left: parent.left; right: parent.right;}
                    hoverEnabled: true

                    onClicked: {
                        dataSource.data["statusbar"]["LayoutModels"].switchLayout(index);
                    }

                    onEntered: {
                        view.currentIndex = index;
                    }
                }
            }

            highlight: PlasmaComponents.Highlight {
                hover: true
            }

            highlightMoveDuration: 250
            highlightMoveVelocity: 2
        }
    }

    Plasmoid.compactRepresentation: CompactRepresentation {
        dataEngine: dataSource
    }

    property int visibility: PlasmaCore.Types.HiddenStatus
    Plasmoid.status: visibility

    PlasmaCore.DataSource {
        id: dataSource
        engine: "kimpanel"
        connectedSources: ["statusbar"]
        onDataChanged: {
            root.visibility = dataSource.data["statusbar"]["Visibility"]?
                        PlasmaCore.Types.ActiveStatus : PlasmaCore.Types.HiddenStatus;
        }
    }
}

