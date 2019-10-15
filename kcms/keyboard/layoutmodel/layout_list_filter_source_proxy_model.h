#ifndef LAYOUTLISTFILTERSOURCEPROXYMODEL_H
#define LAYOUTLISTFILTERSOURCEPROXYMODEL_H

#include "layout_list_model_base.h"
#include <QSortFilterProxyModel>

class LayoutListFilterSourceProxyModel : public QSortFilterProxyModel, public LayoutListModelBase {
    Q_OBJECT

public:
    LayoutListFilterSourceProxyModel(QObject *parent = nullptr);

    bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override;

private:
};

#endif // LAYOUTLISTFILTERSOURCEPROXYMODEL_H
