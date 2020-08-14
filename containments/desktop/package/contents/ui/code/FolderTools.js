/***************************************************************************
 *   Copyright (C) 2014 by Eike Hein <hein@kde.org>                        *
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

var iconSizes = [units.iconSizes.smallMedium,
                 units.iconSizes.medium,
                 units.iconSizes.large,
                 units.iconSizes.huge,
                 units.iconSizes.large*2,
                 units.iconSizes.enormous];

function iconSizeFromTheme(size) {
    return iconSizes[size];
}

function effectiveNavDirection(flow, layoutDirection, direction) {
    if (direction == Qt.LeftArrow) {
        if (flow == GridView.FlowLeftToRight) {
            if (layoutDirection == Qt.LeftToRight) {
                return Qt.LeftArrow;
            } else {
                return Qt.RightArrow;
            }
        } else {
            if (layoutDirection == Qt.LeftToRight) {
                return Qt.UpArrow;
            } else {
                return Qt.DownArrow;
            }
        }
    } else if (direction == Qt.RightArrow) {
        if (flow == GridView.FlowLeftToRight) {
            if (layoutDirection == Qt.LeftToRight) {
                return Qt.RightArrow;
            } else {
                return Qt.LeftArrow;
            }
        } else {
            if (layoutDirection == Qt.LeftToRight) {
                return Qt.DownArrow;
            } else {
                return Qt.UpArrow;
            }
        }
    } else if (direction == Qt.UpArrow) {
        if (flow == GridView.FlowLeftToRight) {
            return Qt.UpArrow;
        } else {
            return Qt.LeftArrow;
        }
    } else if (direction == Qt.DownArrow) {
        if (flow == GridView.FlowLeftToRight) {
            return Qt.DownArrow;
        } else {
            return Qt.RightArrow
        }
    }
}

function isFileDrag(event) {
    var taskUrl = event.mimeData.formats.indexOf("text/x-orgkdeplasmataskmanager_taskurl") != -1;
    var arkService = event.mimeData.formats.indexOf("application/x-kde-ark-dndextract-service") != -1;
    var arkPath = event.mimeData.formats.indexOf("application/x-kde-ark-dndextract-path") != -1;

    return (event.mimeData.hasUrls || taskUrl || (arkService && arkPath));
}
