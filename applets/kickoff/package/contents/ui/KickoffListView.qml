/*
    Copyright (C) 2011  Martin Gräßlin <mgraesslin@kde.org>
    Copyright (C) 2012  Gregor Taetzner <gregor@freenet.de>
    Copyright (C) 2015-2018  Eike Hein <hein@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/
import QtQuick 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.plasma.components 2.0 as PlasmaComponents


FocusScope {
    id: view

    signal reset
    signal addBreadcrumb(var model, string title)

    readonly property Item listView: listView
    readonly property Item scrollArea: scrollArea

    property bool showAppsByName: true
    property bool appView: false

    property alias model: listView.model
    property alias delegate: listView.delegate
    property alias currentIndex: listView.currentIndex
    property alias currentItem: listView.currentItem
    property alias count: listView.count
    property alias interactive: listView.interactive
    property alias contentHeight: listView.contentHeight

    property alias move: listView.move
    property alias moveDisplaced: listView.moveDisplaced

    function incrementCurrentIndex() {
        listView.incrementCurrentIndex();
    }

    function decrementCurrentIndex() {
        listView.decrementCurrentIndex();
    }

    Connections {
        target: plasmoid

        function onExpandedChanged() {
            if (!plasmoid.expanded) {
                listView.positionViewAtBeginning();
            }
        }
    }

    PlasmaExtras.ScrollArea {
        id: scrollArea
        frameVisible: false
        anchors.fill: parent

        ListView {
            id: listView
        
            focus: true
            
            keyNavigationWraps: true
            boundsBehavior: Flickable.StopAtBounds
            
            highlight: KickoffHighlight {}
            highlightMoveDuration : 0
            highlightResizeDuration: 0

            delegate: KickoffItem {
                id: delegateItem

                appView: view.appView
                showAppsByName: view.showAppsByName

                onReset: view.reset()
                onAddBreadcrumb: view.addBreadcrumb(model, title)
            }

            section {
                property: "group"
                criteria: ViewSection.FullString
                delegate: SectionDelegate {}
            }

            MouseArea {
                anchors.left: parent.left

                width: scrollArea.viewport.width
                height: parent.height

                id: mouseArea

                property Item pressed: null
                property int pressX: -1
                property int pressY: -1
                property bool tapAndHold: false

                hoverEnabled: true
                acceptedButtons: Qt.LeftButton | Qt.RightButton

                onPressed: {
                    var mapped = listView.mapToItem(listView.contentItem, mouse.x, mouse.y);
                    var item = listView.itemAt(mapped.x, mapped.y);

                    if (!item) {
                        return;
                    }

                    if (mouse.buttons & Qt.RightButton) {
                        if (item.hasActionList) {
                            mapped = listView.contentItem.mapToItem(item, mapped.x, mapped.y);
                            listView.currentItem.openActionMenu(mapped.x, mapped.y);
                        }
                    } else {
                        pressed = item;
                        pressX = mouse.x;
                        pressY = mouse.y;
                    }
                }

                onReleased: {
                    var mapped = listView.mapToItem(listView.contentItem, mouse.x, mouse.y);
                    var item = listView.itemAt(mapped.x, mapped.y);

                    if (item && pressed === item && !tapAndHold) {
                        if (item.appView) {
                            if (mouse.source == Qt.MouseEventSynthesizedByQt) {
                                positionChanged(mouse);
                            }
                            view.state = "OutgoingLeft";
                        } else {
                            item.activate();
                        }

                        listView.currentIndex = -1;
                    }
                    if (tapAndHold && mouse.source == Qt.MouseEventSynthesizedByQt) {
                        if (item.hasActionList) {
                            mapped = listView.contentItem.mapToItem(item, mapped.x, mapped.y);
                            listView.currentItem.openActionMenu(mapped.x, mapped.y);
                        }
                    }
                    pressed = null;
                    pressX = -1;
                    pressY = -1;
                    tapAndHold = false;
                }

                onPositionChanged: {
                    var mapped = listView.mapToItem(listView.contentItem, mouse.x, mouse.y);
                    var item = listView.itemAt(mapped.x, mapped.y);

                    if (item) {
                        listView.currentIndex = item.itemIndex;
                    } else {
                        listView.currentIndex = -1;
                    }

                    if (mouse.source != Qt.MouseEventSynthesizedByQt || tapAndHold) {
                        if (pressed && pressX != -1 && pressed.url && dragHelper.isDrag(pressX, pressY, mouse.x, mouse.y)) {
                            kickoff.dragSource = item;
                            if (mouse.source == Qt.MouseEventSynthesizedByQt) {
                                dragHelper.dragIconSize = units.iconSizes.huge
                                dragHelper.startDrag(kickoff, pressed.url, pressed.decoration);
                            } else {
                                dragHelper.dragIconSize = units.iconSizes.medium
                                dragHelper.startDrag(kickoff, pressed.url, pressed.decoration);
                            }
                            pressed = null;
                            pressX = -1;
                            pressY = -1;
                            tapAndHold = false;
                        }
                    }
                }

                onContainsMouseChanged: {
                    if (!containsMouse) {
                        pressed = null;
                        pressX = -1;
                        pressY = -1;
                        tapAndHold = false;
                    }
                }

                onPressAndHold: {
                    if (mouse.source == Qt.MouseEventSynthesizedByQt) {
                        tapAndHold = true;
                        positionChanged(mouse);
                    }
                }
            }
        }
    }
}
