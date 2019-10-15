#ifndef LAYOUT_LIST_FILTER_DISBLED_PROXYMODEL_H
#define LAYOUT_LIST_FILTER_DISBLED_PROXYMODEL_H

#include <QSortFilterProxyModel>
#include "layout_list_model_base.h"

class LayoutListFilterDisabledProxyModel : public QSortFilterProxyModel, public LayoutListModelBase {
    Q_OBJECT
public:
    LayoutListFilterDisabledProxyModel(QObject* parent);

    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;
};

#endif // LAYOUT_LIST_FILTER_DISBLED_PROXYMODEL_H
