/*
    SPDX-FileCopyrightText: 2022 Tanbir Jishan <tantalising007@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

pragma ComponentBehavior: Bound

import QtQuick
import org.kde.kitemmodels as KItemModels
import org.kde.plasma.core as PlasmaCore
import org.kde.plasma.private.kicker as Kicker

KickoffListView {
    id: root

    required property Kicker.AppsModel gridModel

    highlight: null // highlight off since it otherwise highlights a whole section
    model: gridModel.sections
    section.property: "section"
    delegate: ListOfGridsViewDelegate {
        id: delegate

        required property int index
        required property string section

        width: root.width

        isCurrentSectionGrid: ListView.isCurrentItem
        isSearchFieldActive: kickoff.contentArea === root && kickoff.searchField.activeFocus
        parentView: ListView.view
        gridIndex: index

        model: KItemModels.KSortFilterProxyModel {
            id: sectionModel

            sourceModel: root.gridModel
            filterString: delegate.section
            filterRoleName: "group"

            function trigger(row, actionId, argument) {
                const filteredIndex = sectionModel.index(row, 0)
                const sourceIndex = sectionModel.mapToSource(filteredIndex)
                sourceModel.trigger(sourceIndex.row, actionId, argument)
            }
        }
        onShowSectionView: sectionName => {
            root.showSectionViewRequested(sectionName)
        }
    }
}
