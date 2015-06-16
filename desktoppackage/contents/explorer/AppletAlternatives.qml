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

        property string currentPlugin
        // we don't want a binding here, just set it to the current plugin once
        Component.onCompleted: currentPlugin = alternativesHelper.currentPlugin

        QtControls.Action {
            shortcut: "Escape"
            onTriggered: dialog.close()
        }
        QtControls.Action {
            shortcut: "Return"
            onTriggered: switchButton.clicked(null)
        }
        QtControls.Action {
            shortcut: "Enter"
            onTriggered: switchButton.clicked(null)
        }

        QtControls.Action {
            shortcut: "Up"
            onTriggered: mainList.decrementCurrentIndex()
        }
        QtControls.Action {
            shortcut: "Down"
            onTriggered: mainList.incrementCurrentIndex()
        }

        WidgetExplorer {
            id: widgetExplorer
            provides: alternativesHelper.appletProvides
        }

        PlasmaExtras.Heading {
            id: heading
            text: i18nd("plasma_shell_org.kde.plasma.desktop", "Alternative Widgets");
        }

        // HACK for some reason initially setting the index does not work
        // I tried setting it in Component.onCompleted of either delegate and list
        // but that did not help either, hence the Timer as a last resort
        Timer {
            id: setCurrentIndexTimer
            property int desiredIndex: 0
            interval: 0
            onTriggered: mainList.currentIndex = desiredIndex
        }

        PlasmaExtras.ScrollArea {
            id: scrollArea
            Layout.fillWidth: true
            Layout.fillHeight: true

            Layout.preferredHeight: mainList.height

            ListView {
                id: mainList
                model: widgetExplorer.widgetsModel
                boundsBehavior: Flickable.StopAtBounds
                highlight: PlasmaComponents.Highlight {
                    id: highlight
                }
                highlightMoveDuration : 0

                delegate: PlasmaComponents.ListItem {
                    enabled: true
                    onClicked: mainList.currentIndex = index

                    Component.onCompleted: {
                        if (model.pluginName === alternativesHelper.currentPlugin) {
                            setCurrentIndexTimer.desiredIndex = index
                            setCurrentIndexTimer.restart()
                        }
                    }

                    Connections {
                        target: mainList
                        onCurrentIndexChanged: {
                            if (mainList.currentIndex === index) {
                                root.currentPlugin = model.pluginName
                            }
                        }
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
                id: switchButton
                enabled: root.currentPlugin != alternativesHelper.currentPlugin
                Layout.fillWidth: true
                text: i18nd("plasma_shell_org.kde.plasma.desktop", "Switch");
                onClicked: {
                    if (enabled) {
                        alternativesHelper.loadAlternative(root.currentPlugin);
                        dialog.close();
                    }
                }
            }
            PlasmaComponents.Button {
                id: cancelButton
                Layout.fillWidth: true
                text: i18nd("plasma_shell_org.kde.plasma.desktop", "Cancel");
                onClicked: {
                    dialog.close();
                }
            }
        }
    }
}
