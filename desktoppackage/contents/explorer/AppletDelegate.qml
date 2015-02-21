/*
 *   Copyright 2011 Marco Martin <mart@kde.org>
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

import QtQuick 2.0
import QtQuick.Layouts 1.1

import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.draganddrop 2.0
import org.kde.kquickcontrolsaddons 2.0

Item {
    id: delegate

    readonly property string pluginName: model.pluginName

    width: list.width
    height: iconWidget.height + units.largeSpacing

    DragArea {
        anchors.fill: parent
        supportedActions: Qt.MoveAction | Qt.LinkAction
        //onDragStarted: tooltipDialog.visible = false
        delegateImage: decoration
        mimeData {
            source: parent
        }
        Component.onCompleted: mimeData.setData("text/x-plasmoidservicename", pluginName)

        onDragStarted: {
            main.preventWindowHide = true;
        }
        onDrop: {
            main.preventWindowHide = false;
        }

        RowLayout {
            anchors {
                fill: parent
                margins: units.smallSpacing
                rightMargin: units.smallSpacing * 2 // don't cram the text to the border too much
            }
            spacing: units.largeSpacing

            QIconItem {
                id: iconWidget
                width: units.iconSizes.huge
                height: width
                icon: model.decoration

                QIconItem {
                    anchors {
                        top: parent.top
                        left: parent.left
                        margins: units.smallSpacing
                    }
                    icon: visible ? "dialog-ok-apply" : undefined
                    visible: running
                    width: units.iconSizes.small
                    height: width
                }
            }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: units.smallSpacing

                PlasmaExtras.Heading {
                    id: heading
                    Layout.fillWidth: true
                    level: 4
                    text: model.name
                    elide: Text.ElideRight
                    wrapMode: Text.WordWrap
                    maximumLineCount: 2
                    lineHeight: 0.95
                }
                PlasmaComponents.Label {
                    Layout.fillWidth: true
                    // otherwise causes binding loop due to the way the Plasma sets the height
                    height: implicitHeight
                    text: model.description
                    font.pointSize: theme.smallestFont.pointSize
                    wrapMode: Text.WordWrap
                    elide: Text.ElideRight
                    maximumLineCount: heading.lineCount === 1 ? 3 : 2
                }
            }
        }

        MouseArea {
            id: mouseArea
            anchors.fill: parent
            hoverEnabled: true
            onDoubleClicked: widgetExplorer.addApplet(pluginName)
            onEntered: delegate.ListView.view.currentIndex = index
            onExited: delegate.ListView.view.currentIndex = -1
        }
    }
}
