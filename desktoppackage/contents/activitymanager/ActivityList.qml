/*   vim:set foldmethod=marker:

    SPDX-FileCopyrightText: 2014 Ivan Cukic <ivan.cukic(at)kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import org.kde.kirigami as Kirigami
import org.kde.activities as Activities

import org.kde.plasma.activityswitcher as ActivitySwitcher

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
        var visited = 0;

        do {
            selectedIndex += distance;
            visited++;

            if (selectedIndex >= activitiesList.count) {
                selectedIndex = 0;
            }

            if (selectedIndex < 0) {
                selectedIndex = activitiesList.count - 1;
            }

            if (visited >= activitiesList.count) {
                // we've visited all activities but could not find a visible
                // one, so stop searching and select the one we started with
                selectedIndex = startingWithSelected;
                break;
            }

            // Searching for the first item that is visible
        } while (!activitiesList.itemAt(selectedIndex).visible);
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
        spacing: Kirigami.Units.smallSpacing * 2

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
                innerPadding : 2 * Kirigami.Units.smallSpacing

                onClicked    : {
                    ActivitySwitcher.Backend.setCurrentActivity(model.id);
                }
            }
        }

        add: Transition {
            NumberAnimation {
                properties: "x"
                from: -100
                duration: Kirigami.Units.shortDuration
            }
        }

        move: Transition {
            NumberAnimation {
                id: animation
                properties: "y"
                duration: Kirigami.Units.longDuration
            }
        }
    }
}

