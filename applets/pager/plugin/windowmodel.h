/********************************************************************
Copyright 2016  Eike Hein <hein.org>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the
Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .
*********************************************************************/

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
