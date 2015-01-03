/*   vim:set foldmethod=marker:
 *
 *   Copyright (C) 2014 Ivan Cukic <ivan.cukic(at)kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License version 2,
 *   or (at your option) any later version, as published by the Free
 *   Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

import QtQuick 2.2
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.activities 0.1 as Activities

Item {
    id: root
    width: 208

    property int spacing: 2 * units.smallSpacing

    property bool showingDialog:
        newActivityDialog.visible ||
        activityList.showingDialog

    Connections {
        target: parent
        onVisibleChanged: {
            newActivityDialog.close();
            activityList.closeDialogs();
        }
    }

    signal closeRequested()

    Keys.onPressed: {
        if (newActivityDialog.visible || activityList.showingDialog) {
            event.accepted = false;

        } else {
            if (event.key == Qt.Key_Escape) {
                if (heading.searchString.length > 0) {
                    heading.searchString = ""
                } else {
                    root.closeRequested();
                }
            } else if (event.key == Qt.Key_Up) {
                activityList.selectPrevious();

            } else if (event.key == Qt.Key_Down) {
                activityList.selectNext();

            } else if (event.key == Qt.Key_Return || event.key == Qt.Key_Enter) {
                activityList.openSelected();

            } else if (event.key == Qt.Key_Tab) {
                // console.log("TAB KEY");

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

        onCloseRequested: root.closeRequested()
        Component.onCompleted: focusSearch()
    }

    PlasmaExtras.ScrollArea {
        anchors {
            top: heading.bottom
            bottom: bottomPanel.top
            left: parent.left
            right: parent.right
            topMargin: root.spacing
        }

        ActivityList {
            id: activityList

            filterString: heading.searchString.toLowerCase()

            onShowingDialogChanged:
                if (showingDialog) newActivityDialog.close()
        }
    }

    Item {
        id: bottomPanel

        height: newActivityButton.height + units.largeSpacing

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
                activityList.closeDialogs();
                newActivityDialog.open(newActivityDialog.height / 2);
            }

            opacity: 1 - newActivityDialog.opacity
        }

        ActivityCreationDialog {
            id: newActivityDialog

            z: 100

            anchors.bottom: newActivityButton.bottom
            anchors.left:   newActivityButton.left
            anchors.right:  newActivityButton.right

            onAccepted: {
                activityList.model.addActivity(activityName, function (id) {
                    activityList.model.setActivityIcon(id, newActivityDialog.activityIconSource, function() {});
                })
            }
            onOpacityChanged: {
                if (opacity == 0) {
                    heading.focusSearch()
                }
            }
        }
    }
}

