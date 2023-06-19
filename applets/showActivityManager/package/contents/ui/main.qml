/*
    SPDX-FileCopyrightText: 2012 Gregor Taetzner <gregor@freenet.de>
    SPDX-FileCopyrightText: 2020 Ivan Čukić <ivan.cukic at kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Layouts 1.1
import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 3.0 as PlasmaComponents3
import org.kde.draganddrop 2.0 as DND

import org.kde.plasma.activityswitcher 1.0 as ActivitySwitcher
import org.kde.kirigami 2.20 as Kirigami
import org.kde.activities 0.1 as Activities

PlasmoidItem {
    id: root

    width: Kirigami.Units.iconSizes.large
    height: Kirigami.Units.iconSizes.large

    Layout.maximumWidth: Infinity
    Layout.maximumHeight: Infinity

    Layout.preferredWidth : icon.width + Kirigami.Units.smallSpacing + (root.showActivityName ? name.implicitWidth : 0)

    Layout.minimumWidth: 0
    Layout.minimumHeight: 0

    readonly property bool inPanel: (plasmoid.location === PlasmaCore.Types.TopEdge
        || plasmoid.location === PlasmaCore.Types.RightEdge
        || plasmoid.location === PlasmaCore.Types.BottomEdge
        || plasmoid.location === PlasmaCore.Types.LeftEdge)
    readonly property bool inVertical: Plasmoid.formFactor === PlasmaCore.Types.Vertical
    property string activeSource: "Status"
    property bool showActivityName: plasmoid.configuration.showActivityName
    property bool showActivityIcon: plasmoid.configuration.showActivityIcon

    Plasmoid.onActivated: ActivitySwitcher.Backend.toggleActivityManager()
    preferredRepresentation: fullRepresentation

    DND.DropArea {
        id: dropArea
        anchors.fill: parent

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
            height: Math.min(parent.height, parent.width)
            width: height

            source: !root.showActivityIcon ? "activities" :
                    currentActivity.icon == "" ? "activities" :
                    currentActivity.icon
        }

        PlasmaComponents3.Label {
            id: name

            anchors {
                left: icon.right
                leftMargin: Kirigami.Units.smallSpacing
            }
            height: parent.height
            width: implicitWidth
            visible: root.showActivityName && !root.inVertical

            text: currentActivity.name
        }
    }
}
