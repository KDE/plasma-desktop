/*
    SPDX-FileCopyrightText: 2023 Akseli Lahtinen <akselmo@akselmo.dev>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

/*
    In the new panel visibility setting we have only two visibility values
    instead of 2.
    This script migrates the values to their closest counterpart.
*/

const panels = desktops().concat(panels());
for (var i in containments) {
    var cont = containments[i];
    var visibility = cont.readConfig("panelVisibility",0);
    // Set visibility to autohide if it has previously been covered by windows
    // since that is the closest setting
    if (visibility === 2)
    {
        cont.writeConfig("panelVisibility", 1);
    }
    // Otherwise use always visible if it's not autohide already
    else if (visibility !== 1)
    {
        cont.writeConfig("panelVisibility", 0);
    }
}
