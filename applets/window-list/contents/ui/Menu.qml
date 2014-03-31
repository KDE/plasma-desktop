/*
 * Copyright 2013  Heena Mahour <heena393@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http: //www.gnu.org/licenses/>.
 */
import QtQuick 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.extras 2.0 as PlasmaExtras

PlasmaExtras.ScrollArea {
    id: menu

    property alias model: menuListView.model
    property alias section: menuListView.section
    property int iconSize: units.iconSizes.small 
    property bool showDesktop: true
    signal itemSelected(int id)
    signal executeJob(string jobName, int id)
    signal setOnDesktop(int id, int desktop)

    ListView {
        id: menuListView
        anchors.top: menu.top
        anchors.left: menu.left
        anchors.bottom: menu.bottom
        focus: true
        section.property: desktop
        section.criteria: ViewSection.FullString
        spacing:0

        section.delegate: PlasmaComponents.Highlight {
            hover: menu.focus
            width: menu.width
            height: sectionDelegateLabel.height + marginHints.top + marginHints.bottom

            PlasmaComponents.Label {
                id: sectionDelegateLabel
                text: section > 0 ? i18n("Desktop") + section : i18n("On all desktops")
                anchors.centerIn: parent
            }
        }

        highlight: PlasmaComponents.Highlight {
            hover: false
            width: menuListView.width
        }

        delegate: TaskDelegate {
            id: menuItemDelegate
            width: menuListView.width
            nameLabel: DisplayRole
            desktop: Desktop
            icon: DecorationRole
            active: Active
            iconSize: menu.iconSize
            showDesktop: menu.showDesktop
            onClicked: menu.itemSelected(Id);
            onExecuteJob: menu.executeJob(jobName, Id);
            onSetOnDesktop: menu.setOnDesktop(Id, desktop);
            onActiveChanged: {
                if (active) {
                    menuListView.currentIndex = index
                }
            }
        }
    }
}
