/*
    SPDX-FileCopyrightText: 2012 Gregor Taetzner <gregor@freenet.de>
    SPDX-FileCopyrightText: 2020 Ivan Čukić <ivan.cukic at kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Layouts 1.1
import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.draganddrop 2.0 as DND

import org.kde.plasma.activityswitcher 1.0 as ActivitySwitcher

import org.kde.activities 0.1 as Activities

DND.DropArea {
    id: root

    width: PlasmaCore.Units.iconSizes.large
    height: PlasmaCore.Units.iconSizes.large

    Layout.maximumWidth: Infinity
    Layout.maximumHeight: Infinity

    Layout.preferredWidth : icon.width + PlasmaCore.Units.smallSpacing + (root.showActivityName ? name.implicitWidth : 0)
    Layout.preferredHeight: inPanel ? PlasmaCore.Units.iconSizeHints.panel : -1

    Layout.minimumWidth: 0
    Layout.minimumHeight: 0

    readonly property bool inPanel: (plasmoid.location === PlasmaCore.Types.TopEdge
        || plasmoid.location === PlasmaCore.Types.RightEdge
        || plasmoid.location === PlasmaCore.Types.BottomEdge
        || plasmoid.location === PlasmaCore.Types.LeftEdge)
    property string activeSource: "Status"
    property bool showActivityName: plasmoid.configuration.showActivityName
    property bool showActivityIcon: plasmoid.configuration.showActivityIcon

    Plasmoid.onActivated: ActivitySwitcher.Backend.toggleActivityManager()
    Plasmoid.preferredRepresentation: Plasmoid.fullRepresentation

    onDragEnter: {
        ActivitySwitcher.Backend.setDropMode(true);
    }

    onDragLeave: {
        ActivitySwitcher.Backend.setDropMode(false);
    }

    Activities.ActivityInfo {
        id: currentActivity
        activityId: ":current"
    }

    MouseArea {
        anchors.fill: parent

        activeFocusOnTab: true

        Keys.onPressed: {
            switch (event.key) {
            case Qt.Key_Space:
            case Qt.Key_Enter:
            case Qt.Key_Return:
            case Qt.Key_Select:
                Plasmoid.activated();
                break;
            }
        }
        Accessible.name: name.text ? i18nc("@info:tooltip", "Current activity is %1", name.text) : ""
        Accessible.description: tooltip.subText
        Accessible.role: Accessible.Button

        onClicked: Plasmoid.activated()
    }

    PlasmaCore.ToolTipArea {
        id: tooltip
        anchors.fill: parent
        mainText: i18n("Show Activity Manager")
        subText: i18n("Click to show the activity manager")
    }

    PlasmaCore.IconItem {
        id: icon
        height: parent.height
        width: height

        source: !root.showActivityIcon ? "activities" :
                currentActivity.icon == "" ? "activities" :
                currentActivity.icon
    }

    PlasmaComponents.Label {
        id: name

        anchors {
            left: icon.right
            leftMargin: PlasmaCore.Units.smallSpacing
        }
        height: parent.height
        width: implicitWidth
        visible: root.showActivityName

        text: currentActivity.name
    }
}

