/*
    SPDX-FileCopyrightText: 2023 Tanbir Jishan <tantalising007@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Templates as T

import org.kde.plasma.extras as PlasmaExtras

KickoffGridView {
    id: root

    property int gridIndex: -1                  // To know the index of the grid when used inside a listivew
    property bool isCurrentSectionGrid: false
    property bool isSearchFieldActive: false // needed since check doesn't work here when gridview used in all apps
    property ListView parentView // needed when used inside a listview e.g. all apps view

    signal showSectionView(string sectionName)

    // When nested insdie a listivew, other items will still treat it as a delegate(because this was the truth always till now), so just call the appropriate function
    readonly property QtObject action: QtObject {
        function triggered(): void {
            root.view.currentItem.action.triggered();
            root.view.currentItem.forceActiveFocus();
        }
    }

    view.height: view.cellHeight * Math.ceil(count / view.columns)
    view.implicitHeight: view.contentHeight
    blockTargetWheel: false

    delegate: KickoffGridDelegate {
        id: itemDelegate
        width: view.cellWidth
        Accessible.role: Accessible.Cell

        background: Loader {
            active: itemDelegate.GridView.isCurrentItem && root.isCurrentSectionGrid
            sourceComponent: PlasmaExtras.Highlight {
                anchors.fill: parent
                z: (root.Drag.active ?? false) ? 3 : 0
                hovered: true

                pressed: itemDelegate.down

                active: root.view.activeFocus ||
                        (kickoff.contentArea === root && kickoff.searchField.activeFocus)
            }
        }

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
