/*
    SPDX-FileCopyrightText: 2016 Eike Hein <hein.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef WINDOWMODEL_H
#define WINDOWMODEL_H

#include "taskfilterproxymodel.h"

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

    void refreshStackingOrder();

private:
    class Private;
    QScopedPointer<Private> d;
};

#endif
