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

import org.kde.plasma.activityswitcher 1.0 as ActivitySwitcher

Flickable {
    id: root

    // contentWidth: content.width
    contentHeight: content.height

    property var    model: ActivitySwitcher.Backend.runningActivitiesModel()
    property string filterString: ""
    property bool   showSwitcherOnly: false

    property int itemsWidth: 0

    property int selectedIndex: -1

    function _selectRelativeToCurrent(distance)
    {
        var startingWithSelected = selectedIndex;

        do {
            selectedIndex += distance;

            if (selectedIndex >= activitiesList.count) {
                selectedIndex = 0;
            }

            if (selectedIndex < 0) {
                selectedIndex = activitiesList.count - 1;
            }

            // Searching for the first item that is visible, or back to the one
            // that we started with
        } while (!activitiesList.itemAt(selectedIndex).visible
                        && startingWithSelected !== selectedIndex);

        _updateSelectedItem();

    }

    function selectNext()
    {
        _selectRelativeToCurrent(1);
    }

    function selectPrevious()
    {
        _selectRelativeToCurrent(-1);
    }

    function _updateSelectedItem()
    {
        for (var i = 0; i < activitiesList.count; i++) {
            activitiesList.itemAt(i).selected = (i === selectedIndex);
        }
    }

    function openSelected()
    {
        var selectedItem = null;

        if (selectedIndex >= 0 && selectedIndex < activitiesList.count) {
            selectedItem = activitiesList.itemAt(selectedIndex);

        } else if (root.filterString != "") {
            // If we have only one item shown, activate it. It doesn't matter
            // that it is not really selected

            for (var i = 0; i < activitiesList.count; i++) {
                var item = activitiesList.itemAt(i);

                if (item.visible) {
                    selectedItem = item;
                    break;
                }
            }
        }

        if (selectedItem !== null) {
            ActivitySwitcher.Backend.setCurrentActivity(selectedItem.activityId);
        }
    }

    Column {
        id: content

        // width: root.width - (root.width % 10)
        width: root.itemsWidth
        spacing: units.smallSpacing * 2

        // Running activities

        Repeater {
            id: activitiesList

            model: ActivitySwitcher.Backend.runningActivitiesModel()

            ActivityItem {

                width:  content.width

                visible      : (root.filterString == "") ||
                               (title.toLowerCase().indexOf(root.filterString) != -1)

                activityId   : model.id
                title        : model.name
                icon         : model.iconSource
                background   : model.background
                current      : model.isCurrent
                hasWindows   : model.hasWindows
                innerPadding : 2 * units.smallSpacing
                stoppable    : activitiesList.count > 1

                onClicked    : {
                    ActivitySwitcher.Backend.setCurrentActivity(model.id);
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
            id: stoppedActivitiesHeading

            text: i18nd("plasma_shell_org.kde.plasma.desktop", "Stopped activities:")
            level: 3
            visible: !root.showSwitcherOnly && stoppedActivitiesList.count > 0
        }

        Repeater {
            id: stoppedActivitiesList

            model: root.showSwitcherOnly ? null : ActivitySwitcher.Backend.stoppedActivitiesModel()

            delegate: StoppedActivityItem {
                id: stoppedActivityItem

                width:  parent.width

                visible      : (root.filterString == "") ||
                               (title.toLowerCase().indexOf(root.filterString) != -1)

                activityId   : model.id
                title        : model.name
                icon         : model.iconSource
                innerPadding : 2 * units.smallSpacing

                onClicked: {
                    ActivitySwitcher.Backend.setCurrentActivity(model.id)
                }
            }
        }

        Item {
            // spacer
            width  : parent.width
            height : units.largeSpacing * 2

            visible: stoppedActivitiesHeading.visible
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

