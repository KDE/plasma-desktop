#include "layout_list_xkb_expand_proxy_model.h"

LayoutListXkbExpandProxyModel::LayoutListXkbExpandProxyModel(QObject *parent)
    : KDescendantsProxyModel(parent)
{
    setDisplayAncestorData(true);
}
