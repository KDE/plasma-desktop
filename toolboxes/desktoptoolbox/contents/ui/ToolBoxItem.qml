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
import QtQuick.Layouts 1.1

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents

ListView {
    id: menu
    width: units.gridUnit * 14
    height: contentHeight

    currentIndex: -1
    focus: true
    keyNavigationWraps: true

    onVisibleChanged: currentIndex = -1

    // needs to be on released, otherwise Dashboard hides because it already gained focus
    // because of the dialog closing right on the key *press* event
    Keys.onReleased: {
        if (event.key === Qt.Key_Escape) {
            main.open = false
            event.accepted = true
        }
    }

    Keys.onPressed: {
        if (event.key === Qt.Key_Return || event.key === Qt.Key_Enter) {
            if (currentIndex >= 0) {
                if (model[currentIndex].operation) {
                    performOperation(model[currentIndex].operation)
                } else {
                    model[currentIndex].trigger()
                }
            }
            main.open = false
            event.accepted = true
        }
    }

    function performOperation(what) {
        var service = dataEngine.serviceForSource("PowerDevil");
        var operation = service.operationDescription(what);
        return service.startOperationCall(operation);
    }

    highlightMoveDuration: 0
    highlightResizeDuration: 0
    highlight: PlasmaComponents.Highlight { }

    PlasmaCore.DataSource {
        id: dataEngine
        engine: "powermanagement"
        connectedSources: ["PowerDevil", "Sleep States"]
    }

    model: {
        var model = []
        var actions = plasmoid.actions
        for (var i = 0, j = actions.length; i < j; ++i) {
            var action = actions[i]
            if (action && action.visible && action.text !== "") {
                model.push(action)
            }
        }

        if (dataEngine.data["Sleep States"].LockScreen) {
            model.push({
                text: i18nd("plasma_toolbox_org.kde.desktoptoolbox", "Lock Screen"),
                icon: "system-lock-screen",
                visible: true,
                enabled: true,
                operation: "lockScreen"
            })
        }

        if (dataEngine.data["Sleep States"].Logout) {
            model.push({
                text: i18nd("plasma_toolbox_org.kde.desktoptoolbox", "Leave"),
                icon: "system-log-out",
                visible: true,
                enabled: true,
                operation: "requestShutDown" // cannot put function() into a model :(
            })
        }

        return model
    }

    delegate: MouseArea {
        width: menu.width
        height: labelRow.implicitHeight + units.smallSpacing * 2
        hoverEnabled: true
        enabled: modelData.enabled
        opacity: modelData.enabled ? 1 : 0.5

        onEntered: menu.currentIndex = index
        onExited: menu.currentIndex = -1

        onClicked: {
            main.open = false
            if (modelData.operation) {
                performOperation(modelData.operation)
            } else {
                modelData.trigger()
            }
        }

        Accessible.role: Accessible.MenuItem
        Accessible.name: textLabel.text

        RowLayout {
            id: labelRow
            anchors {
                left: parent.left
                right: parent.right
                margins: units.smallSpacing
                verticalCenter: parent.verticalCenter
            }
            spacing: units.smallSpacing

            PlasmaCore.IconItem {
                implicitWidth: units.iconSizes.smallMedium
                implicitHeight: implicitWidth
                source: modelData.icon
                Accessible.ignored: true
            }

            PlasmaComponents.Label {
                id: textLabel
                Layout.fillWidth: true
                text: modelData.text.replace("&", "") // hack to get rid of keyboard accelerator hints
                elide: Text.ElideRight
                Accessible.ignored: true
            }
        }
    }
}
