/*
 * SPDX-FileCopyrightText: 2021 Noah Davis <noahadvs@gmail.com>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Templates 2.15 as T
import QtQml 2.15
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 3.0 as PC3
import org.kde.plasma.private.kicker 0.1 as Kicker

BasePage {
    id: root
    sideBarComponent: KickoffListView {
        id: sideBar
        focus: true // needed for Loaders
        model: KickoffSingleton.rootModel
        delegate: KickoffItemDelegate {
            id: itemDelegate
            extendHoverMargins: true
            width: view.availableWidth
            isCategory: model.hasChildren
        }
    }
    contentAreaComponent: VerticalStackView {
        id: stackView
        readonly property string preferredFavoritesViewObjectName: plasmoid.configuration.favoritesDisplay == 0 ? "favoritesGridView" : "favoritesListView"
        readonly property Component preferredFavoritesViewComponent: plasmoid.configuration.favoritesDisplay == 0 ? favoritesGridViewComponent : favoritesListViewComponent
        readonly property string preferredAppsViewObjectName: plasmoid.configuration.applicationsDisplay == 0 ? "applicationsGridView" : "applicationsListView"
        readonly property Component preferredAppsViewComponent: plasmoid.configuration.applicationsDisplay == 0 ? applicationsGridViewComponent : applicationsListViewComponent
        // NOTE: The 0 index modelForRow isn't supposed to be used. That's just how it works.
        property int appsModelRow: 1
        readonly property Kicker.AppsModel appsModel: KickoffSingleton.rootModel.modelForRow(appsModelRow)
        focus: true
        initialItem: preferredFavoritesViewComponent

        Component {
            id: favoritesListViewComponent
            DropAreaListView {
                id: favoritesListView
                objectName: "favoritesListView"
                mainContentView: true
                focus: true
                model: KickoffSingleton.rootModel.favoritesModel
            }
        }

        Component {
            id: favoritesGridViewComponent
            DropAreaGridView {
                id: favoritesGridView
                objectName: "favoritesGridView"
                focus: true
                model: KickoffSingleton.rootModel.favoritesModel
            }
        }

        Component {
            id: applicationsListViewComponent
            KickoffListView {
                id: applicationsListView
                objectName: "applicationsListView"
                mainContentView: true
                model: stackView.appsModel
                section.property: model && model.description == "KICKER_ALL_MODEL" ? "display" : ""
                section.criteria: ViewSection.FirstCharacter
            }
        }

        Component {
            id: applicationsGridViewComponent
            KickoffGridView {
                id: applicationsGridView
                objectName: "applicationsGridView"
                model: stackView.appsModel
            }
        }

        onPreferredFavoritesViewComponentChanged: {
            if (root.sideBarItem != null && root.sideBarItem.currentIndex === 0) {
                stackView.replace(stackView.preferredFavoritesViewComponent)
            }
        }
        onPreferredAppsViewComponentChanged: {
            if (root.sideBarItem != null && root.sideBarItem.currentIndex > 1) {
                stackView.replace(stackView.preferredAppsViewComponent)
            }
        }

        Connections {
            target: root.sideBarItem
            function onCurrentIndexChanged() {
                // Only update row index if the condition is met.
                // The 0 index modelForRow isn't supposed to be used. That's just how it works.
                if (root.sideBarItem.currentIndex > 0) {
                    appsModelRow = root.sideBarItem.currentIndex
                }
                if (root.sideBarItem.currentIndex === 0
                    && stackView.currentItem.objectName !== stackView.preferredFavoritesViewObjectName) {
                    stackView.replace(stackView.preferredFavoritesViewComponent)
                } else if (root.sideBarItem.currentIndex === 1
                    && stackView.currentItem.objectName !== "applicationsListView") {
                    // Always use list view for alphabetical apps view since grid view doesn't have sections
                    // TODO: maybe find a way to have a list view with grids in each section?
                    stackView.replace(applicationsListViewComponent)
                } else if (root.sideBarItem.currentIndex > 1
                    && stackView.currentItem.objectName !== stackView.preferredAppsViewObjectName) {
                    stackView.replace(stackView.preferredAppsViewComponent)
                }
            }
        }
        Connections {
            target: plasmoid
            function onExpandedChanged() {
                if(!plasmoid.expanded) {
                    KickoffSingleton.contentArea.currentItem.forceActiveFocus()
                }
            }
        }
    }
    // NormalPage doesn't get destroyed when deactivated, so the binding uses
    // StackView.status and visible. This way the bindings are reset when
    // NormalPage is Activated again.
    Binding {
        target: KickoffSingleton
        property: "sideBar"
        value: root.sideBarItem
        when: root.T.StackView.status === T.StackView.Active && root.visible
        restoreMode: Binding.RestoreBinding
    }
    Binding {
        target: KickoffSingleton
        property: "contentArea"
        value: root.contentAreaItem.currentItem // NOT just root.contentAreaItem
        when: root.T.StackView.status === T.StackView.Active && root.visible
        restoreMode: Binding.RestoreBinding
    }
}
