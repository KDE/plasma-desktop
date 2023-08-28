/*
    SPDX-FileCopyrightText: 2013-2014 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Controls as QQC2

import org.kde.kquickcontrolsaddons 2.0
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.kirigami 2 as Kirigami

QQC2.ScrollView {
    id: itemList

    property real minimumWidth: Kirigami.Units.gridUnit * 14
    property real maximumWidth: minimumWidth * 2

    width: minimumWidth
    focus: true

    property Item focusParent: null
    property alias dialog: listView.dialog
    property QtObject childDialog: null
    property bool iconsEnabled: false
    property int itemHeight: Math.ceil((Math.max(Kirigami.Units.iconSizes.sizeForLabels, Kirigami.Units.iconSizes.small)
        + Math.max(highlightItemSvg.margins.top + highlightItemSvg.margins.bottom,
        listItemSvg.margins.top + listItemSvg.margins.bottom)) / 2) * 2
    property int separatorHeight: model.sorted === true ? 0 : lineSvg.horLineHeight + (2 * Kirigami.Units.smallSpacing)

    property alias currentIndex: listView.currentIndex
    property alias currentItem: listView.currentItem
    readonly property alias listView: listView
    property alias keyNavigationWraps: listView.keyNavigationWraps
    // Whether to create a child dialog if there are any child items
    property alias showChildDialogs: listView.showChildDialogs
    property alias model: listView.model
    property alias count: listView.count

    Connections {
        target: kicker
        function onExpandedChanged(expanded) {
            if (!expanded) {
                listView.currentIndex = -1; // Will destroy childDialog if any
            }
        }
    }

    contentItem: ListView {
        id: listView

        property bool showChildDialogs: true
        property int eligibleWidth: width
        // The dialog contains this ListView
        property QtObject dialog: null

        width: itemList.availableWidth

        currentIndex: -1
        focus: currentIndex !== -1

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

        KeyNavigation.up: itemList.KeyNavigation.up
        KeyNavigation.down: itemList.KeyNavigation.down
        KeyNavigation.left: itemList.KeyNavigation.left
        KeyNavigation.right: itemList.KeyNavigation.right

        onCountChanged: {
            currentIndex = -1;
        }

        onCurrentIndexChanged: {
            if (!currentItem?.hasChildren || !kicker.expanded || currentIndex === -1) {
                itemList.childDialog?.destroy();
            } else if (listView.showChildDialogs) {
                listView.spawnDialog();
            } else {
                listView.showChildDialogs = true; // Restore
            }
        }

        function spawnDialog(focusOnSpawn = false) {
            if (!kicker.expanded || model === undefined || currentIndex == -1) {
                return;
            }

            if (itemList.childDialog !== null) {
                return;
            }

            itemList.childDialog = itemListDialogComponent.createObject(itemList, {
                "focusParent": itemList,
            });

            if (focusOnSpawn) {
                itemList.childDialog.mainItem.showChildDialogs = false;
                itemList.childDialog.mainItem.currentIndex = 0;
            }
        }
    }
}
