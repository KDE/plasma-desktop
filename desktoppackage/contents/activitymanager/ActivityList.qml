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
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.activities 0.1 as Activities


Flickable {
    id: root

    /*contentWidth: content.width*/
    contentHeight: content.height

    property var    model: activitiesModel
    property string filterString: ""
    property bool   showingDialog: activityDeletionDialog.visible || activityConfigurationDialog.visible

    function closeDialogs()
    {
        activityDeletionDialog.close();
        activityConfigurationDialog.close();
    }

    Activities.ActivityModel {
        id: activitiesModel

        shownStates: "Running,Stopping"
    }

    Activities.ActivityModel {
        id: stoppedActivitiesModel

        shownStates: "Stopped,Starting"
    }

    ActivityDeletionDialog {
        id: activityDeletionDialog
        z: 10

        width: parent.width

        onAccepted: {
            activitiesModel.removeActivity(activityDeletionDialog.activityId, function () {})
        }
    }

    ActivityCreationDialog {
        id: activityConfigurationDialog
        z: 10

        acceptButtonText: i18n("Apply")

        width: parent.width

        onAccepted: {
            var id = activityConfigurationDialog.activityId
            activitiesModel.setActivityName(id,
                activityConfigurationDialog.activityName,
                function () {});
            activitiesModel.setActivityIcon(id,
                activityConfigurationDialog.activityIconSource,
                function () {});
        }
    }

    Column {
        id: content

        width: parent.width

        // Running activities

        Repeater {
            model: activitiesModel

            ActivityItem {

                width:  parent.width

                visible      : (root.filterString == "") ||
                               (title.toLowerCase().indexOf(root.filterString) != -1)

                title        : model.name
                icon         : model.iconSource
                background   : model.background
                current      : model.current
                innerPadding : 2 * units.smallSpacing

                onClicked          : {
                    activitiesModel.setCurrentActivity(model.id, function () {})
                }

                onStopClicked      : {
                    activitiesModel.stopActivity(model.id, function () {});
                }

                onConfigureClicked : {
                    activityConfigurationDialog.activityId = model.id;
                    activityConfigurationDialog.activityName = title;
                    activityConfigurationDialog.activityIconSource = icon;

                    var selfLocation = mapToItem(root, width / 2, height / 2);
                    activityConfigurationDialog.open(selfLocation.y);
                    activityDeletionDialog.close();
                }
            }
        }

        // Stopped activities

        Item {
            // spacer
            width  : parent.width
            height : units.largeSpacing
        }

        PlasmaExtras.Heading {
            text: "Stopped activities:"
            level: 3
            visible: stoppedActivitiesList.count > 0
        }

        Repeater {
            id: stoppedActivitiesList

            model: stoppedActivitiesModel

            delegate: StoppedActivityItem {
                id: stoppedActivityItem

                width:  parent.width

                visible      : (root.filterString == "") ||
                               (title.toLowerCase().indexOf(root.filterString) != -1)

                title        : model.name
                icon         : model.iconSource
                innerPadding : 2 * units.smallSpacing

                onClicked          : {
                    stoppedActivitiesModel.setCurrentActivity(model.id, function () {})
                }

                onDeleteClicked    : {
                    activityDeletionDialog.activityId = model.id;

                    var selfLocation = mapToItem(root, width / 2, height / 2);
                    activityDeletionDialog.open(selfLocation.y);
                    activityConfigurationDialog.close();
                }

                onConfigureClicked : {
                    activityConfigurationDialog.activityId = model.id;
                    activityConfigurationDialog.activityName = title;
                    activityConfigurationDialog.activityIconSource = icon;

                    var selfLocation = mapToItem(root, width / 2, height / 2);
                    activityConfigurationDialog.open(selfLocation.y);
                    activityDeletionDialog.close();
                }
            }
        }

        add: Transition {
            NumberAnimation {
                properties: "x"
                from: -100
                duration: units.shortDuration
            }
        }

        move: Transition {
            NumberAnimation {
                id: animation
                properties: "y"
                duration: units.longDuration
            }
        }
    }
}

