#ifndef LAYOUTLISTREMOVEDUPLICATESPROXYMODEL_H
#define LAYOUTLISTREMOVEDUPLICATESPROXYMODEL_H

#include "layout_list_model_base.h"
#include <QObject>
#include <QSortFilterProxyModel>

class LayoutListFilterDuplicatesProxyModel : public QSortFilterProxyModel, public LayoutListModelBase {
    Q_OBJECT
public:
    explicit LayoutListFilterDuplicatesProxyModel(QObject* parent = nullptr);

    bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override;

    QVariant data(const QModelIndex& index, int role) const override;
    Q_INVOKABLE void add(int idx);

public Q_SLOTS:
    void updateFilter();
    void updateEnabled();

Q_SIGNALS:
    void missingCountChanged();
    void itemAdded(QString const& name);

private:
    int getGhostItemRow(const QModelIndex& index) const;
};

#endif // LAYOUTLISTREMOVEDUPLICATESPROXYMODEL_H
