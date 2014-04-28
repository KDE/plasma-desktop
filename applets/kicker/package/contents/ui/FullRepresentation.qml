/***************************************************************************
 *   Copyright (C) 2013-2014 by Eike Hein <hein@kde.org>                   *
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

Item {
    id: root

    Layout.minimumWidth: sideBar.width + mainRow.spacing + Math.max(rootList.width, runnerColumns.width)
    Layout.maximumWidth: sideBar.width + mainRow.spacing + Math.max(rootList.width, runnerColumns.width)

    Layout.minimumHeight: (rootModel.count * rootList.itemHeight) + searchField.height + (2 * units.smallSpacing)
    Layout.maximumHeight: (rootModel.count * rootList.itemHeight) + searchField.height + (2 * units.smallSpacing)

    property bool hideOnWindowDeactivate: !rootList.containsMouse
    property Item searchField: searchField

    function reset() {
        rootList.currentIndex = -1;

        searchField.text = "";
        searchField.focus = true;
    }

    Binding {
        target: plasmoid
        property: "hideOnWindowDeactivate"
        value: hideOnWindowDeactivate
    }

    Row {
        id: mainRow

        height: parent.height

        spacing: units.smallSpacing

        LayoutMirroring.enabled: ((plasmoid.location == PlasmaCore.Types.RightEdge)
            || (Qt.application.layoutDirection == Qt.RightToLeft))

        PlasmaCore.FrameSvgItem {
            id: sideBar

            width: units.iconSizes.medium + margins.left + margins.right
            height: parent.height

            imagePath: "widgets/frame"
            prefix: "plain"

            SideBarSection {
                id: favoriteApps

                anchors.top: (plasmoid.location == PlasmaCore.Types.TopEdge) ? undefined : parent.top
                anchors.topMargin: (anchors.top != undefined) ? sideBar.margins.top : undefined
                anchors.bottom: (plasmoid.location == PlasmaCore.Types.TopEdge) ? parent.bottom : undefined
                anchors.bottomMargin: (anchors.bottom != undefined) ? sideBar.margins.bottom : undefined

                height: (sideBar.height - sideBar.margins.top - sideBar.margins.bottom
                    - favoriteSystemActions.height - sidebarSeparator.height - (4 * units.smallSpacing))

                model: rootModel.favoritesModelForPrefix("app")
            }

            PlasmaCore.SvgItem {
                id: sidebarSeparator

                anchors.top: (plasmoid.location == PlasmaCore.Types.TopEdge) ? favoriteSystemActions.bottom : undefined
                anchors.topMargin: (anchors.top != undefined) ? (2 * units.smallSpacing) : undefined
                anchors.bottom: (plasmoid.location == PlasmaCore.Types.TopEdge) ? undefined : favoriteSystemActions.top
                anchors.bottomMargin: (anchors.bottom != undefined) ? (2 * units.smallSpacing) : undefined
                anchors.horizontalCenter: parent.horizontalCenter

                width: units.iconSizes.medium
                height: lineSvg.horLineHeight

                svg: lineSvg
                elementId: "horizontal-line"
            }

            SideBarSection {
                id: favoriteSystemActions

                anchors.top: (plasmoid.location == PlasmaCore.Types.TopEdge) ? parent.top : undefined
                anchors.topMargin: (anchors.top != undefined) ? sideBar.margins.top : undefined
                anchors.bottom: (plasmoid.location == PlasmaCore.Types.TopEdge) ? undefined : parent.bottom
                anchors.bottomMargin: (anchors.bottom != undefined) ? sideBar.margins.bottom : undefined

                model: rootModel.favoritesModelForPrefix("sys")
            }
        }

        ItemListView {
            id: rootList

            anchors.top: (plasmoid.location == PlasmaCore.Types.TopEdge) ? undefined : parent.top
            anchors.bottom: (plasmoid.location == PlasmaCore.Types.TopEdge) ? parent.bottom : undefined

            height: (rootModel.count * rootList.itemHeight)

            visible: (searchField.text == "")

            iconsEnabled: false

            model: rootModel

            KeyNavigation.up: searchField
            KeyNavigation.down: searchField
        }

        Row {
            id: runnerColumns

            height: parent.height

            signal focusChanged()

            visible: (searchField.text != "" && runnerModel.count > 0)

            Repeater {
                id: runnerColumnsRepeater

                model: runnerModel

                delegate: RunnerResultsList {
                    id: runnerMatches

                    onFocusChanged: {
                        if (focus) {
                            runnerColumns.focusChanged();
                        }
                    }

                    function focusChanged() {
                        if (!runnerMatches.focus && runnerMatches.currentIndex != -1) {
                            runnerMatches.currentIndex = -1;
                        }
                    }

                    Keys.onPressed: {
                        var target = null;

                        if (event.key == Qt.Key_Right) {
                            if (index < (runnerColumnsRepeater.count - 1)) {
                                target = runnerColumnsRepeater.itemAt(index + 1);
                            } else {
                                target = runnerColumnsRepeater.itemAt(0);
                            }
                        } else if (event.key == Qt.Key_Left) {
                            if (index == 0) {
                                target = runnerColumnsRepeater.itemAt(0);
                            } else {
                                target = runnerColumnsRepeater.itemAt(index - 1);
                            }
                        }

                        if (target) {
                            currentIndex = -1;
                            target.currentIndex = 0;
                            target.focus = true;
                        }
                    }

                    Component.onCompleted: {
                        runnerColumns.focusChanged.connect(focusChanged);
                    }

                    Component.onDestruction: {
                        runnerColumns.focusChanged.disconnect(focusChanged);
                    }


                    KeyNavigation.up: (index == 0) ? searchField : null
                    KeyNavigation.down: (index == 0) ? searchField : null
                }
            }
        }
    }

    PlasmaComponents.TextField {
        id: searchField

        anchors.top: (plasmoid.location == PlasmaCore.Types.TopEdge) ? mainRow.top : undefined
        anchors.bottom: (plasmoid.location == PlasmaCore.Types.TopEdge) ? undefined : mainRow.bottom
        anchors.left: parent.left
        anchors.leftMargin: sideBar.width + mainRow.spacing + units.smallSpacing

        width: rootList.width - (2 * units.smallSpacing)

        focus: true

        placeholderText: i18n("Search...")

        onTextChanged: {
            runnerModel.query = text;
        }

        onFocusChanged: {
            if (focus) {
                // FIXME: Cleanup arbitration between rootList/runnerCols here and in Keys.
                if (rootList.visible) {
                    rootList.currentIndex = -1;
                }

                if (runnerColumns.visible) {
                    runnerColumnsRepeater.itemAt(0).currentIndex = -1;
                }
            }
        }

        Keys.onPressed: {
            if (event.key == Qt.Key_Up) {
                if (rootList.visible) {
                    rootList.containsMouseOverride = true;
                    rootList.currentIndex = rootList.model.count - 1;
                }

                if (runnerColumns.visible) {
                    runnerColumnsRepeater.itemAt(0).currentIndex = runnerModel.modelForRow(0).count - 1;
                }
            } else if (event.key == Qt.Key_Down) {
                if (rootList.visible) {
                    rootList.currentIndex = 0;
                }

                if (runnerColumns.visible) {
                    runnerColumnsRepeater.itemAt(0).currentIndex = 0;
                }

            }
        }

        function appendText(newText) { // FIXME: See Keys in ItemListView.
            focus = true;
            text = text + newText;
        }
    }

    Component.onCompleted: {
        kicker.reset.connect(reset);
    }
}

