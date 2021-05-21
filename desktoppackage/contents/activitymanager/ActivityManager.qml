/*
    SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2014 Ivan Cukic <ivan.cukic@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.0

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.kquickcontrolsaddons 2.0

import org.kde.activities 0.1 as Activities
import org.kde.activities.settings 0.1

FocusScope {
    id: root
    signal closed()

    function parentClosed() {
        activityBrowser.parentClosed();
    }

    //this is used to perfectly align the filter field and delegates
    property int cellWidth: PlasmaCore.Theme.mSize(PlasmaCore.Theme.defaultFont).width * 30
    property int spacing: 2 * PlasmaCore.Units.smallSpacing

    property bool showSwitcherOnly: false

    width: PlasmaCore.Units.gridUnit * 16

    Item {
        id: activityBrowser

        property int spacing: 2 * PlasmaCore.Units.smallSpacing

        signal closeRequested()

        Keys.onPressed: {
            if (event.key === Qt.Key_Escape) {
                if (heading.searchString.length > 0) {
                    heading.searchString = "";
                } else {
                    activityBrowser.closeRequested();
                }

            } else if (event.key === Qt.Key_Up) {
                activityList.selectPrevious();

            } else if (event.key === Qt.Key_Down) {
                activityList.selectNext();

            } else if (event.key === Qt.Key_Return || event.key === Qt.Key_Enter) {
                activityList.openSelected();

            } else if (event.key === Qt.Key_Tab) {
                // console.log("TAB KEY");

            } else  {
                // console.log("OTHER KEY");
                // event.accepted = false;
                // heading.forceActiveFocus();
            }
        }

        // Rectangle {
        //     anchors.fill : parent
        //     opacity      : .4
        //     color        : "white"
        // }

        Heading {
            id: heading

            anchors {
                top: parent.top
                left: parent.left
                right: parent.right

                leftMargin: PlasmaCore.Units.smallSpacing
                rightMargin: PlasmaCore.Units.smallSpacing
            }

            onCloseRequested: activityBrowser.closeRequested()
            Component.onCompleted: focusSearch()

            visible: !root.showSwitcherOnly
        }

        PlasmaExtras.ScrollArea {
            anchors {
                top:    heading.visible ? heading.bottom : parent.top
                bottom: bottomPanel.visible ? bottomPanel.top : parent.bottom
                left:   parent.left
                right:  parent.right
                topMargin: activityBrowser.spacing
            }

            ActivityList {
                id: activityList
                showSwitcherOnly: root.showSwitcherOnly

                filterString: heading.searchString.toLowerCase()

                itemsWidth: root.width - PlasmaCore.Units.smallSpacing
            }
        }

        Item {
            id: bottomPanel

            height: newActivityButton.height + PlasmaCore.Units.largeSpacing

            visible: !root.showSwitcherOnly

            anchors {
                bottom: parent.bottom
                left: parent.left
                right: parent.right
            }

            PlasmaComponents.ToolButton {
                id: newActivityButton

                text: i18nd("plasma_shell_org.kde.plasma.desktop", "Create activity…")
                iconSource: "list-add"

                width: parent.width

                onClicked: ActivitySettings.newActivity()

                visible: ActivitySettings.newActivityAuthorized
                opacity: newActivityDialog.status == Loader.Ready ?
                              1 - newActivityDialog.item.opacity : 1
            }

            Loader {
                id: newActivityDialog

                z: 100

                anchors.bottom: newActivityButton.bottom
                anchors.left:   newActivityButton.left
                anchors.right:  newActivityButton.right

            }
        }

        onCloseRequested: root.closed()

        anchors.fill: parent
    }

}

