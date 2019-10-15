#ifndef LAYOUT_LIST_CONCAT_PROXY_MODEL_H
#define LAYOUT_LIST_CONCAT_PROXY_MODEL_H

#include <KConcatenateRowsProxyModel>

#include "layout_list_model_base.h"

class LayoutListConcatProxyModel : public KConcatenateRowsProxyModel, public LayoutListModelBase {
    Q_OBJECT
public:
    LayoutListConcatProxyModel(QObject* parent);

    QHash<int, QByteArray> roleNames() const override;
};

#endif // LAYOUT_LIST_CONCAT_PROXY_MODEL_H
