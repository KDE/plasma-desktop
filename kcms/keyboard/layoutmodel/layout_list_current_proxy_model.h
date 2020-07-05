/*
 * SPDX-FileCopyrightText: Gun Park <mujjingun@gmail.com>
 * 
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#pragma once

#include "layout_list_model_base.h"
#include <QObject>
#include <QSortFilterProxyModel>

class LayoutListCurrentProxyModel : public QSortFilterProxyModel, public LayoutListModelBase {
    Q_OBJECT
public:
    LayoutListCurrentProxyModel(QObject* parent);

    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

private Q_SLOTS:
    void currentInputSourceChanged();
};
