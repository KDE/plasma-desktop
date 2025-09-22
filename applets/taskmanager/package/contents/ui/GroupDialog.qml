/*
    SPDX-FileCopyrightText: 2012-2013 Eike Hein <hein@kde.org>
    SPDX-FileCopyrightText: 2021 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQml.Models

import org.kde.plasma.core as PlasmaCore
import org.kde.plasma.components as PlasmaComponents3
import org.kde.kirigami as Kirigami
import org.kde.plasma.plasmoid

import "code/layoutmetrics.js" as LayoutMetrics

PlasmaCore.PopupPlasmaWindow {
    id: groupDialog

    width: mouseHandler.implicitWidth + leftPadding + rightPadding
    height: mouseHandler.implicitHeight + topPadding + bottomPadding

    animated: true
    removeBorderStrategy: Plasmoid.location === PlasmaCore.Types.Floating
            ? PlasmaCore.AppletPopup.AtScreenEdges
            : PlasmaCore.AppletPopup.AtScreenEdges | PlasmaCore.AppletPopup.AtPanelEdges

    margin: (Plasmoid.containmentDisplayHints & PlasmaCore.Types.ContainmentPrefersFloatingApplets) ? Kirigami.Units.largeSpacing : 0
    onActiveChanged: {
        if (!active) {
            visible = false;
        }
    }


    popupDirection: switch (Plasmoid.location) {
        case PlasmaCore.Types.TopEdge:
            return Qt.BottomEdge
        case PlasmaCore.Types.LeftEdge:
            return Qt.RightEdge
        case PlasmaCore.Types.RightEdge:
            return Qt.LeftEdge
        default:
            return Qt.TopEdge
    }

    readonly property real preferredWidth: Screen.width / 3
    readonly property real preferredHeight: Screen.height / 2
    readonly property real contentWidth: mainItem.width // No padding here to avoid text elide.

    property /*PlasmaCore.ItemStatus*/int _oldAppletStatus: PlasmaCore.Types.UnknownStatus

    function findActiveTaskIndex(): void {
        if (!tasksModel.activeTask) {
            return;
        }
        for (let i = 0; i < groupListView.count; i++) {
            if (tasksModel.makeModelIndex(visualParent.index, i) === tasksModel.activeTask) {
                groupListView.positionViewAtIndex(i, ListView.Contain); // Prevent visual glitches
                groupListView.currentIndex = i;
                return;
            }
        }
    }

    Component.onCompleted: {
        // Don't bind visible at creation, otherwise it
        // will be made visible before assigning the visual partent
        // making the window flickering in the center of the screen before being moved
        // in the correct position
        visible = true
    }

    mainItem: MouseHandler {
        id: mouseHandler
        implicitWidth: Math.min(groupDialog.preferredWidth, Math.max(groupListView.maxWidth, groupDialog.visualParent.width))
        implicitHeight: Math.min(groupDialog.preferredHeight, groupListView.maxHeight)

        target: groupListView
        handleWheelEvents: !scrollView.overflowing
        isGroupDialog: true

        Keys.onEscapePressed: event => {
            groupDialog.visible = false;
        }

        function moveRow(event: KeyEvent, insertAt: int): void {
            if (!(event.modifiers & Qt.ControlModifier) || !(event.modifiers & Qt.ShiftModifier)) {
                event.accepted = false;
                return;
            } else if (insertAt < 0 || insertAt >= groupListView.count) {
                return;
            }

            const parentModelIndex = tasksModel.makeModelIndex(groupDialog.visualParent.index);
            const status = tasksModel.move(groupListView.currentIndex, insertAt, parentModelIndex);
            if (!status) {
                return;
            }

            groupListView.currentIndex = insertAt;
        }

        PlasmaComponents3.ScrollView {
            id: scrollView

            // To achieve a bottom-to-top layout on vertical panels, the task manager
            // is rotated by 180 degrees(see main.qml). This makes the group dialog's
            // items rotated, so un-rotate them here to fix that.
            rotation: Plasmoid.configuration.reverseMode && Plasmoid.formFactor === PlasmaCore.Types.Vertical ? 180 : 0

            anchors.fill: parent
            readonly property bool overflowing: leftPadding > 0 || rightPadding > 0 // Scrollbar is visible

            ListView {
                id: groupListView

                readonly property real maxWidth: groupFilter.maxTextWidth
                                                + LayoutMetrics.horizontalMargins()
                                                + Kirigami.Units.iconSizes.medium
                                                + 2 * (LayoutMetrics.labelMargin + LayoutMetrics.iconMargin)
                                                + scrollView.leftPadding + scrollView.rightPadding
                // Use groupFilter.count because sometimes count is not updated in time (BUG 446105)
                readonly property real maxHeight: groupFilter.count * (LayoutMetrics.verticalMargins() + Math.max(Kirigami.Units.iconSizes.sizeForLabels, Kirigami.Units.iconSizes.medium))

                model: DelegateModel {
                    id: groupFilter

                    readonly property TextMetrics textMetrics: TextMetrics {}
                    property real maxTextWidth: 0

                    model: tasksModel
                    rootIndex: tasksModel.makeModelIndex(groupDialog.visualParent.index)
                    delegate: Task {
                        width: groupListView.width
                        visible: true
                        inPopup: true
                        tasksRoot: tasks

                        ListView.onRemove: Qt.callLater(groupFilter.updateMaxTextWidth)
                        Connections {
                            enabled: index < 20 // 20 is based on performance considerations.

                            function onLabelTextChanged(): void { // ListView.onAdd included
                                if (groupFilter.maxTextWidth === 0) {
                                    // Update immediately to avoid shrinking
                                    groupFilter.updateMaxTextWidth();
                                } else {
                                    Qt.callLater(groupFilter.updateMaxTextWidth);
                                }
                            }
                        }
                    }

                    function updateMaxTextWidth(): void {
                        let tempMaxTextWidth = 0;
                        // 20 is based on performance considerations.
                        for (let i = 0; i < Math.min(count, 20); i++) {
                            textMetrics.text = items.get(i).model.display;
                            if (textMetrics.boundingRect.width > tempMaxTextWidth) {
                                tempMaxTextWidth = textMetrics.boundingRect.width;
                            }
                        }
                        maxTextWidth = tempMaxTextWidth;
                    }
                }

                reuseItems: false

                Keys.onUpPressed: event => mouseHandler.moveRow(event, groupListView.currentIndex - 1)
                Keys.onDownPressed: event => mouseHandler.moveRow(event, groupListView.currentIndex + 1)

                onCountChanged: {
                    if (count > 0) {
                        tasks.cancelHighlightWindows()
                    } else {
                        groupDialog.visible = false;
                    }
                }
            }
        }
    }

    onVisibleChanged: {
        if (visible) {
            _oldAppletStatus = Plasmoid.status;
            Plasmoid.status = PlasmaCore.Types.RequiresAttentionStatus;

            groupDialog.requestActivate();
            groupListView.forceActiveFocus(); // Active focus on ListView so keyboard navigation can work.
            Qt.callLater(findActiveTaskIndex);
        } else {
            Plasmoid.status = _oldAppletStatus;
            tasks.groupDialog = null;
            destroy();
        }
    }
}
