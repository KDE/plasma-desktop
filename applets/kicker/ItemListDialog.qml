/*
    SPDX-FileCopyrightText: 2013-2014 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

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

    visible: false
    hideOnWindowDeactivate: kicker.hideOnWindowDeactivate
    location: PlasmaCore.Types.Floating
    offset: Kirigami.Units.smallSpacing
    LayoutMirroring.enabled: dialogMirrored

    onWindowDeactivated: {
        if (!aboutToBeDestroyed) {
            kicker.expanded = false;
        }
    }

    mainItem: ItemListView {
        id: itemListView
        height: implicitHeight
        width: Math.min(Math.max(Layout.minimumWidth, implicitWidth), Layout.maximumWidth)

        iconsEnabled: true
        LayoutMirroring.enabled: itemDialog.LayoutMirroring.enabled

        dialog: itemDialog

        model: funnelModel

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
                itemListView.currentIndex = -1;
                itemListView.resetDelegateSizing();
            }
        }
    }

    function delayedDestroy() {
        aboutToBeDestroyed = true;
        Plasmoid.hideOnWindowDeactivate = false;

        Qt.callLater(() => itemDialog.destroy());
    }
}
