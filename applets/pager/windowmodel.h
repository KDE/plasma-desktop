/*
    SPDX-FileCopyrightText: 2016 Eike Hein <hein.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "taskfilterproxymodel.h"

#include <memory>

class PagerModel;

class WindowModel : public TaskManager::TaskFilterProxyModel
{
    Q_OBJECT

public:
    enum WindowModelRoles {
        StackingOrder = Qt::UserRole + 1,
    };
    Q_ENUM(WindowModelRoles)

    explicit WindowModel(PagerModel *parent);
    ~WindowModel() override;

    QHash<int, QByteArray> roleNames() const override;

    QVariant data(const QModelIndex &index, int role) const override;

private Q_SLOTS:
    void onPagerItemSizeChanged();

private:
    class Private;
    std::unique_ptr<Private> d;
};
