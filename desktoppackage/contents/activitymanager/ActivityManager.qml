/*
 *   Copyright 2013 Marco Martin <mart@kde.org>
 *   Copyright 2014 Ivan Cukic <ivan.cukic@kde.org>
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

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.kquickcontrolsaddons 2.0

import org.kde.activities 0.1 as Activities

FocusScope {
    id: root
    signal closed()

    function parentClosed() {
        activityBrowser.parentClosed();
    }

    //this is used to perfectly align the filter field and delegates
    property int cellWidth: theme.mSize(theme.defaultFont).width * 30
    property int spacing: 2 * units.smallSpacing

    property bool showSwitcherOnly: false

    property bool showingDialog: activityBrowser.showingDialog

    width: units.gridUnit * 16

    Item {
        id: activityBrowser

        property int spacing: 2 * units.smallSpacing

        property bool showingDialog:
            activityList.showingDialog ||
            (
                newActivityDialog.status == Loader.Ready &&
                newActivityDialog.item.visible
            )

        Connections {
            target: parent
            onVisibleChanged: {
                newActivityDialog.item.close();
                activityList.closeDialogs();
            }
        }

        signal closeRequested()

        Keys.onPressed: {
            if (activityBrowser.showingDialog) {
                event.accepted = false;

            } else {
                if (event.key == Qt.Key_Escape) {
                    if (heading.searchString.length > 0) {
                        heading.searchString = "";
                    } else {
                        activityBrowser.closeRequested();
                    }

                } else if (event.key == Qt.Key_Up) {
                    activityList.selectPrevious();

                } else if (event.key == Qt.Key_Down) {
                    activityList.selectNext();

                } else if (event.key == Qt.Key_Return || event.key == Qt.Key_Enter) {
                    activityList.openSelected();

                } else if (event.key == Qt.Key_Tab) {
                    // console.log("TAB KEY");

                } else  {
                    // console.log("OTHER KEY");
                    // event.accepted = false;
                    // heading.forceActiveFocus();
                }
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

                leftMargin: units.smallSpacing
                rightMargin: units.smallSpacing
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

                onShowingDialogChanged:
                    if (showingDialog && newActivityDialog.status == Loader.Ready)
                        newActivityDialog.item.close()
            }
        }

        Item {
            id: bottomPanel

            height: newActivityButton.height + units.largeSpacing

            visible: !root.showSwitcherOnly

            anchors {
                bottom: parent.bottom
                left: parent.left
                right: parent.right
            }

            PlasmaComponents.ToolButton {
                id: newActivityButton

                text: i18nd("plasma_shell_org.kde.plasma.desktop", "Create activity...")
                iconSource: "list-add"

                width: parent.width

                onClicked: {
                    newActivityDialog.loadAndOpen();
                }

                opacity: newActivityDialog.status == Loader.Ready ?
                              1 - newActivityDialog.item.opacity : 1
            }

            Loader {
                id: newActivityDialog

                z: 100

                anchors.bottom: newActivityButton.bottom
                anchors.left:   newActivityButton.left
                anchors.right:  newActivityButton.right

                function loadAndOpen() {
                    if (newActivityDialog.source == "ActivityCreationDialog.qml") {
                        showDialog();

                    } else {
                        newActivityDialog.source = "ActivityCreationDialog.qml";

                    }

                }

                function showDialog() {
                    activityList.closeDialogs();
                    newActivityDialog.item.open(
                        newActivityDialog.item.height / 2);
                }

                onLoaded: {
                    newActivityDialog.item.accepted.connect(function() {
                        activityList.model.addActivity(item.activityName, function (id) {
                            activityList.model.setActivityIcon(id, newActivityDialog.item.activityIconSource, function() {});
                        });
                    });
                    showDialog();
                }

                onOpacityChanged: {
                    if (opacity == 0) {
                        heading.focusSearch();
                    }
                }
            }
        }

        onCloseRequested: root.closed()

        anchors.fill: parent
    }

}

