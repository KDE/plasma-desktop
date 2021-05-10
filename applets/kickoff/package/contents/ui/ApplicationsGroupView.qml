/*
    Copyright (C) 2011  Martin Gräßlin <mgraesslin@kde.org>
    Copyright (C) 2012  Gregor Taetzner <gregor@freenet.de>
    Copyright 2014 Sebastian Kügler <sebas@kde.org>
    Copyright (C) 2015-2018  Eike Hein <hein@kde.org>
    Copyright (C) 2021 by Mikel Johnson <mikel5764@gmail.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/
import QtQuick 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 3.0 as PlasmaComponents3
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.plasma.private.kicker 0.1 as Kicker

FocusScope {
    id: appViewContainer
    property QtObject activatedSection: null
    property string newBreadcrumbName: ""
    signal appModelChange()

    objectName: "ApplicationsGroupView"

    property ListView listView: applicationsView.listView

    function keyNavUp() {
        return applicationsView.keyNavUp();
    }

    function keyNavDown() {
        return applicationsView.keyNavDown();
    }

    function activateCurrentIndex() {
        applicationsView.activatedItem = applicationsView.currentItem
        applicationsView.moveRight()
    }

    function openContextMenu() {
        applicationsView.currentItem.openActionMenu();
    }

    function reset() {
        applicationsView.model = rootModel;
    }

    function refreshed() {
        reset();
        updatedLabelTimer.running = true;
    }

    Connections {
        target: plasmoid
        function onExpandedChanged() {
            if (!plasmoid.expanded) {
                reset();
            }
        }
    }

    Kicker.TriangleMouseFilter {
        anchors.fill: parent

        edge: LayoutMirroring.enabled ? Qt.LeftEdge : Qt.RightEdge

        KickoffListView {
            id: applicationsView
            isManagerMode: true
            anchors.fill: parent

            property Item activatedItem: null
            property var newModel: null

            Behavior on opacity {
                NumberAnimation {
                    duration: PlasmaCore.Units.longDuration
                    easing.type: Easing.InOutQuad
                }
            }

            focus: true

            appView: true

            model: rootModel

            function moveRight() {
                var childModel = activatedItem.activate()
                if (childModel != null) {
                    appViewContainer.activatedSection = childModel.model
                    appViewContainer.newBreadcrumbName = childModel.name
                    appViewContainer.appModelChange()
                }
            }

            onReset: appViewContainer.reset()
        }
    }

    // Displays text when application list gets updated
    Timer {
        id: updatedLabelTimer
        // We want to have enough time to show that applications have been updated even for those who disabled animations
        interval: 1500
        running: false
        repeat: true

        onRunningChanged: {
            if (running) {
                updatedLabel.opacity = 1;
                applicationsView.listView.opacity = 0;
            }
        }
        onTriggered: {
            updatedLabel.opacity = 0;
            applicationsView.listView.opacity = 1;
            running = false;
        }
    }

    PlasmaExtras.PlaceholderMessage {
        id: updatedLabel
        width: parent.width - (PlasmaCore.Units.largeSpacing * 4)
        text: i18n("Updating applications...")
        iconName: "view-refresh"
        opacity: 0
        visible: opacity != 0
        anchors.centerIn: parent

        Behavior on opacity {
            NumberAnimation {
                duration: PlasmaCore.Units.shortDuration
                easing.type: Easing.InOutQuad
            }
        }
    }

    Component.onCompleted: {
        rootModel.cleared.connect(refreshed);
    }

} // appViewContainer
