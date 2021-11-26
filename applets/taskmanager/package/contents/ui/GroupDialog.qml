/*
    SPDX-FileCopyrightText: 2012-2013 Eike Hein <hein@kde.org>
    SPDX-FileCopyrightText: 2021 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15
import QtQml.Models 2.2
import QtQuick.Window 2.2

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 3.0 as PlasmaComponents3
import org.kde.draganddrop 2.0

import "code/layout.js" as LayoutManager

PlasmaCore.Dialog {
    id: groupDialog
    visible: false

    type: PlasmaCore.Dialog.PopupMenu
    flags: Qt.WindowStaysOnTopHint
    hideOnWindowDeactivate: true
    location: plasmoid.location

    readonly property int preferredWidth: Screen.width / (3 * Screen.devicePixelRatio)
    readonly property int preferredHeight: Screen.height / (2 * Screen.devicePixelRatio)
    readonly property int contentWidth: mainItem.width // No padding here to avoid text elide.

    property alias overflowing: scrollView.overflowing
    property var _oldAppletStatus: PlasmaCore.Types.UnknownStatus

    property var activeTask: null

    function findActiveTaskIndex() {
        if (!activeTask) {
            return;
        }
        for (let i = 0; i < groupListView.count; i++) {
            if (tasksModel.makeModelIndex(visualParent.itemIndex, i) === activeTask) {
                groupListView.positionViewAtIndex(i, ListView.Contain); // Prevent visual glitches
                groupListView.currentIndex = i;
                return;
            }
        }
    }


    mainItem: MouseHandler {
        id: mouseHandler
        // HACK: 1 is to prevent "trying to show an empty dialog" warning.
        width: Math.min(groupDialog.preferredWidth, Math.max(groupListView.maxWidth, groupDialog.visualParent ? groupDialog.visualParent.width : 0, 1))
        height: Math.max(Math.min(groupDialog.preferredHeight, groupListView.maxHeight), 1)

        target: groupListView
        handleWheelEvents: !scrollView.overflowing
        isGroupDialog: true

        PlasmaComponents3.ScrollView {
            id: scrollView
            anchors.fill: parent
            readonly property bool overflowing: leftPadding > 0 || rightPadding > 0 // Scrollbar is visible

            PlasmaComponents3.ScrollBar.horizontal.policy: PlasmaComponents3.ScrollBar.AlwaysOff

            ListView {
                id: groupListView

                property int maxWidth: getMaxWidth()
                // Use groupFilter.count because sometimes count is not updated in time (BUG 446105)
                property int maxHeight: groupFilter.count * (LayoutManager.verticalMargins() + Math.max(theme.mSize(theme.defaultFont).height, PlasmaCore.Units.iconSizes.medium))

                function getMaxWidth() {
                    return groupFilter.maxTextWidth + LayoutManager.horizontalMargins() + PlasmaCore.Units.iconSizes.medium + 2 * (LayoutManager.labelMargin + LayoutManager.iconMargin);
                }

                model: DelegateModel {
                    id: groupFilter

                    readonly property TextMetrics textMetrics: TextMetrics {}
                    property int maxTextWidth: 0

                    model: groupDialog.visualParent ? tasksModel : null
                    rootIndex: groupDialog.visualParent ? tasksModel.makeModelIndex(groupDialog.visualParent.itemIndex) : null
                    delegate: Task {
                        visible: true
                        inPopup: true

                        onLabelTextChanged: Qt.callLater(groupFilter.updateMaxTextWidth) // ListView.onAdd included
                        ListView.onRemove: Qt.callLater(groupFilter.updateMaxTextWidth)
                    }

                    function updateMaxTextWidth() {
                        maxTextWidth = 0;
                        // 20 is based on performance considerations.
                        for (let i = 0; i < Math.min(count, 20); i++) {
                            textMetrics.text = items.get(i).model.display;
                            if (textMetrics.boundingRect.width > maxTextWidth) {
                                maxTextWidth = textMetrics.boundingRect.width;
                            }
                        }
                    }
                }

                reuseItems: false

                onCountChanged: if (count > 0) backend.cancelHighlightWindows()
            }
            // HACK: Prevent binding loop warnings, so the scrollbar will not block the text.
            onLeftPaddingChanged: groupListView.maxWidth = Qt.binding(() => groupListView.getMaxWidth() + leftPadding + rightPadding)
            onRightPaddingChanged: groupListView.maxWidth = Qt.binding(() => groupListView.getMaxWidth() + leftPadding + rightPadding)
        }
    }

    onVisibleChanged: {
        if (visible && visualParent) {
            _oldAppletStatus = plasmoid.status;
            plasmoid.status = PlasmaCore.Types.RequiresAttentionStatus;

            groupDialog.requestActivate();
            groupListView.forceActiveFocus(); // Active focus on ListView so keyboard navigation can work.
            Qt.callLater(findActiveTaskIndex);
        } else {
            plasmoid.status = _oldAppletStatus;
        }
    }

    onVisualParentChanged: activeTask = null
}
