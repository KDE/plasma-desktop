/*
    SPDX-FileCopyrightText: 2014-2017 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick

import org.kde.draganddrop as DragDrop
import org.kde.kirigami as Kirigami

DragDrop.DropArea {
    id: dropArea

    property Item folderView: null

    function handleDragMove(folderView, pos) {
        // Trigger autoscroll.
        folderView.scrollLeft = (pos.x < (Kirigami.Units.gridUnit * 3));
        folderView.scrollRight = (pos.x > width - (Kirigami.Units.gridUnit * 3));
        folderView.scrollUp = (pos.y < (Kirigami.Units.gridUnit * 3));
        folderView.scrollDown = (pos.y > height - (Kirigami.Units.gridUnit * 3));

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

    onDragMove: event => {
        // TODO: We should reject drag moves onto file items that don't accept drops
        // (cf. QAbstractItemModel::flags() here, but DeclarativeDropArea currently
        // is currently incapable of rejecting drag events.

        if (folderView) {
            handleDragMove(folderView, mapToItem(folderView, event.x, event.y));
        }
    }

    onDragLeave: event => {
        if (folderView) {
            handleDragEnd(folderView);
        }
    }

    onDrop: event => {
        if (folderView) {
            handleDragEnd(folderView);

            folderView.drop(folderView, event, mapToItem(folderView, event.x, event.y));
        }
    }
}
