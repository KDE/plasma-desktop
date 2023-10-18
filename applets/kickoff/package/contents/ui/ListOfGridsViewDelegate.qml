/*
    SPDX-FileCopyrightText: 2023 Tanbir Jishan <tantalising007@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15
import QtQml 2.15

import org.kde.plasma.extras as PlasmaExtras

import org.kde.ksvg 1.0 as KSvg

KickoffGridView {
    id: root

    property string allAppsSection: undefined
    property int gridIndex: -1                  // To know the index of the grid when used inside a listivew
    property bool isCurrentSectionGrid: false
    property bool isSearchFieldActive: false // needed since check doesn't work here when gridview used in all apps
    property ListView parentView: undefined // neeeded when used inside a listview e.g. all apps view

    signal showSectionView(string sectionName)

    // When nested insdie a listivew, other items will still treat it as a delegate(because this was the truth always till now), so just call the appropriate function
    readonly property QtObject action: QtObject {
        function triggered() {
            view.currentItem.action.triggered();
            view.currentItem.forceActiveFocus();
        }
    }

    view.height : view.cellHeight * Math.ceil(count / view.columns)
    view.implicitHeight: view.contentHeight
    blockTargetWheel: false
    view.highlight: PlasmaExtras.Highlight {
        visible: root.isCurrentSectionGrid
        // The default Z value for delegates is 1. The default Z value for the section delegate is 2.
        // The highlight gets a value of 3 while the drag is active and then goes back to the default value of 0.
        z: root.currentItem && root.currentItem.Drag.active ?
            3 : 0
        width: view.cellWidth
        height: view.cellHeight
        pressed: view.currentItem && view.currentItem.isPressed
        active: view.activeFocus
            || (kickoff.contentArea === root
                && kickoff.searchField.activeFocus)
    }

    delegate: KickoffGridDelegate {
        id: itemDelegate
        width: view.cellWidth
        Accessible.role: Accessible.Cell

        Connections {
            target: itemDelegate.mouseArea
            function onPositionChanged(mouse) {
                if (!root.parentView.movedWithKeyboard) {
                    root.parentView.currentIndex = root.gridIndex
                    root.parentView.currentItem.forceActiveFocus()
                }
            }
        }
    }
}
