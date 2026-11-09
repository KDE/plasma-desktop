/*
    SPDX-FileCopyrightText: 2026 Nathan Heller <dev@naheller.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

/*
    This script accompanies a change to add more icon sizes (see MR below).
    Previously the max size (256px) was at index 6. Now it's at index 9.

    To ensure users who previously selected size 6 retain 256px icons,
    this script updates instances of iconSize=6 to iconSize=9.

    Specifically, it targets:
    - Desktop and panel containments whose plugin is org.kde.plasma.folder
    - Folder View applets contained within these desktop and panel
      containments

    MR: https://invent.kde.org/plasma/plasma-desktop/-/merge_requests/4094
*/

const FOLDER_PLUGIN = "org.kde.plasma.folder";
const ICON_SIZE_OLD = 6;
const ICON_SIZE_NEW = 9;

function migrateIconSize(configObject, description) {
    configObject.currentConfigGroup = ["General"];

    const currentSize = Number(configObject.readConfig("iconSize", -1));

    if (currentSize !== ICON_SIZE_OLD) {
        return false;
    }

    configObject.writeConfig("iconSize", ICON_SIZE_NEW);

    print("Migrated Folder View icon size: " + description);

    configObject.currentConfigGroup = [];
    configObject.reloadConfig();

    return true;
}

// Process desktop containments and panels
const containments = desktops().concat(panels());

for (const containment of containments) {

    // Only update desktop containments whose plugin is Folder View
    if (containment.type === FOLDER_PLUGIN) {
        migrateIconSize(
            containment,
            "desktop containment " + containment.id
        );
    }

    // Only update applets whose plugin is Folder View
    const folderWidgets = containment.widgets(FOLDER_PLUGIN);

    for (const widget of folderWidgets) {
        migrateIconSize(
            widget,
            "applet " + widget.id
        );
    }
}
