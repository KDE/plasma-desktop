/***************************************************************************
 *   Copyright (C) 2016 Kai Uwe Broulik <kde@privat.broulik.de>            *
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

import org.kde.plasma.core 2.0 as PlasmaCore

import "code/tools.js" as TaskTools

Item {
    id: background

    Item {
        id: progress
        anchors {
            top: parent.top
            left: parent.left
            bottom: parent.bottom
        }

        width: parent.width * (task.smartLauncherItem.progress / 100)
        clip: true

        PlasmaCore.FrameSvgItem {
            id: progressFrame
            width: background.width
            height: background.height

            imagePath: "widgets/tasks"
            prefix: TaskTools.taskPrefix("progress").concat(TaskTools.taskPrefix("hover"))
        }
    }
}
