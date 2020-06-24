/**
 * SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick 2.4
import org.kde.plasma.extras 2.0 as PlasmaExtras

/**
 * Item used in the category list.
 */
MouseArea {
    id: item

    property int textWidth: label.contentWidth
    property int mouseCol
    
    width: parent.width
    height: label.paintedHeight + highlightItemSvg.margins.top + highlightItemSvg.margins.bottom

    Accessible.role: Accessible.MenuItem
    Accessible.name: model.display

    acceptedButtons: Qt.LeftButton

    hoverEnabled: true

    onContainsMouseChanged: {
        if (!containsMouse) {
            updateCurrentItemTimer.stop();
        }
    }

    onPressed: {
        if (!plasmoid.configuration.switchCategoriesOnHover) {
            ListView.view.currentIndex = index;
        }
    }

    onPositionChanged: { // Lazy menu implementation.
        if (!plasmoid.configuration.switchCategoriesOnHover) {
            return;
        }

        mouseCol = mouse.x;

        if (index == ListView.view.currentIndex) {
            updateCurrentItem();
        } else if ((index == ListView.view.currentIndex - 1) && mouse.y < (item.height - 6)
            || (index == ListView.view.currentIndex + 1) && mouse.y > 5) {

            if (mouse.x > ListView.view.eligibleWidth - 5) {
                updateCurrentItem();
            }
        } else if (mouse.x > ListView.view.eligibleWidth) {
            updateCurrentItem();
        }

        updateCurrentItemTimer.start();
    }

    function updateCurrentItem() {
        ListView.view.currentIndex = index;
        ListView.view.eligibleWidth = Math.min(width, mouseCol);
    }

    Timer {
        id: updateCurrentItemTimer

        interval: 50
        repeat: false

        onTriggered: parent.updateCurrentItem()
    }

    PlasmaExtras.Heading {
        id: label

        anchors {
            fill: parent
            leftMargin: highlightItemSvg.margins.left
            rightMargin: highlightItemSvg.margins.right
        }

        elide: Text.ElideRight
        wrapMode: Text.NoWrap
        opacity: 1.0

        level: 5

        text: model.display
    }
}
