#ifndef LAYOUT_LIST_CURRENT_PROXY_MODEL_H
#define LAYOUT_LIST_CURRENT_PROXY_MODEL_H

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

#endif // LAYOUT_LIST_CURRENT_PROXY_MODEL_H
