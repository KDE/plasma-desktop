/*
 *  Copyright 2014 Marco Martin <mart@kde.org>
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
import QtQuick.Controls 1.0 as QtControls
import QtQuick.Layouts 1.1
import QtQuick.Window 2.1
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.kquickcontrolsaddons 2.0

import org.kde.plasma.private.shell 2.0

ColumnLayout {
    id: root

    signal configurationChanged
    Layout.minimumWidth: units.gridUnit * 20

    property string currentPlugin: alternativesDialog.currentPlugin

    WidgetExplorer {
        id: widgetExplorer
        provides: alternativesDialog.appletProvides
    }

    PlasmaExtras.Heading {
        text: i18nd("org.kde.plasma.desktop", "Alternatives");
    }
    PlasmaExtras.ScrollArea {
        id: scrollArea
        Layout.fillWidth: true
        Layout.fillHeight: true
        Layout.minimumHeight: Math.min(Screen.height - units.gridUnit * 10, mainColumn.height +  units.gridUnit)
        Layout.preferredHeight: mainColumn.height
        
        PlasmaComponents.ButtonColumn {
            id: mainColumn
            //FIXME: why -10 to hide the scrollbar? some problem here
            width: scrollArea.flickableItem.width - 10

            Repeater {
                model: widgetExplorer.widgetsModel
                MouseArea {
                    width: mainColumn.width
                    height: childrenRect.height
                    onClicked: radio.checked = true;
                    property alias checked: radio.checked
                    RowLayout {
                        spacing: units.largeSpacing
                        width: mainColumn.width
                        height: units.iconSizes.huge + units.largeSpacing
                        PlasmaComponents.RadioButton {
                            id: radio
                            Layout.maximumWidth: height
                            Layout.minimumWidth: height
                            checked: model.pluginName == alternativesDialog.currentPlugin
                            onCheckedChanged: {
                                if (checked) {
                                    currentPlugin = model.pluginName
                                }
                                configurationChanged();
                            }
                        }
                        QIconItem {
                            width: units.iconSizes.huge
                            height: width
                            icon: model.decoration
                        }

                        ColumnLayout {
                            Layout.fillWidth: true
                            PlasmaExtras.Heading {
                                level: 4
                                Layout.fillWidth: true
                                text: model.name
                            }
                            PlasmaComponents.Label {
                                Layout.fillWidth: true
                                text: model.description
                                font.pointSize: theme.smallestFont.pointSize
                                wrapMode: Text.WordWrap
                            }
                        }
                    }
                }
            }
        }
    }
    RowLayout {
        Layout.fillWidth: true
        PlasmaComponents.Button {
            enabled: root.currentPlugin != alternativesDialog.currentPlugin
            Layout.fillWidth: true
            text: i18nd("org.kde.plasma.desktop", "Switch");
            onClicked: alternativesDialog.loadAlternative(root.currentPlugin);
        }
        PlasmaComponents.Button {
            Layout.fillWidth: true
            text: i18nd("org.kde.plasma.desktop", "Cancel");
            onClicked: alternativesDialog.visible = false;
        }
    }
}
