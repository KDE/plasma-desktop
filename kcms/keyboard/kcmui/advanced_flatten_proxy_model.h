#ifndef ADVANCED_FLATTEN_PROXY_MODEL_H
#define ADVANCED_FLATTEN_PROXY_MODEL_H

#include <KDescendantsProxyModel>

#include "abstract_advanced_model.h"

class AdvancedFlattenProxyModel : public KDescendantsProxyModel, public AbstractAdvancedModel
{
    Q_OBJECT

public:
    AdvancedFlattenProxyModel(QObject *parent = nullptr);
    QHash<int, QByteArray> roleNames() const override;
};

#endif // ADVANCED_FLATTEN_PROXY_MODEL_H
