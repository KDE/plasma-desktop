#pragma once

#include <KDescendantsProxyModel>

#include "layout_list_model_xkb.h"

/**
 * @brief Flatten tree into a list.
 */
class LayoutListXkbExpandProxyModel : public KDescendantsProxyModel, public LayoutListModelBase {
    Q_OBJECT
public:
    explicit LayoutListXkbExpandProxyModel(QObject* parent = nullptr);
};
