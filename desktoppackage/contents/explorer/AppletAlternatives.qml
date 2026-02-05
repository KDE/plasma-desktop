/*
    SPDX-FileCopyrightText: 2014 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2026 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import QtQuick.Window
import org.kde.plasma.core as PlasmaCore
import org.kde.plasma.components as PlasmaComponents3
import org.kde.plasma.extras as PlasmaExtras
import org.kde.plasma.plasmoid
import org.kde.kirigami as Kirigami

import org.kde.plasma.private.shell
import org.kde.plasma.shell

PlasmaCore.PopupPlasmaWindow {
    id: dialog

    required property AlternativesHelper alternativesHelper

    visualParent: alternativesHelper.applet
    width: root.Layout.preferredWidth + leftPadding + rightPadding
    height: root.Layout.preferredHeight + topPadding + bottomPadding

    onActiveChanged: if (!active) visible = false
    animated: true
    hideOnWindowDeactivate: true

    popupDirection: switch (alternativesHelper.applet.Plasmoid.location) {
        case PlasmaCore.Types.TopEdge:
            return Qt.BottomEdge
        case PlasmaCore.Types.LeftEdge:
            return Qt.RightEdge
        case PlasmaCore.Types.RightEdge:
            return Qt.LeftEdge
        default:
            return Qt.TopEdge
    }
    visible: true

    mainItem: ColumnLayout {
        id: root

        signal configurationChanged

        Layout.preferredWidth: Kirigami.Units.gridUnit * 20
        Layout.preferredHeight: Math.min(Screen.height - Kirigami.Units.gridUnit * 10, implicitHeight)

        LayoutMirroring.enabled: Application.layoutDirection === Qt.RightToLeft
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
            provides: dialog.alternativesHelper.appletProvides
        }

        PlasmaExtras.PlasmoidHeading {
            Kirigami.Heading {
                id: heading
                text: i18ndc("plasma_shell_org.kde.plasma.desktop", "@title:window for widget alternatives explorer", "Alternative Widgets")
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
            dialog.alternativesHelper.loadAlternative(currentPlugin);
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

                    Accessible.name: model.name
                    Accessible.description: model.description

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
                        if (model.pluginName === dialog.alternativesHelper.currentPlugin) {
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
                                type: model.pluginName === dialog.alternativesHelper.currentPlugin ? PlasmaExtras.Heading.Type.Primary : PlasmaExtras.Heading.Type.Normal
                                color: labelLayout.textColor
                            }

                            PlasmaComponents3.Label {
                                Layout.fillWidth: true
                                text: model.description
                                textFormat: Text.PlainText
                                font.pointSize: Kirigami.Theme.smallFont.pointSize
                                font.family: Kirigami.Theme.smallFont.family
                                font.bold: model.pluginName === dialog.alternativesHelper.currentPlugin
                                opacity: 0.75
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
