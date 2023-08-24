/*
 * SPDX-FileCopyrightText: 2021 Mikel Johnson <mikel5764@gmail.com>
 * SPDX-FileCopyrightText: 2021 Noah Davis <noahadvs@gmail.com>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Templates 2.15 as T
import QtQml 2.15
import org.kde.plasma.private.kicker 0.1 as Kicker

BasePage {
    id: root
    sideBarComponent: KickoffListView {
        id: sideBar
        focus: true // needed for Loaders
        model: placesCategoryModel
        delegate: KickoffListDelegate {
            url: ""
            description: ""
            width: view.availableWidth
            isCategoryListItem: true
        }
    }
    contentAreaComponent: KickoffListView {
        id: contentArea
        mainContentView: true
        focus: true
        objectName: "frequentlyUsedView"
        model: switch (root.sideBarItem.currentIndex) {
            case 0: return computerModel
            case 1: return recentUsageModel
            case 2: return frequentUsageModel
        }
        onActiveFocusChanged: if (activeFocus && count < 1) {
            root.sideBarItem.forceActiveFocus()
        }
    }

    Kicker.ComputerModel {
        id: computerModel
        appletInterface: kickoff
        favoritesModel: rootModel.favoritesModel
        systemApplications: plasmoid.configuration.systemApplications
    }

    Kicker.RecentUsageModel {
        id: recentUsageModel
        favoritesModel: rootModel.favoritesModel
    }

    Kicker.RecentUsageModel {
        id: frequentUsageModel
        favoritesModel: rootModel.favoritesModel
        ordering: 1 // Popular / Frequently Used
    }

    // we make our model ourselves
    ListModel {
        id: placesCategoryModel
        ListElement { display: "Computer"; decoration: "computer" }
        ListElement { display: "History"; decoration: "view-history" }
        ListElement { display: "Frequently Used"; decoration: "clock" }
        Component.onCompleted: {
            // Can't use a function in a QML ListElement declaration
            placesCategoryModel.setProperty(0, "display", i18nc("category in Places sidebar", "Computer"))
            placesCategoryModel.setProperty(1, "display", i18nc("category in Places sidebar", "History"))
            placesCategoryModel.setProperty(2, "display", i18nc("category in Places sidebar", "Frequently Used"))
            if (KickoffSingleton.powerManagement.data["PowerDevil"]
                && KickoffSingleton.powerManagement.data["PowerDevil"]["Is Lid Present"]) {
                placesCategoryModel.setProperty(0, "decoration", "computer-laptop")
            }
        }
    }
    // NormalPage doesn't get destroyed when deactivated, so the binding uses
    // StackView.status and visible. This way the bindings are reset when
    // NormalPage is Activated again.
    Binding {
        target: kickoff
        property: "sideBar"
        value: root.sideBarItem
        when: root.T.StackView.status === T.StackView.Active && root.visible
        restoreMode: Binding.RestoreBinding
    }
    Binding {
        target: kickoff
        property: "contentArea"
        value: root.contentAreaItem // NOT root.contentAreaItem.currentItem
        when: root.T.StackView.status === T.StackView.Active && root.visible
        restoreMode: Binding.RestoreBinding
    }
}
