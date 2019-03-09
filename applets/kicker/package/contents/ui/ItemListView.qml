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

    property real minimumWidth: units.gridUnit * 14
    property real maximumWidth: minimumWidth * 2

    width: minimumWidth
    height: listView.contentHeight

    signal exited
    signal keyNavigationAtListEnd
    signal appendSearchText(string text)

    property Item focusParent: null
    property QtObject dialog: null
    property QtObject childDialog: null
    property bool iconsEnabled: false
    property int itemHeight: Math.ceil((Math.max(theme.mSize(theme.defaultFont).height, units.iconSizes.small)
        + Math.max(highlightItemSvg.margins.top + highlightItemSvg.margins.bottom,
        listItemSvg.margins.top + listItemSvg.margins.bottom)) / 2) * 2
    property int separatorHeight: lineSvg.horLineHeight + (2 * units.smallSpacing)

    property alias currentIndex: listView.currentIndex
    property alias currentItem: listView.currentItem
    property alias keyNavigationWraps: listView.keyNavigationWraps
    property alias showChildDialogs: listView.showChildDialogs
    property alias model: listView.model
    property alias count: listView.count
    property alias containsMouse: listener.containsMouse
    property alias resetOnExitDelay: resetIndexTimer.interval

    onFocusParentChanged: {
        appendSearchText.connect(focusParent.appendSearchText);
    }

    Timer {
        id: dialogSpawnTimer

        property bool focusOnSpawn: false

        interval: 70
        repeat: false

        onTriggered: {
            if (!plasmoid.expanded || model === undefined || currentIndex == -1) {
                return;
            }

            if (childDialog != null) {
                childDialog.delayedDestroy();
            }

            // Gets reenabled after the dialog spawn causes a focus-in on the dialog window.
            plasmoid.hideOnWindowDeactivate = false;

            childDialog = itemListDialogComponent.createObject(itemList);
            childDialog.focusParent = itemList;
            childDialog.visualParent = listView.currentItem;
            childDialog.model = model.modelForRow(listView.currentIndex);
            childDialog.visible = true;

            windowSystem.forceActive(childDialog.mainItem);
            childDialog.mainItem.focus = true;

            if (focusOnSpawn) {
                childDialog.mainItem.showChildDialogs = false;
                childDialog.mainItem.currentIndex = 0;
                childDialog.mainItem.showChildDialogs = true;
            }
        }
    }

    Timer {
        id: resetIndexTimer

        interval: (dialog != null) ? 50 : 150
        repeat: false

        onTriggered: {
            if (focus && (!childDialog || !childDialog.mainItem.containsMouse)) {
                currentIndex = -1;
                itemList.exited();
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
                itemList.forceActiveFocus();
            } else if ((!childDialog || !dialog)
                && (!currentItem || !currentItem.menu.opened)) {
                resetIndexTimer.start();
            }
        }

        PlasmaExtras.ScrollArea {
            anchors.fill: parent

            focus: true

            ListView {
                id: listView

                property bool showChildDialogs: true
                property int eligibleWidth: width

                currentIndex: -1

                boundsBehavior: Flickable.StopAtBounds
                snapMode: ListView.SnapToItem
                spacing: 0
                keyNavigationWraps: (dialog != null)

                delegate: ItemListDelegate {
                    onFullTextWidthChanged: {
                        if (fullTextWidth > itemList.width) itemList.width = Math.min(fullTextWidth, itemList.maximumWidth);
                    }
                }

                highlight: PlasmaComponents.Highlight {
                    visible: listView.currentItem && !listView.currentItem.isSeparator
                }

                highlightMoveDuration: 0

                onCountChanged: {
                    currentIndex = -1;
                }

                onCurrentIndexChanged: {
                    if (currentIndex != -1) {
                        if (childDialog) {
                            if (currentItem && currentItem.hasChildren) {
                                childDialog.mainItem.width = itemList.minimumWidth;
                                childDialog.model = model.modelForRow(currentIndex);
                                childDialog.visualParent = listView.currentItem;
                            } else {
                                childDialog.delayedDestroy();
                            }

                            return;
                        }

                        if (currentItem == null || !currentItem.hasChildren || !plasmoid.expanded) {
                            dialogSpawnTimer.stop();

                            return;
                        }

                        if (showChildDialogs) {
                            dialogSpawnTimer.focusOnSpawn = false;
                            dialogSpawnTimer.restart();
                        }
                    } else if (childDialog != null) {
                        childDialog.delayedDestroy();
                        childDialog = null;
                    }
                }

                onCurrentItemChanged: {
                    if (currentItem) {
                        currentItem.menu.closed.connect(resetIndexTimer.restart);
                    }
                }

                Keys.onPressed: {
                    if (event.key === Qt.Key_Up) {
                        event.accepted = true;

                        if (!keyNavigationWraps && currentIndex == 0) {
                            itemList.keyNavigationAtListEnd();

                            return;
                        }

                        showChildDialogs = false;
                        decrementCurrentIndex();

                        if (currentItem.isSeparator) {
                            decrementCurrentIndex();
                        }

                        showChildDialogs = true;
                    } else if (event.key === Qt.Key_Down) {
                        event.accepted = true;

                        if (!keyNavigationWraps && currentIndex == count - 1) {
                            itemList.keyNavigationAtListEnd();

                            return;
                        }

                        showChildDialogs = false;
                        incrementCurrentIndex();

                        if (currentItem.isSeparator) {
                            incrementCurrentIndex();
                        }

                        showChildDialogs = true;
                    } else if ((event.key === Qt.Key_Right || event.key === Qt.Key_Return || event.key === Qt.Key_Enter) && childDialog != null) {
                        windowSystem.forceActive(childDialog.mainItem);
                        childDialog.mainItem.focus = true;
                        childDialog.mainItem.currentIndex = 0;
                    } else if ((event.key === Qt.Key_Right || event.key === Qt.Key_Return || event.key === Qt.Key_Enter) && childDialog == null
                        && currentItem != null && currentItem.hasChildren) {
                        dialogSpawnTimer.focusOnSpawn = true;
                        dialogSpawnTimer.restart();
                    } else if (event.key === Qt.Key_Left && dialog != null) {
                        dialog.destroy();
                    } else if (event.key === Qt.Key_Escape) {
                        plasmoid.expanded = false;
                    } else if (event.key === Qt.Key_Tab) {
                        //do nothing, and skip appending text
                    } else if (event.text !== "") {
                        appendSearchText(event.text);
                    }
                }
            }
        }
    }

    Component.onCompleted: {
        windowSystem.monitorWindowFocus(itemList);

        if (dialog == null) {
            appendSearchText.connect(root.appendSearchText);
        }
    }
}
