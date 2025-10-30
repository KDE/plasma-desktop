/*
    SPDX-FileCopyrightText: 2024 Evgeny Chesnokov <echesnokov@astralinux.ru>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick

import org.kde.kitemmodels as KItemModels

KItemModels.KSortFilterProxyModel {
    id: layoutsProxy
    sourceModel: layoutSearchProxy
    sortRoleName: "searchScore"
    sortOrder: Qt.DescendingOrder

    filterRowCallback: function (row, parent): bool {
        const modelIndex = sourceModel.index(row, 0, parent);
        const currentVariantName = sourceModel.data(modelIndex, KItemModels.KRoleNames.role("variantName"));
        const description = sourceModel.data(modelIndex, KItemModels.KRoleNames.role("description"));

        if (currentVariantName !== '') {
            return false;
        }

        if (searchField.text.length > 0) {
            const score = sourceModel.data(modelIndex, KItemModels.KRoleNames.role("searchScore"));
            if (score !== 0) {
                return true;
            }
            const shortNameRole = KItemModels.KRoleNames.role("shortName");
            const currentName = sourceModel.data(modelIndex, shortNameRole);
            const searchScoreRole = KItemModels.KRoleNames.role("searchScore");
            for (let i = 0; i < sourceModel.rowCount(); i++) {
                const index = sourceModel.index(i, 0, parent);
                const name = sourceModel.data(index, shortNameRole);
                const variantSearchScore = sourceModel.data(index, searchScoreRole);
                if (name === currentName && variantSearchScore > 100) {
                    return true;
                }
            }
            return false;
        }

        return true;
    }
}
