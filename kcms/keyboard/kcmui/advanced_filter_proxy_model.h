#ifndef ADVANCED_FILTER_PROXY_MODEL_H
#define ADVANCED_FILTER_PROXY_MODEL_H

#include "abstract_advanced_model.h"
#include <QSortFilterProxyModel>

class AdvancedFilterProxyModel : public QSortFilterProxyModel, public AbstractAdvancedModel {
    Q_OBJECT
public:
    explicit AdvancedFilterProxyModel(QObject* parent = nullptr);

    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;
};

#endif // ADVANCED_FILTER_PROXY_MODEL_H
