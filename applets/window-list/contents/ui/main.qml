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
import QtQuick.Layouts 1.1
import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.extras 2.0 as PlasmaExtras


MouseArea {
    id: main
    anchors.fill: parent
    Layout.minimumWidth: units.gridUnit * 15
    Layout.minimumHeight: units.gridUnit * 25
    Plasmoid.switchWidth: units.gridUnit * 15
    Plasmoid.switchHeight: units.gridUnit * 25
    //property alias data: tasksSource.data;
    property variant desktopList: []
    property int iconSize: units.iconSizes.smallMedium 
    property bool showDesktop: true
    property bool highlight: false

    PlasmaComponents.Highlight {
        id: highlightItem

        property Item trackingItem
        onTrackingItemChanged: {
            if (trackingItem) {
                y = trackingItem.mapToItem(main, 0, 0).y
            }
        }

        hover: true
        width: windowListMenu.width
        x: 0
        y: 0
        height: trackingItem.height
        visible: true
        opacity: highlight ? 1 : 0

        Behavior on opacity {
            NumberAnimation {
                duration: units.longDuration
                easing: Easing.InOutQuad
            }
        }
        Behavior on y {
            NumberAnimation {
                duration: units.longDuration
                easing: Easing.InOutQuad
            }
        }
    }
    Connections {
        target: windowListMenu.flickableItem
        onContentYChanged: {
            highlightItem.trackingItem = null
        }
    }


    function performOperation(op) {
        var service =tasksSource.serviceForSource("");
        var operation = service.operationDescription(op);
        service.startOperationCall(operation);
    }

    function executeJob(operationName, id) {
        var service = tasksSource.serviceForSource("tasks");
        var operation = service.operationDescription(operationName);
operation.Id = id;
        service.startOperationCall(operation);
    }

    function setOnDesktop(id, desktop) {
        var service = tasksSource.serviceForSource("tasks");
        var operation = service.operationDescription("toDesktop");
operation.Id = id;
        operation.desktop = desktop;
        service.startOperationCall(operation);
    }

    Component.onCompleted: {
        var toolTipData = new Object;
        toolTipData["image"] = "preferences-system-window"; 
        toolTipData["mainText"] = i18n("Window List"); 
        toolTipData["subText"] = i18n("Show list of opened windows");
        plasmoid.popupIconToolTip = toolTipData;
        plasmoid.popupIcon = QIcon("preferences-system-windows"); 
    }

    PlasmaCore.DataSource {
        id: tasksSource
        engine: "tasks"
        onSourceAdded: {
            connectSource(source);
        }
        connectedSources: "tasks"
        Component.onCompleted: {
    print("connected");
            connectSource("virtualDesktops");
            main.desktopList = tasksSource.data["virtualDesktops"]["names"];
    print(models.tasks)
        }
    }

    PlasmaCore.SortFilterModel {
        id: tasksModelSortedByDesktop
        sortRole: "desktop"
        sourceModel: tasksSource.models.tasks
    }

    Column {
        id: col

        PlasmaComponents.Highlight {
            hover: menu.focus
            width: windowListMenu.width
            height: actions.height + marginHints.top + marginHints.bottom
            PlasmaComponents.Label {
                id: actions
                text: i18n("Actions")
                anchors.centerIn: parent
            }
        }

        TaskDelegate {
            nameLabel: i18n("Unclutter Windows")
            onClicked: performOperation("unclutter")
        }

        TaskDelegate {
            nameLabel : i18n("Cascade Windows")
            onClicked: performOperation("cascade")
        }
    }

    Menu {
        id: windowListMenu
        anchors {
            top: col.bottom
            left: parent.left
            right: parent.right
            bottom: parent.bottom
        }

        model: tasksModelSortedByDesktop
        section.property: "desktop"
        section.criteria: ViewSection.FullString
        iconSize: main.iconSize
        showDesktop: main.showDesktop
        onItemSelected: main.executeJob("activate", id);
        onExecuteJob: main.executeJob(jobName, id);
        onSetOnDesktop: main.setOnDesktop(id, desktop);
    }
}

