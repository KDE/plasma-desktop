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

import QtQuick 2.2
import QtQuick.Layouts 1.1
import QtGraphicalEffects 1.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents

Item {
    id: root

    Layout.minimumWidth: sideBar.width + mainRow.spacing + Math.max(rootList.width, runnerColumns.width)
    Layout.maximumWidth: sideBar.width + mainRow.spacing + Math.max(rootList.width, runnerColumns.width)

    Layout.minimumHeight: Math.max((rootModel.count * rootList.itemHeight) + searchField.height + (2 * units.smallSpacing),
        sideBar.margins.top + sideBar.margins.bottom + favoriteApps.contentHeight + favoriteSystemActions.contentHeight
        + sidebarSeparator.height + (4 * units.smallSpacing))
    Layout.maximumHeight: Math.max((rootModel.count * rootList.itemHeight) + searchField.height + (2 * units.smallSpacing),
        sideBar.margins.top + sideBar.margins.bottom + favoriteApps.contentHeight + favoriteSystemActions.contentHeight
        + sidebarSeparator.height + (4 * units.smallSpacing))

    signal appendSearchText(string text)

    function reset() {
        rootList.currentIndex = -1;

        searchField.text = "";
        searchField.focus = true;
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

                anchors.top: parent.top
                anchors.topMargin: sideBar.margins.top

                height: (sideBar.height - sideBar.margins.top - sideBar.margins.bottom
                    - favoriteSystemActions.height - sidebarSeparator.height - (4 * units.smallSpacing))

                model: rootModel.favoritesModelForPrefix("app")

                states: [ State {
                    name: "top"
                    when: (plasmoid.location == PlasmaCore.Types.TopEdge)

                    AnchorChanges {
                        target: favoriteApps
                        anchors.top: undefined
                        anchors.bottom: parent.bottom
                    }

                    PropertyChanges {
                        target: favoriteApps
                        anchors.topMargin: undefined
                        anchors.bottomMargin: sideBar.margins.bottom
                    }
                }]
            }

            /* NOTE FIXME: The following is used to fade out favorites overflow. It's
             * currently not used since the popup is resized to fit all favorites in-
             * stead.
            ShaderEffectSource {
                id: favoriteAppsWrapper

                anchors.fill: favoriteApps

                enabled: true
                visible: false

                live: enabled
                hideSource: enabled

                sourceItem: favoriteApps
            }

            LinearGradient {
                id: maskGradient

                enabled: favoriteAppsWrapper.enabled

                anchors.fill: favoriteApps

                visible: false

                property int cutoff: 5

                start: Qt.point(0, Math.max(0, height - 10))
                end: Qt.point(0, height)

                gradient: Gradient {
                    GradientStop { position: 0.0; color: "#FF000000" }
                    GradientStop { position: 1.0; color: "#00000000" }
                }
            }

            OpacityMask {
                anchors.fill: favoriteApps

                enabled: favoriteAppsWrapper.enabled
                visible: enabled

                source: favoriteAppsWrapper
                maskSource: maskGradient
            }
            */

            PlasmaCore.SvgItem {
                id: sidebarSeparator

                anchors.bottom: favoriteSystemActions.top
                anchors.bottomMargin: (2 * units.smallSpacing)
                anchors.horizontalCenter: parent.horizontalCenter

                width: units.iconSizes.medium
                height: lineSvg.horLineHeight

                svg: lineSvg
                elementId: "horizontal-line"

                states: [ State {
                    name: "top"
                    when: (plasmoid.location == PlasmaCore.Types.TopEdge)

                    AnchorChanges {
                        target: sidebarSeparator
                        anchors.top: favoriteSystemActions.bottom
                        anchors.bottom: undefined

                    }

                    PropertyChanges {
                        target: sidebarSeparator
                        anchors.topMargin: (2 * units.smallSpacing)
                        anchors.bottomMargin: undefined
                    }
                }]
            }

            SideBarSection {
                id: favoriteSystemActions

                anchors.bottom: parent.bottom
                anchors.bottomMargin: sideBar.margins.bottom

                model: rootModel.favoritesModelForPrefix("sys")

                states: [ State {
                    name: "top"
                    when: (plasmoid.location == PlasmaCore.Types.TopEdge)

                    AnchorChanges {
                        target: favoriteSystemActions
                        anchors.top: parent.top
                        anchors.bottom: undefined
                    }

                    PropertyChanges {
                        target: favoriteSystemActions
                        anchors.topMargin: sideBar.margins.top
                        anchors.bottomMargin: undefined
                    }
                }]
            }
        }

        ItemListView {
            id: rootList

            anchors.top: parent.top

            height: (rootModel.count * rootList.itemHeight)

            visible: (searchField.text == "")

            iconsEnabled: false

            model: rootModel

            states: [ State {
                name: "top"
                when: (plasmoid.location == PlasmaCore.Types.TopEdge)

                AnchorChanges {
                    target: rootList
                    anchors.top: undefined
                    anchors.bottom: parent.bottom
                }
            }]

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

        anchors.bottom: mainRow.bottom
        anchors.left: parent.left
        anchors.leftMargin: sideBar.width + mainRow.spacing + units.smallSpacing

        width: rootList.width - (2 * units.smallSpacing)

        focus: true

        placeholderText: i18n("Search...")
        clearButtonShown: true

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

        states: [ State {
            name: "top"
            when: plasmoid.location == PlasmaCore.Types.TopEdge

            AnchorChanges {
                target: searchField
                anchors.top: mainRow.top
                anchors.bottom: undefined
                anchors.left: parent.left
                anchors.right: undefined
            }

            PropertyChanges {
                target: searchField
                anchors.leftMargin: sideBar.width + mainRow.spacing + units.smallSpacing
                anchors.rightMargin: undefined
            }
        },
        State {
            name: "right"
            when: plasmoid.location == PlasmaCore.Types.RightEdge

            AnchorChanges {
                target: searchField
                anchors.top: undefined
                anchors.bottom: mainRow.bottom
                anchors.left: undefined
                anchors.right: parent.right
            }

            PropertyChanges {
                target: searchField
                anchors.leftMargin: undefined
                anchors.rightMargin: sideBar.width + mainRow.spacing + units.smallSpacing
            }
        }]

        Keys.onPressed: {
            if (event.key == Qt.Key_Up) {
                if (rootList.visible) {
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

        function appendText(newText) {
            focus = true;
            text = text + newText;
        }
    }

    Keys.onPressed: {
        if (event.key == Qt.Key_Escape) {
            plasmoid.expanded = false;
        }
    }

    Component.onCompleted: {
        appendSearchText.connect(searchField.appendText);

        kicker.reset.connect(reset);
    }
}

