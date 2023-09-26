/*   vim:set foldmethod=marker:

    SPDX-FileCopyrightText: 2023 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

function filterDisabled(entries) {
    let filteredEntries = [];

    // 0 = screen, 1 = activity, 2 = how many entries, 3 = desktop entry
    let state = 0;
    let entriesForCurrentScreen = 0;

    let currentScreen = -1;
    let currentActivity = "";
    let currentEntrtriesNumber = 0;
    let currentEntry = 0;
    let currentEntries = [];

    for (let e of entries) {
        switch (state) {
        case 0: // Screen
            currentScreen = e;
            state = 1;
            break;
        case 1: // Activity
            currentActivity = e;
            state = 2;
            break;
        case 2: // Entries number
            currentEntrtriesNumber = Number(e);
            state = 3;
            break;
        case 3: // Desktop file
            if (e.indexOf("desktop:/") !== 0) { // User has a folderview not in desktop:/
                currentEntries.push(e);
                currentEntry++;
            } else {
                let count = (e.match(/\//g) || []).length;
                if (count == 1) {
                    currentEntries.push(e);
                    currentEntry++;
                } else {
                    currentEntrtriesNumber--;
                }
            }

            if (currentEntry === currentEntrtriesNumber) {
                state = 0;
                if (currentEntries.length > 0) {
                    filteredEntries = filteredEntries.concat([currentScreen, currentActivity, currentEntrtriesNumber]);
                    filteredEntries = filteredEntries.concat(currentEntries);
                    currentEntries = [];
                }
            }
            break;
        }

    }
    return filteredEntries;
}

function filterEnabled(entries) {
    let filteredEntries = [];

    // 0 = desktop entry, 1 = screen 2 = activity
    let state = 0;
    let shouldDrop = false; //true when this entry should be dropped

    for (let e of entries) {
        switch (state) {
        case 0: // Desktop file
            if (e.indexOf("desktop:/") !== 0) { // User has a folderview not in desktop:/
                filteredEntries.push(e);
                shouldDrop = false;
            } else {
                let count = (e.match(/\//g) || []).length;
                if (count == 1) {
                    filteredEntries.push(e);
                    shouldDrop = false;
                } else {
                    shouldDrop = true;
                }
            }
            break;
        case 1: // Screen
        case 2: // Activity
            if (!shouldDrop) {
                filteredEntries.push(e);
            }
        }
        state = (state + 1) % 3;
    }
    return filteredEntries;
}

const config = ConfigFile('plasma-org.kde.plasma.desktop-appletsrc');
config.group = 'ScreenMapping';

let entries = config.readEntry("itemsOnDisabledScreens").split(",");
let filteredEntries = filterDisabled(entries);

config.writeEntry("itemsOnDisabledScreens", filteredEntries.join(","));

entries = config.readEntry("screenMapping").split(",");
filteredEntries = filterEnabled(entries);

config.writeEntry("screenMapping", filteredEntries.join(","));
