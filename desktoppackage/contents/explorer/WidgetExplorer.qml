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
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.kquickcontrolsaddons 2.0
import QtQuick.Window 2.1
import org.kde.plasma.private.shell 2.0

Item {
    id: main

    width: 240
    height: 800//Screen.height
    //this is used to perfectly align the filter field and delegates
    property int cellWidth: theme.mSize(theme.defaultFont).width * 10

    property int minimumWidth: theme.mSize(theme.defaultFont).width * 12
    property int minimumHeight: 800//topBar.height + list.delegateHeight + (widgetExplorer.orientation == Qt.Horizontal ? scrollBar.height : 0) + 4

    property alias containment: widgetExplorer.containment

    property Item getWidgetsButton
    property Item categoryButton

    signal closed()

    WidgetExplorer {
        id: widgetExplorer
        //view: desktop
        onShouldClose: main.closed();
    }

    PlasmaComponents.ModelContextMenu {
        id: categoriesDialog
        visualParent: main.categoryButton
        model: widgetExplorer.filterModel

        onClicked: {
            list.contentX = 0
            list.contentY = 0
            main.categoryButton.text = model.display
            widgetExplorer.widgetsModel.filterQuery = model.filterData
            widgetExplorer.widgetsModel.filterType = model.filterType
        }
    }

    PlasmaComponents.ModelContextMenu {
        id: getWidgetsDialog
        visualParent: main.getWidgetsButton
        model: widgetExplorer.widgetsMenuActions
        onClicked: model.trigger()
    }
    /*
    PlasmaCore.Dialog {
        id: tooltipDialog
        property Item appletDelegate
        location: PlasmaCore.Types.RightEdge //actually we want this to be the opposite location of the explorer itself

        type: PlasmaCore.Dialog.Tooltip
        flags:Qt.Window|Qt.WindowStaysOnTopHint|Qt.X11BypassWindowManagerHint

        onAppletDelegateChanged: {
            if (!appletDelegate) {
                toolTipHideTimer.restart()
                toolTipShowTimer.running = false
            } else if (tooltipDialog.visible) {
                tooltipDialog.visualParent = appletDelegate
            } else {
                tooltipDialog.visualParent = appletDelegate
                toolTipShowTimer.restart()
                toolTipHideTimer.running = false
            }
        }
        mainItem: Tooltip { id: tooltipWidget }

        Behavior on y {
            NumberAnimation { duration: units.longDuration }
        }
    }
    Timer {
        id: toolTipShowTimer
        interval: 500
        repeat: false
        onTriggered: {
            tooltipDialog.visible = true
        }
    }
    Timer {
        id: toolTipHideTimer
        interval: 1000
        repeat: false
        onTriggered: tooltipDialog.visible = false
    }
    */

    Loader {
        id: topBar
        property Item categoryButton

        sourceComponent: verticalTopBarComponent
        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
            topMargin: 0
            leftMargin: units.smallSpacing
            rightMargin: units.smallSpacing
        }
    }
    Component {
        id: verticalTopBarComponent

        Column {
            anchors.top: parent.top
            anchors.left:parent.left
            anchors.right: parent.right
            spacing: 4

            PlasmaComponents.ToolButton {
                anchors.right: parent.right
                iconSource: "window-close"
                onClicked: main.closed()
            }
            PlasmaComponents.TextField {
                anchors {
                    left: parent.left
                    right: parent.right
                }
                clearButtonShown: true
                placeholderText: i18n("Enter search term...")
                onTextChanged: {
                    list.contentX = 0
                    list.contentY = 0
                    widgetExplorer.widgetsModel.searchTerm = text
                }
                Component.onCompleted: forceActiveFocus()
            }
            PlasmaComponents.Button {
                anchors {
                    left: parent.left
                    right: parent.right
                }
                id: categoryButton
                text: i18n("Categories")
                onClicked: categoriesDialog.open(0, categoryButton.height)
            }
            Component.onCompleted: {
                main.categoryButton = categoryButton
            }
        }
    }

    PlasmaExtras.ScrollArea {
        anchors {
            top: topBar.bottom
            left: parent.left
            right: parent.right
            bottom: bottomBar.top
            topMargin: units.smallSpacing
            leftMargin: units.smallSpacing
            bottomMargin: units.smallSpacing
        }
        flickableItem: ListView {
            id: list

            property int delegateWidth: list.width
            property int delegateHeight: theme.defaultFont.pixelSize * 7 - 4

            snapMode: ListView.SnapToItem
            model: widgetExplorer.widgetsModel

            clip: true //TODO work out why this is this needed

            delegate: AppletDelegate {}

            //slide in to view from the left
            add: Transition {
                NumberAnimation {
                    properties: "x"
                    from: -list.width
                    to: 0
                    duration: units.shortDuration * 3

                }
            }

            //slide out of view to the right
            remove: Transition {
                NumberAnimation {
                    properties: "x"
                    to: list.width
                    duration: units.shortDuration * 3
                }
            }

            //if we are adding other items into the view use the same animation as normal adding
            //this makes everything slide in together
            //if we make it move everything ends up weird
            addDisplaced: list.add

            //moved due to filtering
            displaced: Transition {
                NumberAnimation {
                    properties: "y"
                    duration: units.shortDuration * 3
                }
            }
        }
    }

    Loader {
        id: bottomBar

        sourceComponent: verticalBottomBarComponent
        anchors {
            left: parent.left
            right: parent.right
            bottom: parent.bottom
            leftMargin: units.smallSpacing
            rightMargin: units.smallSpacing
            bottomMargin: units.smallSpacing
        }
    }

    Component {
        id: verticalBottomBarComponent
        Column {
            anchors {
                left: parent.left
                right: parent.right
                bottom: parent.bottom
            }

            spacing: units.smallSpacing

            PlasmaComponents.Button {
                anchors {
                    left: parent.left
                    right: parent.right
                }
                id: getWidgetsButton
                iconSource: "get-hot-new-stuff"
                text: i18n("Get new widgets")
                onClicked: getWidgetsDialog.open()
            }

            Repeater {
                model: widgetExplorer.extraActions.length
                PlasmaComponents.Button {
                    anchors {
                        left: parent.left
                        right: parent.right
                    }
                    iconSource: widgetExplorer.extraActions[modelData].icon
                    text: widgetExplorer.extraActions[modelData].text
                    onClicked: {
                        widgetExplorer.extraActions[modelData].trigger()
                    }
                }
            }

            Component.onCompleted: {
                main.getWidgetsButton = getWidgetsButton
            }
        }
    }
}

