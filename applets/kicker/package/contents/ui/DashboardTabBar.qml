/*
    SPDX-FileCopyrightText: 2016 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15

Row {
    id: tabBar

    property int activeTab: 0
    property int hoveredTab: -1
    Accessible.role: Accessible.PageTabList

    signal containsMouseChanged(int index, bool containsMouse)

    onContainsMouseChanged: (index, containsMouse) => {
        if (containsMouse) {
            hoveredTab = index;
        } else {
            hoveredTab = -1;
        }
    }

    DashboardTabButton {
        id: appsdocs

        index: 0

        text: i18n("Apps & Docs")

        active: (tabBar.activeTab == 0)
        focus: parent.focus & active
    }

    DashboardTabButton {
        id: widgets

        index: 1

        text: i18n("Widgets")

        active: (tabBar.activeTab == 1)
        focus: parent.focus & active
    }

    Keys.onLeftPressed: event => {
        if (activeTab == 1) {
            activeTab = 0;
        }
    }

    Keys.onRightPressed: event => {
        if (activeTab == 0) {
            activeTab = 1;
        }
    }

    Keys.onPressed: event => {
        if (event.key === Qt.Key_Tab) {
            event.accepted = true;

            if (searching) {
                cancelSearchButton.focus = true;
            } else {
                mainColumn.tryActivate(0, 0);
            }
        } else if (event.key === Qt.Key_Backtab) {
            event.accepted = true;

            if (globalFavoritesGrid.enabled) {
                globalFavoritesGrid.tryActivate(0, 0);
            } else {
                systemFavoritesGrid.tryActivate(0, 0);
            }
        }
    }
}
