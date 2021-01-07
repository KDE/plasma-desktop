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

FocusScope {
    id: appViewContainer
    property QtObject activatedSection: null
    property string rootBreadcrumbName: ""
    signal appModelChange()
    onAppModelChange: {
        if (activatedSection != null) {
            applicationsView.clearBreadcrumbs();
            applicationsView.listView.model = activatedSection;
        }
    }

    objectName: "ApplicationsView"

    property ListView listView: applicationsView.listView

    function keyNavUp() {
        return applicationsView.keyNavUp();
    }

    function keyNavDown() {
        return applicationsView.keyNavDown();
    }

    function activateCurrentIndex() {
        applicationsView.state = "OutgoingLeft";
    }

    function openContextMenu() {
        applicationsView.currentItem.openActionMenu();
    }

    function deactivateCurrentIndex() {
        if (crumbModel.count > 0) { // this is not the case when switching from the right sidebar to the left when going "left"
            breadcrumbsElement.children[crumbModel.count-1].clickCrumb();
            applicationsView.state = "OutgoingRight";
            return true;
        }
        return false;
    }

    function reset() {
        applicationsView.model = activatedSection;
        applicationsView.clearBreadcrumbs();
        if (applicationsView.model == null) {
            applicationsView.currentIndex = -1
        } else {
            applicationsView.currentIndex = 0
        }
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
    Connections {
        target: rootBreadcrumb
        function onRootClick() {
            applicationsView.newModel = activatedSection;
        }
    }
    Item {
        id: crumbContainer

        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
        }
        visible: applicationsView.model != null && applicationsView.model.description && applicationsView.model.description != "KICKER_ALL_MODEL"
        height: visible ? breadcrumbFlickable.height : 0

        Behavior on opacity {
            NumberAnimation {
                duration: PlasmaCore.Units.longDuration
                easing.type: Easing.InOutQuad
            }
        }

        PlasmaCore.SvgItem {
            id: horizontalSeparator
            opacity: applicationsView.listView.contentY !== 0
            height: PlasmaCore.Units.devicePixelRatio
            elementId: "horizontal-line"
            z: 1

            anchors {
                left: parent.left
                leftMargin: PlasmaCore.Units.smallSpacing * 4
                right: parent.right
                rightMargin: PlasmaCore.Units.smallSpacing * 4
                bottom: parent.bottom
            }

            Behavior on opacity {
                NumberAnimation {
                    duration: PlasmaCore.Units.shortDuration
                    easing.type: Easing.InOutQuad
                }
            }

            svg: PlasmaCore.Svg {
                imagePath: "widgets/line"
            }
        }

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

            // HACK: Align the content to right for RTL locales
            leftMargin: LayoutMirroring.enabled ? Math.max(0, width - contentWidth) : 0

            Row {
                id: breadcrumbsElement

                spacing: PlasmaCore.Units.smallSpacing * 2

                Breadcrumb {
                    id: rootBreadcrumb
                    root: true
                    text: rootBreadcrumbName
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

    KickoffListView {
        id: applicationsView
        anchors {
            top: crumbContainer.bottom
            bottom: parent.bottom
        }

        width: parent.width

        property Item activatedItem: null
        property var newModel: null

        section.property: model && model.description == "KICKER_ALL_MODEL" ? "display" : ""
        section.criteria: ViewSection.FirstCharacter

        Behavior on opacity {
            NumberAnimation {
                duration: PlasmaCore.Units.longDuration
                easing.type: Easing.InOutQuad
            }
        }

        focus: true

        appView: true

        function moveLeft() {
            state = "Normal";
            // newModelIndex set by clicked breadcrumb
            var oldModel = applicationsView.model;
            applicationsView.model = applicationsView.newModel;

            var oldModelIndex = model.rowForModel(oldModel);
            // new model may not be a parent of old model (e.g. grandparent)
            if (oldModelIndex === -1) {
                listView.positionAtBeginning()
            } else {
                listView.currentIndex = oldModelIndex;
                listView.positionViewAtIndex(oldModelIndex, ListView.Center);
            }
        }

        function moveRight() {
            state = "Normal";
            activatedItem.activate()
            applicationsView.listView.positionAtBeginning()
        }

        function clearBreadcrumbs() {
            crumbModel.clear();
            crumbModel.models = [];
        }

        onReset: appViewContainer.reset()

        onAddBreadcrumb: {
            crumbModel.append({"text": title, "depth": crumbModel.count+1})
            crumbModel.models.push(model);
        }

        state: "Normal"
        states: [
            State {
                name: "Normal"
                PropertyChanges {
                    target: applicationsView
                    x: 0
                    opacity: 1.0
                }
            },
            State {
                name: "OutgoingLeft"
                PropertyChanges {
                    target: applicationsView
                    x: LayoutMirroring.enabled ? parent.width : -parent.width
                    opacity: 0.0
                }
            },
            State {
                name: "OutgoingRight"
                PropertyChanges {
                    target: applicationsView
                    x: LayoutMirroring.enabled ? -parent.width : parent.width
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
                    ScriptAction {
                        script: applicationsView.activatedItem = applicationsView.currentItem
                    }
                    NumberAnimation {
                        properties: "x"
                        easing.type: Easing.InQuad
                        duration: PlasmaCore.Units.longDuration
                    }
                    ScriptAction {
                        script: applicationsView.moveRight()
                    }
                }
            },
            Transition {
                to: "OutgoingRight"
                SequentialAnimation {
                    NumberAnimation {
                        properties: "x"
                        easing.type: Easing.InQuad
                        duration: PlasmaCore.Units.longDuration
                    }
                    ScriptAction {
                        script: applicationsView.moveLeft()
                    }
                }
            }
        ]
    }

    MouseArea {
        anchors.fill: parent

        acceptedButtons: Qt.BackButton

        onClicked: {
            deactivateCurrentIndex()
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
                crumbContainer.opacity = 0.3;
                applicationsView.listView.opacity = 0.3;
            }
        }
        onTriggered: {
            updatedLabel.opacity = 0;
            crumbContainer.opacity = 1;
            applicationsView.listView.opacity = 1;
            running = false;
        }
    }

    PlasmaComponents3.Label {
        id: updatedLabel
        text: i18n("Applications updated.")
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
