/***************************************************************************
 *   Copyright (C) 2012-2013 by Eike Hein <hein@kde.org>                   *
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

import QtQuick 2.0

import org.kde.draganddrop 2.0
import org.kde.kquickcontrolsaddons 2.0

import "../code/layout.js" as LayoutManager
import "../code/tools.js" as TaskTools

Item {
    property Item target

    DropArea {
        id: dropHandler

        preventStealing: true;
        anchors.fill: parent

        property Item hoveredItem

        onDragMove: {
            if (target.animating) {
                return;
            }

            var above = target.childAt(event.x, event.y);

            // HACK: plasmoid.configuration.sortingStrategy is an integer representation
            // of the TaskManager::GroupManager::TaskSortingStrategy enum.
            if (tasks.dragSource && plasmoid.configuration.sortingStrategy == 1) {
                if (tasks.dragSource != above && !tasks.dragSource.isLauncher
                    && !(above && "isLauncher" in above && above.isLauncher)) {
                    itemMove(tasks.dragSource.itemId,
                        TaskTools.insertionIndexAt(tasks.dragSource.itemIndex,
                            event.x, event.y));
                }
            } else if (!tasks.dragSource && above && hoveredItem != above) {
                hoveredItem = above;
                activationTimer.restart();
            } else if (!above) {
                hoveredItem = null;
                activationTimer.stop();
            }
        }

        onDragLeave: {
            hoveredItem = null;
            activationTimer.stop();
        }

        Timer {
            id: activationTimer

            interval: 250
            repeat: false

            onTriggered: {
                if (parent.hoveredItem.isGroupParent) {
                    groupDialog.visualParent = parent.hoveredItem;
                    groupDialog.visible = true;
                } else if (!parent.hoveredItem.isLauncher) {
                    tasks.activateItem(parent.hoveredItem.itemId, false);
                }
            }
        }
    }

    MouseEventListener {
        id: wheelHandler

        anchors.fill: parent

        onWheelMoved: TaskTools.activateNextPrevTask(false, wheel.delta < 0)
    }
}
