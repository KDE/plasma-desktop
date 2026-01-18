/*
    SPDX-FileCopyrightText: 2013-2014 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.ksvg as KSvg
import org.kde.plasma.components as PlasmaComponents3
import org.kde.plasma.plasmoid

RowLayout {
    id: runnerResultsList

    signal keyNavigationAtListEnd
    signal navigateLeftRequested
    signal navigateRightRequested
    signal interactionConcluded

    required property int index

    property alias currentIndex: runnerMatches.currentIndex
    property alias currentItem: runnerMatches.currentItem
    property alias count: runnerMatches.count
    property alias mainSearchField: runnerMatches.mainSearchField
    property alias model: runnerMatches.model

    spacing: Kirigami.Units.smallSpacing

    function giveFocus(focusReason): void {
        runnerMatches.forceActiveFocus(focusReason)
    }

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
        Layout.minimumWidth: runnerMatches.Layout.minimumWidth
        Layout.maximumWidth: runnerMatches.Layout.maximumWidth
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
            Layout.fillHeight: true
            visible: Plasmoid.configuration.alignResultsToBottom
        }

        ItemListView {
            id: runnerMatches

            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.maximumHeight: Plasmoid.configuration.alignResultsToBottom ? contentHeight : -1
            implicitWidth: Kirigami.Units.gridUnit * 17
            Layout.minimumWidth: implicitWidth
            Layout.maximumWidth: implicitWidth

            Accessible.name: header.text

            dynamicResize: false
            iconsEnabled: true
            keyNavigationWraps: !searchFieldPlaceholder.visible
            LayoutMirroring.enabled: runnerResultsList.LayoutMirroring.enabled

            resetOnExitDelay: 0

            Connections {
                target: runnerModel
                function onAnyRunnerFinished () {
                    Qt.callLater( () => { // these come in quickly at the start
                        if (runnerResultsList.activeFocus) {
                            return; // don't interfere if the user has already moved focus
                        }
                        if (searchFieldPlaceholder.visible && runnerMatches.mainSearchField.focus) {
                            runnerMatches.currentIndex = 0;
                        } else {
                            runnerMatches.currentIndex = -1;
                        }
                    })
                }
            }
            onNavigateLeftRequested: runnerResultsList.navigateLeftRequested()
            onNavigateRightRequested: runnerResultsList.navigateRightRequested()
            onKeyNavigationAtListEnd: mainSearchField.forceActiveFocus(Qt.TabFocusReason)
            onInteractionConcluded: runnerResultsList.interactionConcluded()
        }

        Item {
            id: searchFieldPlaceholder

            implicitHeight: runnerResultsList.mainSearchField.height
            Layout.fillWidth: true
            visible: runnerResultsList.parent.visibleChildren[0] === runnerResultsList
        }
    }
}
