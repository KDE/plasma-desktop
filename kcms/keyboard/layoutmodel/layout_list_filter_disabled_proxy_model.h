/*
 * SPDX-FileCopyrightText: (C) 2019 Gun Park <mujjingun@gmail.com>
 * SPDX-FileCopyrightText: (C) 2020 Carl Schwan <carl@carlschwan.eu>
 * 
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <QSortFilterProxyModel>
#include "layout_list_model_base.h"

/**
 * @brief Filter out disabled layout from the selected list.
 */
class LayoutListFilterDisabledProxyModel : public QSortFilterProxyModel, public LayoutListModelBase {
    Q_OBJECT
public:
    LayoutListFilterDisabledProxyModel(QObject* parent);

    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;
};
