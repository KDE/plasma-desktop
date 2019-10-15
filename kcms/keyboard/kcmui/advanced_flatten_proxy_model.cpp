#include "advanced_flatten_proxy_model.h"

AdvancedFlattenProxyModel::AdvancedFlattenProxyModel(QObject *parent)
    : KDescendantsProxyModel(parent)
{
}

QHash<int, QByteArray> AdvancedFlattenProxyModel::roleNames() const
{
    return AbstractAdvancedModel::roleNames();
}
