/***************************************************************************
 *   Copyright (C) 2014-2015 by Eike Hein <hein@kde.org>                   *
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
import org.kde.plasma.plasmoid 2.0

import org.kde.plasma.core 2.0 as PlasmaCore

import org.kde.private.desktopcontainment.folder 0.1 as Folder

Folder.SubDialog {
    id: dialog

    visible: false

    property bool containsDrag: {
        if (folderViewDropArea.containsDrag) {
            return true;
        }

        if (folderView.hoveredItem && folderView.hoveredItem.popupDialog) {
            return folderView.hoveredItem.popupDialog.containsDrag;
        }

        return false;
    }

    property QtObject closeTimer: closeTimer
    property QtObject childDialog: (folderView.hoveredItem != null) ? folderView.hoveredItem.popupDialog : null
    property bool containsMouse: folderView.containsMouse || (childDialog != null && childDialog.containsMouse)

    property alias url: folderView.url

    location: PlasmaCore.Types.Floating
    hideOnWindowDeactivate: (childDialog == null)

    onContainsMouseChanged: {
        if (containsMouse) {
            closeTimer.stop();
        } else {
            closeTimer.start();
        }
    }

    mainItem: FolderViewDropArea {
        id: folderViewDropArea

        width: folderView.cellWidth * 3 + (16 * units.devicePixelRatio) // FIXME HACK: Use actual scrollbar width.
        height: folderView.cellHeight * 2

        folderView: folderView

        FolderView {
            id: folderView

            anchors.fill: parent

            isRootView: false
            dialog: dialog

            locked: true

            sortMode: ((plasmoid.configuration.sortMode === 0) ? 1 : plasmoid.configuration.sortMode)
            filterMode: 0

            // TODO: Bidi.
            flow:  GridView.FlowLeftToRight
            layoutDirection: Qt.LeftToRight

            onDraggingChanged: {
                if (!dragging && !dialog.visible) {
                    dialog.destroy();
                }
            }
        }
    }

    data: [
        Timer {
            id: closeTimer

            interval: units.longDuration * 2

            onTriggered: {
                if (childDialog != null) {
                    childDialog.closeTimer.stop();
                    childDialog.visible = false;
                }

                dialog.visible = false;
                delayedDestroy();
            }
        }
    ]

    function requestDestroy() {
        if (folderView.dragging) {
            visible = false;
        } else {
            destroy();
        }
    }

    function delayedDestroy() {
        Qt.callLater(function() {
            itemDialog.destroy();
        });
    }

    Component.onDestruction: {
        closeTimer.stop();
    }
}
