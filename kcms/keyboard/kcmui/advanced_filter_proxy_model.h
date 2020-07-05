/*
 * SPDX-FileCopyrightText: Gun Park <mujjingun@gmail.com>
 * 
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <QSortFilterProxyModel>

class AdvancedFilterProxyModel : public QSortFilterProxyModel {
    Q_OBJECT
public:
    explicit AdvancedFilterProxyModel(QObject* parent = nullptr);

    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
};
