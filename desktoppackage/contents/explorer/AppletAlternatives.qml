/*
    SPDX-FileCopyrightText: 2014 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Controls 2.5 as QQC2
import QtQuick.Layouts 1.1
import QtQuick.Window 2.1
import org.kde.plasma.core as PlasmaCore
import org.kde.plasma.components 3.0 as PlasmaComponents3
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.plasma.plasmoid
import org.kde.kirigami 2.20 as Kirigami

import org.kde.plasma.private.shell 2.0

PlasmaCore.Dialog {
    id: dialog
    visualParent: alternativesHelper.applet
    location: alternativesHelper.applet.Plasmoid.location
    hideOnWindowDeactivate: true
    backgroundHints: (alternativesHelper.applet.Plasmoid.containmentDisplayHints & PlasmaCore.Types.ContainmentPrefersOpaqueBackground) ? PlasmaCore.Dialog.SolidBackground : PlasmaCore.Dialog.StandardBackground

    Component.onCompleted: {
        flags = flags |  Qt.WindowStaysOnTopHint;
        dialog.show();
    }

    ColumnLayout {
        id: root

        signal configurationChanged

        Layout.minimumWidth: Kirigami.Units.gridUnit * 20
        Layout.minimumHeight: Math.min(Screen.height - Kirigami.Units.gridUnit * 10, implicitHeight)

        LayoutMirroring.enabled: Qt.application.layoutDirection === Qt.RightToLeft
        LayoutMirroring.childrenInherit: true

        property string currentPlugin: ""

        Shortcut {
            sequence: "Escape"
            onActivated: dialog.close()
        }
        Shortcut {
            sequence: "Return"
            onActivated: root.savePluginAndClose()
        }
        Shortcut {
            sequence: "Enter"
            onActivated: root.savePluginAndClose()
        }


        WidgetExplorer {
            id: widgetExplorer
            provides: alternativesHelper.appletProvides
        }

        PlasmaExtras.PlasmoidHeading {
            Kirigami.Heading {
                id: heading
                text: i18nd("plasma_shell_org.kde.plasma.desktop", "Alternative Widgets")
                textFormat: Text.PlainText
            }
        }

        // This timer checks with a short delay whether a new item in the list has been hovered by the cursor.
        // If not, then the cursor has left the view and thus no item should be selected.
        Timer {
            id: resetCurrentIndex
            property string oldPlugin
            interval: 100
            onTriggered: {
                if (root.currentPlugin === oldPlugin) {
                    mainList.currentIndex = -1
                    root.currentPlugin = ""
                }
            }
        }

        function savePluginAndClose() {
            alternativesHelper.loadAlternative(currentPlugin);
            dialog.close();
        }

        PlasmaComponents3.ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true

            Layout.preferredHeight: mainList.contentHeight

            focus: true

            ListView {
                id: mainList

                focus: dialog.visible
                model: widgetExplorer.widgetsModel
                boundsBehavior: Flickable.StopAtBounds
                highlight: PlasmaExtras.Highlight {
                    pressed: mainList.currentItem && mainList.currentItem.pressed
                }
                highlightMoveDuration : 0
                highlightResizeDuration: 0

                height: contentHeight+Kirigami.Units.smallSpacing

                delegate: PlasmaComponents3.ItemDelegate {
                    id: listItem

                    implicitHeight: contentLayout.implicitHeight + Kirigami.Units.smallSpacing * 2
                    width: ListView.view.width

                    onHoveredChanged: {
                        if (hovered) {
                            resetCurrentIndex.stop()
                            mainList.currentIndex = index
                        } else {
                            resetCurrentIndex.oldPlugin = model.pluginName
                            resetCurrentIndex.restart()
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

                    onClicked: root.savePluginAndClose()

                    Component.onCompleted: {
                        if (model.pluginName === alternativesHelper.currentPlugin) {
                            root.currentPlugin = model.pluginName
                            setAsCurrent.restart()
                        }
                    }

                    // we don't want to select any entry by default
                    // this cannot be set in Component.onCompleted
                    Timer {
                        id: setAsCurrent
                        interval: 100
                        onTriggered: {
                            mainList.currentIndex = index
                        }
                    }

                    contentItem: RowLayout {
                        id: contentLayout
                        spacing: Kirigami.Units.largeSpacing

                        Kirigami.Icon {
                            implicitWidth: Kirigami.Units.iconSizes.huge
                            implicitHeight: Kirigami.Units.iconSizes.huge
                            source: model.decoration
                        }

                        ColumnLayout {
                            id: labelLayout

                            readonly property color textColor: listItem.pressed ? Kirigami.Theme.highlightedTextColor : Kirigami.Theme.textColor

                            Layout.fillHeight: true
                            Layout.fillWidth: true
                            spacing: 0 // The labels bring their own bottom margins

                            Kirigami.Heading {
                                level: 4
                                Layout.fillWidth: true
                                text: model.name
                                textFormat: Text.PlainText
                                elide: Text.ElideRight
                                type: model.pluginName === alternativesHelper.currentPlugin ? PlasmaExtras.Heading.Type.Primary : PlasmaExtras.Heading.Type.Normal
                                color: labelLayout.textColor
                            }

                            PlasmaComponents3.Label {
                                Layout.fillWidth: true
                                text: model.description
                                textFormat: Text.PlainText
                                font.pointSize: Kirigami.Theme.smallFont.pointSize
                                font.family: Kirigami.Theme.smallFont.family
                                font.bold: model.pluginName === alternativesHelper.currentPlugin
                                opacity: 0.6
                                maximumLineCount: 2
                                wrapMode: Text.WordWrap
                                elide: Text.ElideRight
                                color: labelLayout.textColor
                            }
                        }
                    }
                }
            }
        }
    }
}
