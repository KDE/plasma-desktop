/*
    SPDX-FileCopyrightText: 2013-2014 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15

import org.kde.kquickcontrolsaddons 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.extras 2.0 as PlasmaExtras

FocusScope {
    id: itemList

    property real minimumWidth: PlasmaCore.Units.gridUnit * 14
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
    property int itemHeight: Math.ceil((Math.max(theme.mSize(theme.defaultFont).height, PlasmaCore.Units.iconSizes.small)
        + Math.max(highlightItemSvg.margins.top + highlightItemSvg.margins.bottom,
        listItemSvg.margins.top + listItemSvg.margins.bottom)) / 2) * 2
    property int separatorHeight: lineSvg.horLineHeight + (2 * PlasmaCore.Units.smallSpacing)

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

            if (itemList.childDialog != null) {
                itemList.childDialog.delayedDestroy();
            }

            // Gets reenabled after the dialog spawn causes a focus-in on the dialog window.
            plasmoid.hideOnWindowDeactivate = false;

            itemList.childDialog = itemListDialogComponent.createObject(itemList);
            itemList.childDialog.focusParent = itemList;
            itemList.childDialog.visualParent = listView.currentItem;
            itemList.childDialog.model = model.modelForRow(listView.currentIndex);
            itemList.childDialog.visible = true;

            windowSystem.forceActive(itemList.childDialog.mainItem);
            itemList.childDialog.mainItem.focus = true;

            if (focusOnSpawn) {
                itemList.childDialog.mainItem.showChildDialogs = false;
                itemList.childDialog.mainItem.currentIndex = 0;
                itemList.childDialog.mainItem.showChildDialogs = true;
            }
        }
    }

    Timer {
        id: resetIndexTimer

        interval: (dialog != null) ? 50 : 150
        repeat: false

        onTriggered: {
            if (focus && (!itemList.childDialog || !itemList.childDialog.mainItem.containsMouse)) {
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
            } else if ((!itemList.childDialog || !dialog)
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

                highlight: PlasmaExtras.Highlight {
                    visible: listView.currentItem && !listView.currentItem.isSeparator
                }

                highlightMoveDuration: 0

                onCountChanged: {
                    currentIndex = -1;
                }

                onCurrentIndexChanged: {
                    if (currentIndex != -1) {
                        if (itemList.childDialog) {
                            if (currentItem && currentItem.hasChildren) {
                                itemList.childDialog.mainItem.width = itemList.minimumWidth;
                                itemList.childDialog.model = model.modelForRow(currentIndex);
                                itemList.childDialog.visualParent = listView.currentItem;
                            } else {
                                itemList.childDialog.delayedDestroy();
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
                    } else if (itemList.childDialog != null) {
                        itemList.childDialog.delayedDestroy();
                        itemList.childDialog = null;
                    }
                }

                onCurrentItemChanged: {
                    if (currentItem) {
                        currentItem.menu.closed.connect(resetIndexTimer.restart);
                    }
                }

                Keys.onPressed: event => {
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
                    } else if ((event.key === Qt.Key_Right || event.key === Qt.Key_Return || event.key === Qt.Key_Enter) && itemList.childDialog != null) {
                        windowSystem.forceActive(itemList.childDialog.mainItem);
                        itemList.childDialog.mainItem.focus = true;
                        itemList.childDialog.mainItem.currentIndex = 0;
                    } else if ((event.key === Qt.Key_Right || event.key === Qt.Key_Return || event.key === Qt.Key_Enter) && itemList.childDialog == null
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
                        if (/[\x00-\x1F\x7F]/.test(event.text)) {
                            // We still want to focus it
                            appendSearchText("");
                        } else {
                            appendSearchText(event.text);
                        }
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
