/*
 *   Copyright 2012 Gregor Taetzner <gregor@freenet.de>
 *   Copyright 2020 Ivan Čukić <ivan.cukic at kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2 or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

import QtQuick 2.0
import QtQuick.Layouts 1.1
import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.draganddrop 2.0 as DND

import org.kde.plasma.activityswitcher 1.0 as ActivitySwitcher

import org.kde.activities 0.1 as Activities

DND.DropArea {
    id: root
    property string activeSource: "Status"
    height: units.iconSizes.large
    width: units.iconSizes.large

    property bool showActivityName: plasmoid.configuration.showActivityName
    property bool showActivityIcon: plasmoid.configuration.showActivityIcon

    readonly property bool inPanel: (plasmoid.location === PlasmaCore.Types.TopEdge
        || plasmoid.location === PlasmaCore.Types.RightEdge
        || plasmoid.location === PlasmaCore.Types.BottomEdge
        || plasmoid.location === PlasmaCore.Types.LeftEdge)

    Layout.maximumWidth: Infinity
    Layout.maximumHeight: Infinity

    Layout.preferredWidth : icon.width + units.smallSpacing + (root.showActivityName ? name.implicitWidth : 0)
    Layout.preferredHeight: inPanel ? units.iconSizeHints.panel : -1

    Layout.minimumWidth: 0
    Layout.minimumHeight: 0

    Plasmoid.preferredRepresentation: Plasmoid.fullRepresentation

    MouseArea {
        anchors.fill: parent
        onClicked: {
            ActivitySwitcher.Backend.toggleActivityManager()
        }
    }

    onDragEnter: {
        ActivitySwitcher.Backend.setDropMode(true);
    }

    onDragLeave: {
        ActivitySwitcher.Backend.setDropMode(false);
    }

    PlasmaCore.ToolTipArea {
        id: tooltip
        mainText: i18n("Show Activity Manager")
        subText: i18n("Click to show the activity manager")
        anchors.fill: parent
    }

    Activities.ActivityInfo {
        id: currentActivity
        activityId: ":current"
    }

    PlasmaCore.IconItem {
        id: icon
        source: !root.showActivityIcon ? "activities" :
                currentActivity.icon == "" ? "activities" :
                currentActivity.icon

        height: parent.height
        width: height
    }

    PlasmaComponents.Label {
        id: name

        text: currentActivity.name
        height: parent.height
        width: implicitWidth

        visible: root.showActivityName

        anchors {
            left: icon.right
            leftMargin: units.smallSpacing
        }
    }

}

