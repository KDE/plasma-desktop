/*
    SPDX-FileCopyrightText: 2013-2014 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.plasma.core as PlasmaCore
import org.kde.plasma.plasmoid
import org.kde.plasma.private.kicker as Kicker

Kicker.SubMenu {
    id: itemDialog

    property alias model: funnelModel.sourceModel

    property int index: -1
    property bool aboutToBeDestroyed: false

    property alias mainSearchField: itemListView.mainSearchField

    signal interactionConcluded

    visible: false
    backgroundHints: Plasmoid.containmentDisplayHints & PlasmaCore.Types.ContainmentPrefersOpaqueBackground ? PlasmaCore.Dialog.SolidBackground : PlasmaCore.Dialog.StandardBackground
    location: PlasmaCore.Types.Floating
    offset: 0 // slightly overlap submenu to match QtWidgets menu behavior
    LayoutMirroring.enabled: dialogMirrored

    onWindowDeactivated: {
        if (!aboutToBeDestroyed) {
            interactionConcluded()
        }
    }

    mainItem: ItemListView {
        id: itemListView
        height: implicitHeight
        width: Math.min(Math.max(Layout.minimumWidth, implicitWidth), Layout.maximumWidth)

        hoverEnabled: !hoverBlock.enabled
        iconsEnabled: true
        LayoutMirroring.enabled: itemDialog.LayoutMirroring.enabled
        // force tooltip for recent files - the path is relevant no matter the display setting
        showDescriptionInTooltip: (funnelModel.sourceModel as Kicker.RecentUsageModel)?.shownItems === Kicker.RecentUsageModel.OnlyDocs

        dialog: itemDialog

        model: funnelModel

        onInteractionConcluded: itemDialog.interactionConcluded()

        Kicker.FunnelModel {
            id: funnelModel

            property bool sorted: sourceModel?.sorted ?? false

            Component.onCompleted: {
                kicker.reset.connect(funnelModel.reset);
            }

            onCountChanged: {
                if (sourceModel && count === 0) {
                    itemDialog.delayedDestroy();
                }
            }

            onSourceModelChanged: {
                hoverBlock.reset()
                itemListView.currentIndex = -1;
                itemListView.resetDelegateSizing();
            }
        }

        HoverBlocker {
            id: hoverBlock
            anchors.fill: parent
            z: 10
        }
    }

    function delayedDestroy() {
        aboutToBeDestroyed = true;

        Qt.callLater(() => itemDialog.destroy());
    }
}
