/*
    SPDX-FileCopyrightText: 2012 Gregor Taetzner <gregor@freenet.de>
    SPDX-FileCopyrightText: 2020 Ivan Čukić <ivan.cukic at kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
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
    height: PlasmaCore.Units.iconSizes.large
    width: PlasmaCore.Units.iconSizes.large

    property bool showActivityName: Plasmoid.configuration.showActivityName
    property bool showActivityIcon: Plasmoid.configuration.showActivityIcon

    readonly property bool inPanel: (Plasmoid.location === PlasmaCore.Types.TopEdge
        || Plasmoid.location === PlasmaCore.Types.RightEdge
        || Plasmoid.location === PlasmaCore.Types.BottomEdge
        || Plasmoid.location === PlasmaCore.Types.LeftEdge)

    Layout.maximumWidth: Infinity
    Layout.maximumHeight: Infinity

    Layout.preferredWidth : icon.width + PlasmaCore.Units.smallSpacing + (root.showActivityName ? name.implicitWidth : 0)
    Layout.preferredHeight: inPanel ? PlasmaCore.Units.iconSizeHints.panel : -1

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
            leftMargin: PlasmaCore.Units.smallSpacing
        }
    }

}

