#ifndef LAYOUT_LIST_SORT_FILTER_PROXY_MODEL_H
#define LAYOUT_LIST_SORT_FILTER_PROXY_MODEL_H

#include "layout_list_model_base.h"
#include <QObject>
#include <QSortFilterProxyModel>

class LayoutListSortFilterProxyModel : public QSortFilterProxyModel, public LayoutListModelBase {
    Q_OBJECT
public:
    LayoutListSortFilterProxyModel(QObject* parent);
};

#endif // LAYOUT_LIST_SORT_FILTER_PROXY_MODEL_H
