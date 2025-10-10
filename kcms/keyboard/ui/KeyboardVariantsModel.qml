/*
    SPDX-FileCopyrightText: 2024 Evgeny Chesnokov <echesnokov@astralinux.ru>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick

import org.kde.kitemmodels as KItemModels

KItemModels.KSortFilterProxyModel {
    id: variantProxy
    sourceModel: layoutSearchProxy
    sortRoleName: "searchScore"
    sortOrder: Qt.DescendingOrder

    filterRowCallback: function (row, parent) {
        if (!layoutsView.currentItem) {
            return false;
        }

        const modelIndex = sourceModel.index(row, 0, parent);
        const currentName = sourceModel.data(modelIndex, KItemModels.KRoleNames.role("shortName"));
        const selectedName = layoutsView.currentItem.shortName;

        if (currentName !== selectedName) {
            return false;
        }

        if (searchField.text.length > 0) {
            const searchScore = sourceModel.data(modelIndex, KItemModels.KRoleNames.role("searchScore"));
            return searchScore > 100;
        }

        return true;
    }
}
