/*
    Copyright (C) 2011  Martin Gräßlin <mgraesslin@kde.org>
    Copyright (C) 2012  Gregor Taetzner <gregor@freenet.de>

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
import org.kde.plasma.private.kickoff 0.1 as Kickoff

Item {
    id: appViewContainer

    anchors.fill: parent

    objectName: "ApplicationsView"

    function decrementCurrentIndex() {
        applicationsView.decrementCurrentIndex();
    }

    function incrementCurrentIndex() {
        applicationsView.incrementCurrentIndex();
    }

    function activateCurrentIndex(start) {
        if (!applicationsView.currentItem.modelChildren)
            if (!start)
                return;

        appViewScrollArea.state = "OutgoingLeft";
    }

    function deactivateCurrentIndex() {
        if (crumbModel.count > 0) { // this is not the case when switching from the "Applications" to the "Favorites" tab using the "Left" key
            breadcrumbsElement.children[crumbModel.count-1].clickCrumb();
            appViewScrollArea.state = "OutgoingRight";
            return true;
        }
        return false;
    }

    ContextMenu {
        id: contextMenu
    }

    Item {
        id: crumbContainer

        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
        }
        height: childrenRect.height

        Flickable {
            anchors {
                top: parent.top
                left: parent.left
                right: parent.right
            }
            height: breadcrumbsElement.height

            contentWidth: breadcrumbsElement.width
            contentX: contentWidth - width

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
                        // Array of the category indexes
                        property var indexes: []
                    }

                    Breadcrumb {
                        root: false
                        text: model.text
                    }
                }
            }
        } // Flickable
    } // crumbContainer

    PlasmaExtras.ScrollArea {
        id: appViewScrollArea

        anchors {
            top: crumbContainer.bottom
            bottom: parent.bottom
            rightMargin: -units.largeSpacing
            leftMargin: -units.largeSpacing
        }


        width: parent.width

        function moveRight() {
            state = "";
            applicationsView.currentItem.activate();
        }

        function moveLeft() {
            state = "";
            // newModelIndex set by clicked breadcrumb
            applicationsView.model.rootIndex = applicationsView.newModelIndex;
        }

        ListView {
            id: applicationsView

            property variant newModelIndex

            focus: true
            keyNavigationWraps: true
            boundsBehavior: Flickable.StopAtBounds
            highlight: PlasmaComponents.Highlight {}

            model: VisualDataModel {
                id: vmodel

                delegate: KickoffItem {
                    id: kickoffItem

                    appView: true

                    PlasmaCore.SvgItem {
                        anchors {
                            right: parent.right
                            verticalCenter: parent.verticalCenter
                            rightMargin: y
                        }
                        height: units.iconSizes.small
                        width: height

                        svg: arrowSvg
                        elementId: "go-next"
                        visible: hasModelChildren
                    }
                }

                model: Kickoff.ApplicationModel {}
            } // VisualDataModel

            function addBreadcrumb(categoryIndex, title) {
                crumbModel.append({"text": title, "depth": crumbModel.count+1})
                crumbModel.indexes.push(categoryIndex);
            }

            PlasmaCore.Svg {
                id: arrowSvg

                imagePath: "toolbar-icons/go"
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
} // appViewContainer
