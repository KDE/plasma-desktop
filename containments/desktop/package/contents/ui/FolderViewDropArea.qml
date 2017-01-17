/***************************************************************************
 *   Copyright (C) 2014-2017 by Eike Hein <hein@kde.org>                   *
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

import QtQuick 2.4

import org.kde.draganddrop 2.0 as DragDrop

DragDrop.DropArea {
    id: dropArea

    property Item folderView: null

    function handleDragMove(folderView, pos) {
        // Trigger autoscroll.
        folderView.scrollLeft = (pos.x < (units.largeSpacing * 3));
        folderView.scrollRight = (pos.x > width - (units.largeSpacing * 3));
        folderView.scrollUp = (pos.y < (units.largeSpacing * 3));
        folderView.scrollDown = (pos.y > height - (units.largeSpacing * 3));

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
