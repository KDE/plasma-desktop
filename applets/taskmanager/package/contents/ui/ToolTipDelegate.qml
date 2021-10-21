/*
    SPDX-FileCopyrightText: 2013 Sebastian Kügler <sebas@kde.org>
    SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>
    SPDX-FileCopyrightText: 2016 Kai Uwe Broulik <kde@privat.broulik.de>
    SPDX-FileCopyrightText: 2017 Roman Gilg <subdiff@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.8
import QtQuick.Layouts 1.1
import QtGraphicalEffects 1.0
import QtQml.Models 2.2

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 3.0 as PlasmaComponents3
import org.kde.kquickcontrolsaddons 2.0 as KQuickControlsAddons

import org.kde.taskmanager 0.1 as TaskManager

PlasmaComponents3.ScrollView {
    id: toolTipDelegate

    property Item parentTask
    property var rootIndex
    // hasRootIndex is needed to avoid unnecessary reevaluation of model property in DelegateModel,
    // because !!toolTipDelegate.rootIndex will not work as expected.
    readonly property bool hasRootIndex: !!rootIndex

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

    property int textWidth: PlasmaCore.Theme.mSize(PlasmaCore.Theme.defaultFont).width * 20

    Loader {
        id: contentItem

        active: toolTipDelegate.rootIndex !== undefined
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

            Grid {
                rows: !isVerticalPanel
                columns: isVerticalPanel
                flow: isVerticalPanel ? Grid.TopToBottom : Grid.LeftToRight
                spacing: PlasmaCore.Units.largeSpacing

                Repeater {
                    id: groupRepeater
                    model: DelegateModel {
                        model: toolTipDelegate.hasRootIndex ? tasksModel : null

                        rootIndex: toolTipDelegate.rootIndex

                        delegate: ToolTipInstance {
                            submodelIndex: tasksModel.makeModelIndex(toolTipDelegate.rootIndex.row, index)
                        }
                    }
                }
            }
        }
    }
}
