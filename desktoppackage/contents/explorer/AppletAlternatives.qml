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

PlasmaCore.Dialog {
    id: dialog
    visualParent: alternativesHelper.applet
    location: alternativesHelper.applet.location

    Component.onCompleted: {
        flags = flags |  Qt.WindowStaysOnTopHint;
        dialog.show();
    }

    ColumnLayout {
        id: root

        signal configurationChanged

        Layout.minimumWidth: units.gridUnit * 20
        Layout.minimumHeight: Math.min(Screen.height - units.gridUnit * 10, heading.height + buttonsRow.height + mainList.contentHeight + units.gridUnit)

        property string currentPlugin: alternativesHelper.currentPlugin

        WidgetExplorer {
            id: widgetExplorer
            provides: alternativesHelper.appletProvides
        }

        PlasmaExtras.Heading {
            id: heading
            text: i18nd("plasma_shell_org.kde.plasma.desktop", "Alternatives");
        }

        PlasmaExtras.ScrollArea {
            id: scrollArea
            Layout.fillWidth: true
            Layout.fillHeight: true

            Layout.preferredHeight: mainList.height

            ListView {
                id: mainList
                model: widgetExplorer.widgetsModel
                highlight: PlasmaComponents.Highlight {
                    id: highlight
                }
                highlightMoveDuration : 0
                delegate: PlasmaComponents.ListItem {
                    enabled: true
                    onClicked: checked = true;
                    property bool checked: model.pluginName == alternativesHelper.currentPlugin
                    onCheckedChanged: {
                        if (checked) {
                            root.currentPlugin = model.pluginName;
                            mainList.currentIndex = index;
                        }
                    }
                    Connections {
                        target: mainList
                        onCurrentIndexChanged: checked = false;
                    }
                    RowLayout {
                        x: 2 * units.smallSpacing
                        width: parent.width - 2 * x
                        spacing: units.largeSpacing
                        QIconItem {
                            width: units.iconSizes.huge
                            height: width
                            icon: model.decoration
                        }

                        ColumnLayout {
                            height: parent.height
                            Layout.fillWidth: true
                            PlasmaExtras.Heading {
                                level: 4
                                Layout.fillWidth: true
                                text: model.name
                                wrapMode: Text.NoWrap
                                elide: Text.ElideRight
                            }
                            PlasmaComponents.Label {
                                Layout.fillWidth: true
                                text: model.description
                                font.pointSize: theme.smallestFont.pointSize
                                maximumLineCount: 2
                                wrapMode: Text.WordWrap
                                elide: Text.ElideRight
                            }
                        }
                    }
                }
            }
        }
        RowLayout {
            id: buttonsRow

            Layout.fillWidth: true
            PlasmaComponents.Button {
                enabled: root.currentPlugin != alternativesHelper.currentPlugin
                Layout.fillWidth: true
                text: i18nd("plasma_shell_org.kde.plasma.desktop", "Switch");
                onClicked: {
                    alternativesHelper.loadAlternative(root.currentPlugin);
                    dialog.close();
                }
            }
            PlasmaComponents.Button {
                Layout.fillWidth: true
                text: i18nd("plasma_shell_org.kde.plasma.desktop", "Cancel");
                onClicked: {
                    dialog.close();
                }
            }
        }
    }
}
