/***************************************************************************
 *   Copyright (C) 2013-2014 by Eike Hein <hein@kde.org>                   *
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

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.kquickcontrolsaddons 2.0

FocusScope {
    id: itemList

    width: units.gridUnit * 14
    height: listView.contentHeight

    property QtObject dialog: null
    property QtObject childDialog: null
    property bool containsMouse: (listener.containsMouse
        || (childDialog != null && childDialog.mainItem.containsMouse)
        || (listView.currentItem != null && listView.currentItem.menu.opened)
        || containsMouseOverride)
    property bool containsMouseOverride: false
    property bool iconsEnabled: false
    property int itemHeight: (Math.max(theme.mSize(theme.defaultFont).height, units.iconSizes.small)
        + highlightItemSvg.margins.top + highlightItemSvg.margins.bottom)

    property alias currentIndex: listView.currentIndex
    property alias keyNavigationWraps: listView.keyNavigationWraps
    property alias model: listView.model

    onContainsMouseChanged: {
        if (!containsMouse) {
            resetIndexTimer.start();
        } else {
            resetIndexTimer.stop();
        }
    }

    Timer {
        id: dialogSpawnTimer

        interval: 70
        repeat: false

        onTriggered: {
            if (!plasmoid.expanded || model == undefined || listView.currentIndex == -1) {
                return;
            }

            childDialog = itemListDialogComponent.createObject(itemList);
            childDialog.visualParent = listView.currentItem;
            childDialog.model = model.modelForRow(listView.currentIndex);
            childDialog.visible = true;

            windowSystem.forceActive(itemList);
        }
    }

    Timer {
        id: resetIndexTimer

        interval: (dialog != null) ? 50 : 150
        repeat: false

        onTriggered: {
            if (focus) {
                currentIndex = -1;
            }
        }
    }

    MouseEventListener {
        id: listener

        anchors.fill: parent

        hoverEnabled: true

        onContainsMouseChanged: {
            listView.eligibleWidth = listView.width;
            itemList.containsMouseOverride = false;
        }

        PlasmaExtras.ScrollArea {
            anchors.fill: parent

            ListView {
                id: listView

                property int eligibleWidth: width

                currentIndex: -1

                focus: true

                snapMode: ListView.SnapToItem
                spacing: 0
                keyNavigationWraps: (dialog != null)

                delegate: ItemListDelegate {}

                highlight: PlasmaComponents.Highlight { anchors.fill: listView.currentItem }

                onCountChanged: {
                    currentIndex = -1;
                }

                onCurrentIndexChanged: {
                    if (currentIndex != -1) {
                        itemList.focus = true;

                        if (childDialog) {
                            childDialog.visible = false;
                            childDialog.delayedDestroy();
                        }

                        if (currentItem != null && !currentItem.hasChildren || !plasmoid.expanded) {
                            return;
                        }

                        dialogSpawnTimer.restart();
                    } else if (childDialog != null) {
                        itemList.containsMouseOverride = false;
                        dialogSpawnTimer.stop();
                        childDialog.visible = false;
                        childDialog.delayedDestroy();
                        childDialog = null;
                    }
                }

                Keys.onPressed: {
                    if (event.key == Qt.Key_Right && childDialog != null) {
                        windowSystem.forceActive(childDialog.mainItem);
                        childDialog.mainItem.focus = true;
                        childDialog.mainItem.currentIndex = 0;
                    } else if (event.key == Qt.Key_Left && dialog != null) {
                        dialog.destroy();
                    } else if (event.key != Qt.Key_Escape && event.text != "") {
                        return; // FIXME
                        searchField.appendText(event.text);
                    }
                }
            }
        }
    }
}
