/***************************************************************************
 *   Copyright 2012 by Sebastian Kügler <sebas@kde.org>                    *
 *   Copyright 2015 by Kai Uwe Broulik <kde@privat.broulik.de>             *
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
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.kquickcontrolsaddons 2.0 as KQuickControlsAddons

Item {
    width: units.gridUnit * 14
    height: actionsColumn.implicitHeight

    PlasmaCore.DataSource {
        id: dataEngine
        engine: "powermanagement"
        connectedSources: ["PowerDevil"]
    }

    function performOperation(what) {
        var service = dataEngine.serviceForSource("PowerDevil");
        var operation = service.operationDescription(what);
        return service.startOperationCall(operation);
    }

    function lockScreen() {
        print("TB locking...");
        performOperation("lockScreen");
    }

    function lockWidgets(lock) {
        plasmoid.action("lock").trigger();
    }

    function logout() {
        print("TB shutdown...");
        performOperation("requestShutDown");
    }

    function containmentSettings() {
        plasmoid.action("configure").trigger();
    }

    function shortcutSettings() {
        print("FIXME: implement shortcut settings");
    }

    function showWidgetsExplorer() {
        plasmoid.action("add widgets").trigger();
    }

    function showActivities() {
        print("TB FIXME: Show Activity Manager");
    }

    PlasmaComponents.Highlight {
        id: highlight
        y: actionsColumn.currentItem ? actionsColumn.currentItem.y : 0
        width: actionsColumn.currentItem ? actionsColumn.currentItem.width : 0
        height: actionsColumn.currentItem ? actionsColumn.currentItem.height : 0
        visible: actionsColumn.currentItem !== null
    }

    Timer {
        id: exitTimer
        interval: 1
        onTriggered: actionsColumn.currentItem = null
    }

    Column {
        id: actionsColumn

        property Item currentItem: null

        width: parent.width
        spacing: 0

        Repeater {
            id: unlockedList
            model: plasmoid.actions
            delegate: ActionDelegate {
                width: parent.width
                objectName: modelData.objectName
                icon: modelData.icon
                text: (modelData.text || "").replace("&", "") // hack to get rid of keyboard accelerator hints
                visible: modelData.visible && modelData.text !== ""
                enabled: modelData.enabled
            }
        }

        ActionDelegate {
            width: parent.width
            objectName: "leave"
            text: i18nd("plasma_toolbox_org.kde.desktoptoolbox", "Leave")
            icon: "system-log-out"
            onClicked: logout()
        }
    }
}
