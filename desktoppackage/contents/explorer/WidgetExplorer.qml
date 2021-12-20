/*
    SPDX-FileCopyrightText: 2011 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Controls 2.5 as QQC2

import org.kde.plasma.components 2.0 as PC2 // for DialogStatus, ModelCOntextMenu, and Highlight
import org.kde.plasma.components 3.0 as PC3
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.kquickcontrolsaddons 2.0
import org.kde.kwindowsystem 1.0

import QtQuick.Window 2.1
import QtQuick.Layouts 1.1

import org.kde.plasma.private.shell 2.0

PC3.Page {
    id: main

    width: contentItem.implicitWidth
    height: 800//Screen.height

    opacity: draggingWidget ? 0.3 : 1

    property QtObject containment
    
    property PlasmaCore.Dialog sidePanel

    //external drop events can cause a raise event causing us to lose focus and
    //therefore get deleted whilst we are still in a drag exec()
    //this is a clue to the owning dialog that hideOnWindowDeactivate should be deleted
    //See https://bugs.kde.org/show_bug.cgi?id=332733
    property bool preventWindowHide: draggingWidget || categoriesDialog.status !== PC2.DialogStatus.Closed
                                  || getWidgetsDialog.status !== PC2.DialogStatus.Closed

    property bool outputOnly: draggingWidget

    property Item categoryButton

    property bool draggingWidget: false

    signal closed()

    onVisibleChanged: {
        if (!visible) {
            kwindowsystem.showingDesktop = false
        }
    }

    Component.onCompleted: {
        if (!root.widgetExplorer) {
            root.widgetExplorer = widgetExplorerComponent.createObject(root)
        }
        root.widgetExplorer.containment = main.containment
    }

    Component.onDestruction: {
        if (pendingUninstallTimer.running) {
            // we're not being destroyed so at least reset the filters
            widgetExplorer.widgetsModel.filterQuery = ""
            widgetExplorer.widgetsModel.filterType = ""
            widgetExplorer.widgetsModel.searchTerm = ""
        } else {
            root.widgetExplorer.destroy()
            root.widgetExplorer = null
        }
    }

    KWindowSystem {
        id: kwindowsystem
    }

    QQC2.Action {
        shortcut: "Escape"
        onTriggered: {
            if (searchInput.length > 0) {
                searchInput.text = ""
            } else {
                main.closed()
            }
        }
    }

    Component {
        id: widgetExplorerComponent

        WidgetExplorer {
            //view: desktop
            onShouldClose: main.closed();
        }
    }

    PC2.ModelContextMenu {
        id: getWidgetsDialog
        visualParent: getWidgetsButton
        placement: PlasmaCore.Types.TopPosedLeftAlignedPopup
        // model set on first invocation
        onClicked: model.trigger()
    }

    header: PlasmaExtras.PlasmoidHeading {
        ColumnLayout {
            id: header
            anchors.fill: parent

            RowLayout {
                PlasmaExtras.Heading {
                    id: heading
                    level: 1
                    text: i18nd("plasma_shell_org.kde.plasma.desktop", "Widgets")
                    elide: Text.ElideRight

                    Layout.fillWidth: true
                }
                PC3.ToolButton {
                    id: closeButton
                    icon.name: "window-close"
                    onClicked: main.closed()
                }
            }

            RowLayout {
                PC3.TextField {
                    id: searchInput
                    Layout.fillWidth: true
                    clearButtonShown: true
                    placeholderText: i18nd("plasma_shell_org.kde.plasma.desktop", "Search…")

                    inputMethodHints: Qt.ImhNoPredictiveText

                    onTextChanged: {
                        list.positionViewAtBeginning()
                        list.currentIndex = -1
                        widgetExplorer.widgetsModel.searchTerm = text
                    }

                    Component.onCompleted: forceActiveFocus()
                }
                PC3.ToolButton {
                    id: getWidgetsButton
                    icon.name: "get-hot-new-stuff"
                    text: i18nd("plasma_shell_org.kde.plasma.desktop", "Get New Widgets…")
                    onClicked: {
                        getWidgetsDialog.model = widgetExplorer.widgetsMenuActions
                        getWidgetsDialog.openRelative()
                    }
                }
            }
        }
    }

    Timer {
        id: setModelTimer
        interval: 20
        running: true
        onTriggered: list.model = widgetExplorer.widgetsModel
    }

    contentItem: RowLayout {
        PC3.ScrollView {
            Layout.fillHeight: true
            implicitWidth: PlasmaCore.Units.gridUnit * 10
            activeFocusOnTab: true

            PC3.ScrollBar.horizontal.policy: PC3.ScrollBar.AlwaysOff

            ListView {
                id: sideView
                model: widgetExplorer.filterModel
                activeFocusOnTab: true
                highlight: PC2.Highlight {}
                highlightMoveDuration: 0
                onCurrentItemChanged: if (currentItem) currentItem.apply()

                delegate: PC2.ListItem {
                    RowLayout {
                        PlasmaExtras.Heading {
                            visible: model.separator === true
                            level: 4
                            text: model.display
                            elide: Text.ElideRight
                            Layout.fillWidth: true
                            Layout.topMargin: PlasmaCore.Units.smallSpacing*2
                        }
                        PC3.Label {
                            visible: !(model.separator === true)
                            text: model.display
                            elide: Text.ElideRight
                            Layout.fillWidth: true
                        }
                    }
                    function apply() {
                        if (model.separator === true)
                            return

                        sideView.currentIndex = model.index
                        widgetExplorer.widgetsModel.filterQuery = model.filterData
                        widgetExplorer.widgetsModel.filterType = model.filterType
                    }
                    TapHandler {
                        onTapped: apply()
                    }
                }
            }
        }
        QQC2.ToolSeparator {
            Layout.fillHeight: true
        }
        PC3.ScrollView {
            Layout.fillHeight: true
            implicitWidth: list.cellWidth * 3 + PlasmaCore.Units.gridUnit

            PC3.ScrollBar.horizontal.policy: PC3.ScrollBar.AlwaysOff

            // hide the flickering by fading in nicely
            opacity: setModelTimer.running ? 0 : 1
            Behavior on opacity {
                OpacityAnimator {
                    duration: PlasmaCore.Units.longDuration
                    easing.type: Easing.InOutQuad
                }
            }

            GridView {
                id: list

                // model set delayed by Timer above

                PlasmaExtras.PlaceholderMessage {
                    anchors.centerIn: parent
                    width: parent.width - (PlasmaCore.Units.largeSpacing * 4)
                    text: searchInput.text.length > 0 ? i18n("No widgets matched the search terms") : i18n("No widgets available")
                    visible: list.count == 0
                }

                activeFocusOnTab: true
                cellWidth: PlasmaCore.Units.iconSizes.enormous + (PlasmaCore.Units.gridUnit*2)
                cellHeight: cellWidth + PlasmaCore.Units.gridUnit * 4 + PlasmaCore.Units.smallSpacing * 2

                delegate: AppletDelegate {}
                highlight: PC2.Highlight {}
                highlightMoveDuration: 0
                //highlightResizeDuration: 0

                //slide in to view from the left
                add: Transition {
                    NumberAnimation {
                        properties: "y"
                        from: -list.height
                        duration: PlasmaCore.Units.shortDuration
                    }
                }

                //slide out of view to the right
                remove: Transition {
                    NumberAnimation {
                        properties: "y"
                        to: list.height
                        duration: PlasmaCore.Units.shortDuration
                    }
                }

                //if we are adding other items into the view use the same animation as normal adding
                //this makes everything slide in together
                //if we make it move everything ends up weird
                addDisplaced: list.add

                //moved due to filtering
                displaced: Transition {
                    NumberAnimation {
                        properties: "x,y"
                        duration: PlasmaCore.Units.shortDuration
                    }
                }
            }
        }
    }
}
