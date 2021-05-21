/*   vim:set foldmethod=marker:

    SPDX-FileCopyrightText: 2015 Ivan Cukic <ivan.cukic(at)kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

.pragma library

var currentlyHoveredActivityItem = null
function showActivityItemActionsBar(
        activityItem
    )
{
    if (activityItem == currentlyHoveredActivityItem) {
        return;
    }

    if (currentlyHoveredActivityItem != null) {
        currentlyHoveredActivityItem.state = "plain"
    }

    currentlyHoveredActivityItem = activityItem;
    currentlyHoveredActivityItem.state = "showingControls";
}

