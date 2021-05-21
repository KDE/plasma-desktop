/*
    SPDX-FileCopyrightText: 2013-2014 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.0
import org.kde.plasma.plasmoid 2.0

import org.kde.plasma.core 2.0 as PlasmaCore

import org.kde.plasma.private.kicker 0.1 as Kicker

Kicker.SubMenu {
    id: itemDialog

    property alias focusParent: itemListView.focusParent
    property alias model: funnelModel.sourceModel

    property bool aboutToBeDestroyed: false

    visible: false
    hideOnWindowDeactivate: plasmoid.hideOnWindowDeactivate

    location: PlasmaCore.Types.Floating

    offset: PlasmaCore.Units.smallSpacing

    onWindowDeactivated: {
        if (!aboutToBeDestroyed) {
            plasmoid.expanded = false;
        }
    }

    mainItem: ItemListView {
        id: itemListView

        height: itemDialog.model !== undefined ? Math.min(((Math.floor((itemDialog.availableScreenRectForItem(itemListView).height
            - itemDialog.margins.top - itemDialog.margins.bottom) / itemHeight) - 1)
            * itemHeight) - (itemDialog.model.separatorCount * itemHeight) + (itemDialog.model.separatorCount * separatorHeight),
            ((itemDialog.model.count - itemDialog.model.separatorCount) * itemHeight) + (itemDialog.model.separatorCount * separatorHeight)) : 0

        iconsEnabled: true

        dialog: itemDialog

        model: funnelModel

        Kicker.FunnelModel {
            id: funnelModel

            Component.onCompleted: {
                kicker.reset.connect(funnelModel.reset);
            }

            onCountChanged: {
                if (sourceModel && count == 0) {
                    itemDialog.delayedDestroy();
                }
            }

            onSourceModelChanged: {
                itemListView.currentIndex = -1;
            }
        }
    }

    function delayedDestroy() {
        aboutToBeDestroyed = true;
        plasmoid.hideOnWindowDeactivate = false;
        visible = false;

        Qt.callLater(function() {
            itemDialog.destroy();
        });
    }
}
