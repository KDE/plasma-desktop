/*
    Copyright (C) 2011  Martin Gräßlin <mgraesslin@kde.org>
    Copyright (C) 2012  Gregor Taetzner <gregor@freenet.de>
    Copyright 2014 Sebastian Kügler <sebas@kde.org>
    Copyright (C) 2015  Eike Hein <hein@kde.org>

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
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.plasma.components 2.0 as PlasmaComponents

Item {
    id: appViewContainer

    anchors.fill: parent

    objectName: "ApplicationsView"

    property ListView listView: applicationsView

    function decrementCurrentIndex() {
        applicationsView.decrementCurrentIndex();
    }

    function incrementCurrentIndex() {
        applicationsView.incrementCurrentIndex();
    }

    function activateCurrentIndex(start) {
        if (!applicationsView.currentItem.modelChildren) {
            if (!start) {
                return;
            }
        }
        appViewScrollArea.state = "OutgoingLeft";
    }

    function openContextMenu() {
        applicationsView.currentItem.openActionMenu();
    }

    function deactivateCurrentIndex() {
        if (crumbModel.count > 0) { // this is not the case when switching from the "Applications" to the "Favorites" tab using the "Left" key
            breadcrumbsElement.children[crumbModel.count-1].clickCrumb();
            appViewScrollArea.state = "OutgoingRight";
            return true;
        }
        return false;
    }

    function reset() {
        applicationsView.model = rootModel;
        applicationsView.positionViewAtBeginning();
        applicationsView.clearBreadcrumbs();
    }

    function refreshed() {
        reset();
        updatedLabelTimer.running = true;
    }

    Connections {
        target: plasmoid
        onExpandedChanged: {
            if (!expanded) {
                reset();
            }
        }
    }

    Item {
        id: crumbContainer

        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
        }
        height: childrenRect.height

        Behavior on opacity { NumberAnimation { duration: units.longDuration } }

        Flickable {
            id: breadcrumbFlickable
            anchors {
                top: parent.top
                left: parent.left
                right: parent.right
            }
            height: breadcrumbsElement.height
            boundsBehavior: Flickable.StopAtBounds

            contentWidth: breadcrumbsElement.width
            pixelAligned: true
            //contentX: contentWidth - width

            // HACK: Align the content to right for RTL locales
            leftMargin: LayoutMirroring.enabled ? Math.max(0, width - contentWidth) : 0

            PlasmaComponents.ButtonRow {
                id: breadcrumbsElement

                exclusive: false

                Breadcrumb {
                    id: rootBreadcrumb
                    root: true
                    text: i18n("All Applications")
                    depth: 0
                }
                Repeater {
                    model: ListModel {
                        id: crumbModel
                        // Array of the models
                        property var models: []
                    }

                    Breadcrumb {
                        root: false
                        text: model.text
                    }
                }
                onWidthChanged: {
                    if (LayoutMirroring.enabled) {
                        breadcrumbFlickable.contentX = -Math.max(0, breadcrumbsElement.width - breadcrumbFlickable.width)
                    } else {
                        breadcrumbFlickable.contentX = Math.max(0, breadcrumbsElement.width - breadcrumbFlickable.width)
                    }
                }
            }
        } // Flickable
    } // crumbContainer

    PlasmaExtras.ScrollArea {
        id: appViewScrollArea

        property Item activatedItem: null

        anchors {
            top: crumbContainer.bottom
            bottom: parent.bottom
            rightMargin: -units.largeSpacing
            leftMargin: -units.largeSpacing
        }

        Behavior on opacity { NumberAnimation { duration: units.longDuration } }

        width: parent.width

        function moveRight() {
            state = "";
            activatedItem.activate()
            applicationsView.positionViewAtBeginning()
        }

        function moveLeft() {
            state = "";
            // newModelIndex set by clicked breadcrumb
            var oldModel = applicationsView.model;
            applicationsView.model = applicationsView.newModel;
            applicationsView.positionViewAtIndex(applicationsView.model.rowForModel(oldModel), ListView.Center)
        }

        ListView {
            id: applicationsView

            property variant newModel

            focus: true
            keyNavigationWraps: true
            boundsBehavior: Flickable.StopAtBounds
            highlight: KickoffHighlight {}
            highlightMoveDuration : 0
            highlightResizeDuration: 0

            model: rootModel

            delegate: KickoffItem {
                id: kickoffItem

                appView: true
            }

            function addBreadcrumb(model, title) {
                crumbModel.append({"text": title, "depth": crumbModel.count+1})
                crumbModel.models.push(model);
            }

            function clearBreadcrumbs() {
                crumbModel.clear();
                crumbModel.models = [];
            }
        } // applicationsView

        states: [
            State {
                name: "OutgoingLeft"
                PropertyChanges {
                    target: appViewScrollArea
                    x: -parent.width
                    opacity: 0.0
                }
            },
            State {
                name: "OutgoingRight"
                PropertyChanges {
                    target: appViewScrollArea
                    x: parent.width
                    opacity: 0.0
                }
            }
        ]

        transitions:  [
            Transition {
                to: "OutgoingLeft"
                SequentialAnimation {
                    // We need to cache the currentItem since the selection can move during animation,
                    // and we want the item that has been clicked on, not the one that is under the
                    // mouse once the animation is done
                    ScriptAction { script: appViewScrollArea.activatedItem = applicationsView.currentItem }
                    NumberAnimation { properties: "x,opacity"; easing.type: Easing.InQuad; duration: units.longDuration }
                    ScriptAction { script: appViewScrollArea.moveRight() }
                }
            },
            Transition {
                to: "OutgoingRight"
                SequentialAnimation {
                    NumberAnimation { properties: "x,opacity"; easing.type: Easing.InQuad; duration: units.longDuration }
                    ScriptAction { script: appViewScrollArea.moveLeft() }
                }
            }
        ]
    } // appViewScrollArea

    MouseArea {
        anchors.fill: appViewScrollArea
        acceptedButtons: Qt.BackButton
        onClicked: {
            deactivateCurrentIndex()
        }
    }

    Timer {
        id: updatedLabelTimer
        interval: 1500
        running: false
        repeat: true

        onRunningChanged: {
            if (running) {
                updatedLabel.opacity = 1;
                crumbContainer.opacity = 0.3;
                appViewScrollArea.opacity = 0.3;
            }
        }
        onTriggered: {
            updatedLabel.opacity = 0;
            crumbContainer.opacity = 1;
            appViewScrollArea.opacity = 1;
            running = false;
        }
    }

    PlasmaComponents.Label {
        id: updatedLabel
        text: i18n("Applications updated.")
        opacity: 0
        visible: opacity != 0
        anchors.centerIn: parent

        Behavior on opacity { NumberAnimation { duration: units.shortDuration } }
    }

    Component.onCompleted: {
        rootModel.cleared.connect(refreshed);
    }

} // appViewContainer
