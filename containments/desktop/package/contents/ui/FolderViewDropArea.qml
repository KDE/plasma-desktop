/*
    SPDX-FileCopyrightText: 2014-2017 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.4

import org.kde.draganddrop 2.0 as DragDrop
import org.kde.plasma.core 2.0 as PlasmaCore

DragDrop.DropArea {
    id: dropArea

    property Item folderView: null

    function handleDragMove(folderView, pos) {
        // Trigger autoscroll.
        folderView.scrollLeft = (pos.x < (PlasmaCore.Units.largeSpacing * 3));
        folderView.scrollRight = (pos.x > width - (PlasmaCore.Units.largeSpacing * 3));
        folderView.scrollUp = (pos.y < (PlasmaCore.Units.largeSpacing * 3));
        folderView.scrollDown = (pos.y > height - (PlasmaCore.Units.largeSpacing * 3));

        folderView.handleDragMove(pos.x, pos.y);
    }

    function handleDragEnd(folderView) {
        // Cancel autoscroll.
        folderView.scrollLeft = false;
        folderView.scrollRight = false;
        folderView.scrollUp = false;
        folderView.scrollDown = false;

        folderView.endDragMove();
    }

    onDragMove: {
        // TODO: We should reject drag moves onto file items that don't accept drops
        // (cf. QAbstractItemModel::flags() here, but DeclarativeDropArea currently
        // is currently incapable of rejecting drag events.

        if (folderView) {
            handleDragMove(folderView, mapToItem(folderView, event.x, event.y));
        }
    }

    onDragLeave: {
        if (folderView) {
            handleDragEnd(folderView);
        }
    }

    onDrop: {
        if (folderView) {
            handleDragEnd(folderView);

            folderView.drop(folderView, event, mapToItem(folderView, event.x, event.y));
        }
    }
}
