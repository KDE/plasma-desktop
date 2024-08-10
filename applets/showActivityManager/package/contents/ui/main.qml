/*
    SPDX-FileCopyrightText: 2012 Gregor Taetzner <gregor@freenet.de>
    SPDX-FileCopyrightText: 2020 Ivan Čukić <ivan.cukic at kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import org.kde.plasma.plasmoid
import org.kde.plasma.core as PlasmaCore
import org.kde.plasma.components as PlasmaComponents3
import org.kde.draganddrop as DND

import org.kde.plasma.activityswitcher as ActivitySwitcher
import org.kde.kirigami as Kirigami
import org.kde.activities as Activities

PlasmoidItem {
    id: root

    width: Kirigami.Units.iconSizes.large
    height: Kirigami.Units.iconSizes.large

    Layout.maximumWidth: Infinity
    Layout.maximumHeight: Infinity

    Layout.preferredWidth : Math.min(height, Kirigami.Units.iconSizes.huge) + (root.showActivityName ? nameLabel.implicitWidth + 4 * Kirigami.Units.smallSpacing : 0)

    Layout.minimumWidth: 0
    Layout.minimumHeight: 0

    readonly property bool inVertical: Plasmoid.formFactor === PlasmaCore.Types.Vertical
    readonly property bool showActivityName: Plasmoid.configuration.showActivityName
    readonly property bool showActivityIcon: Plasmoid.configuration.showActivityIcon

    Plasmoid.onActivated: ActivitySwitcher.Backend.toggleActivityManager()
    preferredRepresentation: fullRepresentation

    DND.DropArea {
        id: dropArea
        anchors.fill: parent

        onDragEnter: ActivitySwitcher.Backend.setDropMode(true)
        onDragLeave: ActivitySwitcher.Backend.setDropMode(false)

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
            Accessible.name: nameLabel.text ? i18nc("@info:tooltip", "Current activity is %1", nameLabel.text) : ""
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

        Kirigami.Icon {
            id: icon

            anchors {
                verticalCenter: parent.verticalCenter
            }

            x: nameLabel.visible ? Kirigami.Units.smallSpacing :
               (parent.width - icon.width) / 2

            height: Math.min(parent.height,
                             parent.width,
                             Kirigami.Units.iconSizes.huge)
            width: height

            source: !root.showActivityIcon ? "activities" :
                    currentActivity.icon === "" ? "activities" :
                    currentActivity.icon
        }

        PlasmaComponents3.Label {
            id: nameLabel

            anchors {
                left: icon.right
                right: parent.right
                top: parent.top
                bottom: parent.bottom
                leftMargin: Kirigami.Units.smallSpacing
                rightMargin: Kirigami.Units.smallSpacing
            }

            visible: root.showActivityName && !root.inVertical
            elide: Text.ElideRight

            verticalAlignment: Text.AlignVCenter

            text: currentActivity.name
            textFormat: Text.PlainText
        }
    }
}
