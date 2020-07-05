/*
 * SPDX-FileCopyrightText: Gun Park <mujjingun@gmail.com>
 * 
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#pragma once

#include <KConcatenateRowsProxyModel>

#include "layout_list_model_base.h"

/**
 * @brief Contat the Fcitx, Xkb and selected model
 */
class LayoutListConcatProxyModel : public KConcatenateRowsProxyModel, public LayoutListModelBase {
    Q_OBJECT
public:
    LayoutListConcatProxyModel(QObject* parent);

    QHash<int, QByteArray> roleNames() const override;
};
