#ifndef ADVANCED_FILTER_PROXY_MODEL_H
#define ADVANCED_FILTER_PROXY_MODEL_H

#include <QSortFilterProxyModel>

class AdvancedFilterProxyModel : public QSortFilterProxyModel {
    Q_OBJECT
public:
    explicit AdvancedFilterProxyModel(QObject* parent = nullptr);

    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
};

#endif // ADVANCED_FILTER_PROXY_MODEL_H
