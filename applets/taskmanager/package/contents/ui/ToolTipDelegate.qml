/*
    SPDX-FileCopyrightText: 2013 Sebastian Kügler <sebas@kde.org>
    SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>
    SPDX-FileCopyrightText: 2016 Kai Uwe Broulik <kde@privat.broulik.de>
    SPDX-FileCopyrightText: 2017 Roman Gilg <subdiff@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.6
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

    Layout.minimumWidth: contentItem.width
    Layout.maximumWidth: Layout.minimumWidth

    Layout.minimumHeight: contentItem.height
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
                        model: toolTipDelegate.rootIndex ? tasksModel : null

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
