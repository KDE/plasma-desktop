/*
    SPDX-FileCopyrightText: 2014 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.0
import QtQuick.Controls 2.5 as QQC2
import QtQuick.Layouts 1.1
import QtQuick.Window 2.1
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 3.0 as PlasmaComponents3
import org.kde.plasma.extras 2.0 as PlasmaExtras

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

        Layout.minimumWidth: PlasmaCore.Units.gridUnit * 20
        Layout.minimumHeight: Math.min(Screen.height - PlasmaCore.Units.gridUnit * 10, heading.height + buttonsRow.height + mainList.contentHeight + PlasmaCore.Units.gridUnit)

        LayoutMirroring.enabled: Qt.application.layoutDirection === Qt.RightToLeft
        LayoutMirroring.childrenInherit: true

        property string currentPlugin
        // we don't want a binding here, just set it to the current plugin once
        Component.onCompleted: currentPlugin = alternativesHelper.currentPlugin

        QQC2.Action {
            shortcut: "Escape"
            onTriggered: dialog.close()
        }
        QQC2.Action {
            shortcut: "Return"
            onTriggered: switchButton.clicked(null)
        }
        QQC2.Action {
            shortcut: "Enter"
            onTriggered: switchButton.clicked(null)
        }

        WidgetExplorer {
            id: widgetExplorer
            provides: alternativesHelper.appletProvides
        }

        PlasmaExtras.PlasmoidHeading {
            PlasmaExtras.Heading {
                id: heading
                text: i18nd("plasma_shell_org.kde.plasma.desktop", "Alternative Widgets");
            }
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
            Layout.fillWidth: true
            Layout.fillHeight: true

            Layout.preferredHeight: mainList.height

            focus: true

            ListView {
                id: mainList

                focus: dialog.visible
                model: widgetExplorer.widgetsModel
                boundsBehavior: Flickable.StopAtBounds
                highlight: PlasmaExtras.Highlight {
                    id: highlight
                }
                highlightMoveDuration : 0
                highlightResizeDuration: 0

                KeyNavigation.down: switchButton.enabled ? switchButton : cancelButton

                delegate: PlasmaExtras.ListItem {
                    implicitHeight: contentLayout.implicitHeight + PlasmaCore.Units.smallSpacing * 2

                    onClicked: mainList.currentIndex = index

                    Component.onCompleted: {
                        if (model.pluginName === alternativesHelper.currentPlugin) {
                            setCurrentIndexTimer.desiredIndex = index
                            setCurrentIndexTimer.restart()
                        }
                    }

                    Connections {
                        target: mainList
                        function onCurrentIndexChanged() {
                            if (mainList.currentIndex === index) {
                                root.currentPlugin = model.pluginName
                            }
                        }
                    }

                    contentItem: RowLayout {
                        id: contentLayout
                        spacing: PlasmaCore.Units.largeSpacing

                        PlasmaCore.IconItem {
                            implicitWidth: PlasmaCore.Units.iconSizes.huge
                            implicitHeight: PlasmaCore.Units.iconSizes.huge
                            source: model.decoration
                        }

                        ColumnLayout {
                            Layout.fillHeight: true
                            Layout.fillWidth: true
                            spacing: 0 // The labels bring their own bottom margins

                            PlasmaExtras.Heading {
                                level: 4
                                Layout.fillWidth: true
                                text: model.name
                                elide: Text.ElideRight
                            }
                            PlasmaComponents3.Label {
                                Layout.fillWidth: true
                                text: model.description
                                font: PlasmaCore.Theme.smallestFont
                                opacity: 0.6
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
            PlasmaComponents3.Button {
                id: switchButton
                enabled: root.currentPlugin !== alternativesHelper.currentPlugin
                Layout.fillWidth: true
                text: i18nd("plasma_shell_org.kde.plasma.desktop", "Switch");

                KeyNavigation.up: mainList
                KeyNavigation.right: cancelButton

                onClicked: {
                    if (enabled) {
                        alternativesHelper.loadAlternative(root.currentPlugin);
                        dialog.close();
                    }
                }
            }
            PlasmaComponents3.Button {
                id: cancelButton

                Layout.fillWidth: true
                text: i18nd("plasma_shell_org.kde.plasma.desktop", "Cancel");

                KeyNavigation.up: mainList

                onClicked: {
                    dialog.close();
                }
            }
        }
    }
}
