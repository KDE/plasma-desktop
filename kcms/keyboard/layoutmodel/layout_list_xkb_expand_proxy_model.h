#ifndef EXPAND_LAYOUT_LIST_PROXY_MODEL_H
#define EXPAND_LAYOUT_LIST_PROXY_MODEL_H

#include <KDescendantsProxyModel>

#include "layout_list_model_xkb.h"

class LayoutListXkbExpandProxyModel : public KDescendantsProxyModel, public LayoutListModelBase {
    Q_OBJECT
public:
    explicit LayoutListXkbExpandProxyModel(QObject* parent = nullptr);
};

#endif // EXPAND_LAYOUT_LIST_PROXY_MODEL_H
