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

    signal appendSearchText(string text)

    property Item focusParent: null
    property QtObject dialog: null
    property QtObject childDialog: null
    property bool iconsEnabled: false
    property int itemHeight: (Math.max(theme.mSize(theme.defaultFont).height, units.iconSizes.small)
        + highlightItemSvg.margins.top + highlightItemSvg.margins.bottom)

    property alias currentIndex: listView.currentIndex
    property alias keyNavigationWraps: listView.keyNavigationWraps
    property alias model: listView.model
    property alias containsMouse: listener.containsMouse

    onFocusParentChanged: {
        appendSearchText.connect(focusParent.appendSearchText);
    }

    Timer {
        id: dialogSpawnTimer

        interval: 70
        repeat: false

        onTriggered: {
            if (!plasmoid.expanded || model == undefined || currentIndex == -1) {
                return;
            }

            childDialog = itemListDialogComponent.createObject(itemList);
            childDialog.focusParent = itemList;
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
            if (focus && (!childDialog || !childDialog.mainItem.containsMouse)) {
                if (!dialog) {
                    root.reset();
                } else {
                    currentIndex = -1;
                }
            }
        }
    }

    MouseEventListener {
        id: listener

        anchors.fill: parent

        hoverEnabled: true

        onContainsMouseChanged: {
            listView.eligibleWidth = listView.width;

            if (containsMouse) {
                resetIndexTimer.stop();
            } else if (!childDialog || !dialog) {
                resetIndexTimer.start();
            }
        }

        PlasmaExtras.ScrollArea {
            anchors.fill: parent

            ListView {
                id: listView

                property int eligibleWidth: width

                currentIndex: -1

                focus: true

                boundsBehavior: Flickable.StopAtBounds
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
                            childDialog.model = model.modelForRow(currentIndex);
                            childDialog.visualParent = listView.currentItem;

                            return;
                        }

                        if (currentItem != null && !currentItem.hasChildren || !plasmoid.expanded) {
                            return;
                        }

                        dialogSpawnTimer.restart();
                    } else if (childDialog != null) {
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
                    } else if (event.key == Qt.Key_Escape) {
                        plasmoid.expanded = false;
                    } else if (event.text != "") {
                        appendSearchText(event.text);
                    }
                }
            }
        }
    }

    Component.onCompleted: {
        if (dialog == null) {
            appendSearchText.connect(root.appendSearchText);
        }
    }
}
