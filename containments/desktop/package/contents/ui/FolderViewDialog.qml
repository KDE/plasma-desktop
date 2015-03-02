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

    mainItem: FolderView {
        id: folderView

        width: cellWidth * 3 + 10 // FIXME HACK: Use actual scrollbar width.
        height: cellHeight * 2

        isRootView: false

        locked: true

        sortMode: ((plasmoid.configuration.sortMode == 0) ? 1 : plasmoid.configuration.sortMode)
        filterMode: 0

        // TODO: Bidi.
        flow:  GridView.FlowLeftToRight
        layoutDirection: Qt.LeftToRight
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

    function delayedDestroy() {
        var timer = Qt.createQmlObject('import QtQuick 2.0; Timer { onTriggered: itemDialog.destroy() }', itemDialog);
        timer.interval = 0;
        timer.start();
    }

    Component.onDestruction: {
        closeTimer.stop();
    }
}
