#include "layout_list_concat_proxy_model.h"

#include "layout_list_model_base.h"

LayoutListConcatProxyModel::LayoutListConcatProxyModel(QObject* parent)
    : KConcatenateRowsProxyModel(parent)
{
}

QHash<int, QByteArray> LayoutListConcatProxyModel::roleNames() const
{
    return LayoutListModelBase::roleNames();
}
