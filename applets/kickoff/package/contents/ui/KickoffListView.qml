/*
    Copyright (C) 2011  Martin Gräßlin <mgraesslin@kde.org>
    Copyright (C) 2012  Gregor Taetzner <gregor@freenet.de>
    Copyright (C) 2015-2018  Eike Hein <hein@kde.org>
    Copyright (C) 2021 by Mikel Johnson <mikel5764@gmail.com>

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
import org.kde.plasma.components 3.0 as PC3

FocusScope {
    id: view

    signal reset
    signal addBreadcrumb(var model, string title)

    readonly property Item listView: listView

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

    // left sidebar app list
    property bool isManagerMode: false

    // left sidebar places list
    property bool isExternalManagerMode: false

    property alias section: listView.section

    // Accessibility NOTE: We don't name panes because doing so would be annoying and redundant

    property bool upsideDown: false

    function incrementCurrentIndex() {
        if (listView.currentIndex < listView.count - 1) {
            listView.incrementCurrentIndex();
            return true;
        } else {
            return false;
        }
    }
    function decrementCurrentIndex() {
        if (listView.currentIndex > 0) {
            listView.decrementCurrentIndex();
            return true;
        } else {
            return false;
        }
    }
    function keyNavDown() {
        if (upsideDown) {
            return view.decrementCurrentIndex()
        } else {
            return view.incrementCurrentIndex()
        }
    }

    function keyNavUp() {
        if (upsideDown) {
            return view.incrementCurrentIndex()
        } else {
            return view.decrementCurrentIndex()
        }
    }

    Connections {
        target: plasmoid

        function onExpandedChanged() {
            if (!plasmoid.expanded) {
                listView.positionAtBeginning()
            }
        }
    }

    // This only applies to the left sidebar
    // If we're hovering over items quickly they change instantly
    // however if we stop on one item (section) we add a delay
    // so that we can stop accidental category switching when going from left sidebar to the right one
    // that way you can move your cursor diagonally without switching sections
    // and still have highlight following instantly when quickly hovering over sections
    // thus preventing feeling of sluggishness

    Timer {
        id: changedIndexRecently
        // this is an interaction and not an animation, so we want it as a constant
        interval: 300
        repeat: false
    }

    PC3.ScrollView {
        anchors.fill: parent
        PC3.ScrollBar.horizontal.visible: false
        focus: true

        ListView {
            currentIndex: 0
            onCurrentIndexChanged: {
                if (currentIndex != -1) {
                    // stop reporting list count when index changes
                    accessibilityCount = false
                    if (view.isManagerMode || view.isExternalManagerMode) {
                        changedIndexRecently.restart()
                    }
                    if (view.isExternalManagerMode) {
                        model.trigger(currentIndex)
                        return;
                    }
                    if (view.isManagerMode && currentItem.appView && currentItem.modelChildren) {
                        view.activatedItem = view.currentItem;
                        view.moveRight();
                    }
                }
            }
            id: listView
            property int listBeginningMargin: currentSection == "" && (!view.appView || view.isManagerMode) ? PlasmaCore.Units.largeSpacing : 0 // don't add margin in the right app view or when sections are present
            property int listEndMargin: (contentHeight + listBeginningMargin) > parent.height ? PlasmaCore.Units.largeSpacing : 0
            clip: currentSection == "" && view.appView && !view.isManagerMode //clip only in the right app view where breadcrumb is present

            topMargin: view.upsideDown ? listEndMargin : listBeginningMargin
            bottomMargin: view.upsideDown ? listBeginningMargin : listEndMargin

            focus: true

            verticalLayoutDirection: view.upsideDown ? ListView.BottomToTop : ListView.TopToBottom

            // Currently narrator only notifies about focus changes
            // That means that if item has focus narrator won't notify about name/description changes, like count changing
            // which is most apparent with submenus
            // We work around this by having the first focused item having the list count and name as a description
            // When we unfocus it goes back to only reporting it's name
            // That way we create a seamless experience where when model changes we always report the new item count

            // Determines whether or not we tell the amount of items in the list
            property bool accessibilityCount: true

            // we report item amount when model changes
            onModelChanged: {
                accessibilityCount = true
            }

            // and also when we focus on our list
            onActiveFocusChanged: {
                accessibilityCount = true
            }

            // Let root handle keyboard interaction
            Keys.forwardTo: [root]

            function positionAtBeginning() {
                positionViewAtBeginning();
                // positionViewAtBeginning doesn't account for margins
                // move content manually only if it overflows
                if (visibleArea.heightRatio !== 1.0) {
                    if (view.upsideDown) {
                        contentY += topMargin;
                    } else {
                        contentY -= topMargin;
                    }
                }
                currentIndex = 0;
            }
            boundsBehavior: Flickable.StopAtBounds
            property bool hasKeyboardFocus: navigationMethod.inSearch || (keyboardNavigation.state == (view.isManagerMode || view.isExternalManagerMode  ? "LeftColumn" : "RightColumn"))

            highlight: Item {
                opacity: navigationMethod.state != "keyboard" || (listView.hasKeyboardFocus && listView.activeFocus) ? 1 : 0.5
                PlasmaCore.FrameSvgItem {
                    anchors {
                        fill: parent
                        leftMargin: PlasmaCore.Units.smallSpacing * 4
                        rightMargin: PlasmaCore.Units.smallSpacing * 4
                    }
                    imagePath: "widgets/viewitem"
                    prefix: "hover"
                }
            }
            highlightMoveDuration : 0
            highlightResizeDuration: 0

            delegate: KickoffItem {
                id: delegateItem
                isManagerMode: view.isManagerMode
                appView: view.appView

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

                width: parent.width
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
                            if (isManagerMode && view.currentItem && !view.currentItem.modelChildren) {
                                view.activatedItem = view.currentItem;
                                view.moveRight();
                            } else if (!isManagerMode && item.modelChildren) {
                                view.state = "OutgoingLeft";
                            } else if (!isManagerMode) {
                                item.activate();
                            }
                        } else {
                            item.activate();
                        }
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
                        navigationMethod.state = "mouse"
                        if (!navigationMethod.inSearch) {
                            if (view.isManagerMode || view.isExternalManagerMode) {
                                keyboardNavigation.state = "LeftColumn"
                            } else {
                                keyboardNavigation.state = "RightColumn"
                            }
                        }
                        if (kickoff.dragSource == null || kickoff.dragSource == item) {
                            listView.currentIndex = item.itemIndex;
                        }
                    }

                    if (mouse.source != Qt.MouseEventSynthesizedByQt || tapAndHold) {
                        if (pressed && pressX != -1 && pressed.url && dragHelper.isDrag(pressX, pressY, mouse.x, mouse.y)) {
                            kickoff.dragSource = item;
                            if (mouse.source == Qt.MouseEventSynthesizedByQt) {
                                dragHelper.dragIconSize = PlasmaCore.Units.iconSizes.huge
                                dragHelper.startDrag(kickoff, pressed.url, pressed.decoration);
                            } else {
                                dragHelper.dragIconSize = PlasmaCore.Units.iconSizes.medium
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
