/*
    SPDX-FileCopyrightText: 2014 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

var iconSizes = [PlasmaCore.Units.iconSizes.smallMedium,
                 PlasmaCore.Units.iconSizes.medium,
                 PlasmaCore.Units.iconSizes.large,
                 PlasmaCore.Units.iconSizes.huge,
                 PlasmaCore.Units.iconSizes.large*2,
                 PlasmaCore.Units.iconSizes.enormous,
                 PlasmaCore.Units.iconSizes.enormous*2];

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
