/***************************************************************************
 *   Copyright (C) 2016 by Eike Hein <hein@kde.org>                        *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

import QtQuick 2.4

Row {
    id: tabBar

    property int activeTab: 0
    property int hoveredTab: -1

    signal containsMouseChanged(int index, bool containsMouse)

    onContainsMouseChanged: {
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
    }

    DashboardTabButton {
        id: widgets

        index: 1

        text: i18n("Widgets")

        active: (tabBar.activeTab == 1)
    }

    Keys.onLeftPressed: {
        if (activeTab == 1) {
            activeTab = 0;
        }
    }

    Keys.onRightPressed: {
        if (activeTab == 0) {
            activeTab = 1;
        }
    }

    Keys.onPressed: {
        if (event.key == Qt.Key_Tab) {
            event.accepted = true;

            if (searching) {
                cancelSearchButton.focus = true;
            } else {
                mainColumn.tryActivate(0, 0);
            }
        } else if (event.key == Qt.Key_Backtab) {
            event.accepted = true;

            if (globalFavoritesGrid.enabled) {
                globalFavoritesGrid.tryActivate(0, 0);
            } else {
                systemFavoritesGrid.tryActivate(0, 0);
            }
        }
    }
}

