/*
    SPDX-FileCopyrightText: 2013-2014 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.ksvg as KSvg
import org.kde.plasma.components as PlasmaComponents3
import org.kde.plasma.core as PlasmaCore
import org.kde.plasma.plasmoid

FocusScope {
    id: runnerResultsList
    width: runnerColumnRowLayout.implicitWidth
    height: parent.height

    signal keyNavigationAtListEnd
    signal navigateLeftRequested
    signal navigateRightRequested

    property alias currentIndex: runnerMatches.currentIndex
    property alias currentItem: runnerMatches.currentItem
    property alias count: runnerMatches.count
    property alias containsMouse: runnerMatches.containsMouse
    property alias mainSearchField: runnerMatches.mainSearchField

    Accessible.name: header.text
    Accessible.role: Accessible.MenuItem

    RowLayout {
        id: runnerColumnRowLayout
        anchors.fill: parent
        spacing: Kirigami.Units.smallSpacing
        LayoutMirroring.enabled: runnerResultsList.LayoutMirroring.enabled

        KSvg.SvgItem {
            id: vertLine

            Layout.fillHeight: true
            visible: runnerResultsList.parent.visibleChildren[0] !== runnerResultsList

            imagePath: "widgets/line"
            elementId: "vertical-line"
        }

        ColumnLayout {
            id: runnerInnerColumn
            Layout.fillHeight: true
            Layout.fillWidth: true
            spacing: Kirigami.Units.smallSpacing

            PlasmaComponents3.Label {
                id: header

                Layout.fillWidth: true

                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter

                textFormat: Text.PlainText
                wrapMode: Text.NoWrap
                elide: Text.ElideRight
                font.weight: Font.Bold

                text: (runnerMatches.model !== null) ? runnerMatches.model.name : ""
            }

            Item {
                id: runnerListViewContainer

                Layout.fillHeight: true
                Layout.fillWidth: true
                implicitWidth: runnerMatches.width

                ItemListView {
                    id: runnerMatches

                    anchors.left: parent.left
                    anchors.top: Plasmoid.configuration.alignResultsToBottom ? undefined : parent.top
                    anchors.bottom: Plasmoid.configuration.alignResultsToBottom ? parent.bottom : undefined

                    height: Math.min(implicitHeight, runnerListViewContainer.height)

                    focus: true

                    iconsEnabled: true
                    keyNavigationWraps: (index != 0)
                    LayoutMirroring.enabled: runnerResultsList.LayoutMirroring.enabled

                    resetOnExitDelay: 0

                    model: runnerModel.modelForRow(index)

                    Connections {
                        target: runnerModel
                        function onAnyRunnerFinished () {
                            Qt.callLater( () => { // these come in quickly at the start
                                if (searchFieldPlaceholder.visible && searchField.focus) {
                                    currentIndex = 0;
                                } else {
                                    currentIndex = -1;
                                }
                            })
                        }
                    }
                    onNavigateLeftRequested: runnerResultsList.navigateLeftRequested()
                    onNavigateRightRequested: runnerResultsList.navigateRightRequested()
                    onKeyNavigationAtListEnd: mainSearchField.forceActiveFocus(Qt.TabFocusReason)
                }
            }

            Item {
                id: searchFieldPlaceholder

                height: mainSearchField.height
                Layout.fillWidth: true
                visible: runnerResultsList.parent.visibleChildren[0] === runnerResultsList
            }
        }
    }
}
