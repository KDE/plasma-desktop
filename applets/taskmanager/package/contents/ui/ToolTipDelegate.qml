/*
    SPDX-FileCopyrightText: 2013 Sebastian Kügler <sebas@kde.org>
    SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>
    SPDX-FileCopyrightText: 2016 Kai Uwe Broulik <kde@privat.broulik.de>
    SPDX-FileCopyrightText: 2017 Roman Gilg <subdiff@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Window 2.15
import QtQml.Models 2.15

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 3.0 as PlasmaComponents3

Loader {
    id: toolTipDelegate

    property Item parentTask
    property var rootIndex

    property string appName
    property int pidParent
    property bool isGroup

    property var windows
    readonly property bool isWin: windows !== undefined

    property variant icon
    property url launcherUrl
    property bool isLauncher
    property bool isMinimizedParent

    // Needed for generateSubtext()
    property string displayParent
    property string genericName
    property var virtualDesktopParent
    property bool isOnAllVirtualDesktopsParent
    property var activitiesParent
    //
    property bool smartLauncherCountVisible
    property int smartLauncherCount

    readonly property bool isVerticalPanel: plasmoid.formFactor === PlasmaCore.Types.Vertical
    // This number controls the overall size of the window tooltips
    readonly property int tooltipInstanceMaximumWidth: PlasmaCore.Units.gridUnit * 16

    // These properties are required to make tooltip interactive when there is a player but no window is present.
    readonly property string mprisSourceName: mpris2Source.sourceNameForLauncherUrl(launcherUrl, pidParent)
    readonly property var playerData: mprisSourceName != "" ? mpris2Source.data[mprisSourceName] : 0
    readonly property bool hasPlayer: !!mprisSourceName && !!playerData

    Binding on Layout.minimumWidth {
        value: implicitWidth
        delayed: true // Prevent early hide of tooltip (BUG439522)
    }
    Layout.maximumWidth: Layout.minimumWidth

    Binding on Layout.minimumHeight {
        value: implicitHeight
        delayed: true // Prevent early hide of tooltip (BUG439522)
    }
    Layout.maximumHeight: Layout.minimumHeight

    LayoutMirroring.enabled: Qt.application.layoutDirection === Qt.RightToLeft
    LayoutMirroring.childrenInherit: true

    active: rootIndex !== undefined
    asynchronous: true

    sourceComponent: isGroup ? groupToolTip : singleTooltip

    Component {
        id: singleTooltip

        ToolTipInstance {
            submodelIndex: toolTipDelegate.rootIndex
        }
    }

    Component {
        id: groupToolTip

        PlasmaComponents3.ScrollView {
            // 2 * PlasmaCore.Units.smallSpacing is for the margin of tooltipDialog
            implicitWidth: leftPadding + rightPadding + Math.min(Screen.desktopAvailableWidth - 2 * PlasmaCore.Units.smallSpacing, Math.max(delegateModel.estimatedWidth, contentItem.contentItem.childrenRect.width))
            implicitHeight: bottomPadding + Math.min(Screen.desktopAvailableHeight - 2 * PlasmaCore.Units.smallSpacing, Math.max(delegateModel.estimatedHeight, contentItem.contentItem.childrenRect.height))

            // HACK: workaround for https://bugreports.qt.io/browse/QTBUG-83890
            PlasmaComponents3.ScrollBar.horizontal.policy: isVerticalPanel ? PlasmaComponents3.ScrollBar.AlwaysOff : PlasmaComponents3.ScrollBar.AsNeeded
            PlasmaComponents3.ScrollBar.vertical.policy: isVerticalPanel ? PlasmaComponents3.ScrollBar.AsNeeded : PlasmaComponents3.ScrollBar.AlwaysOff

            ListView {
                id: groupToolTipListView

                // HACK: workaround for https://bugreports.qt.io/browse/QTBUG-102811
                model: delegateModel.count > 0 ? delegateModel : null

                orientation: isVerticalPanel ? ListView.Vertical : ListView.Horizontal
                reuseItems: true
                spacing: PlasmaCore.Units.largeSpacing
            }

            DelegateModel {
                id: delegateModel

                // On Wayland, a tooltip has a significant resizing process, so estimate the size first.
                readonly property real estimatedWidth: (toolTipDelegate.isVerticalPanel ? 1 : count) * (toolTipDelegate.tooltipInstanceMaximumWidth + PlasmaCore.Units.largeSpacing) - PlasmaCore.Units.largeSpacing
                readonly property real estimatedHeight: (toolTipDelegate.isVerticalPanel ? count : 1) * (toolTipDelegate.tooltipInstanceMaximumWidth / 2 + PlasmaCore.Units.largeSpacing) - PlasmaCore.Units.largeSpacing

                model: tasksModel

                rootIndex: toolTipDelegate.rootIndex
                onRootIndexChanged: groupToolTipListView.positionViewAtBeginning() // Fix a visual glitch (when the mouse moves from a tooltip with a moved scrollbar to another tooltip without a scrollbar)

                delegate: ToolTipInstance {
                    submodelIndex: tasksModel.makeModelIndex(toolTipDelegate.rootIndex.row, index)
                }
            }
        }
    }
}
