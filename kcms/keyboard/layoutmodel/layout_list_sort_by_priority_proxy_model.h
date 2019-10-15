#ifndef LAYOUTLISTENABLEDPROXYMODEL_H
#define LAYOUTLISTENABLEDPROXYMODEL_H

#include <QSortFilterProxyModel>
#include <layout_list_model_base.h>

class LayoutListSortByPriorityProxyModel : public QSortFilterProxyModel, public LayoutListModelBase {
    Q_OBJECT
public:
    LayoutListSortByPriorityProxyModel(QObject* parent);

    Q_INVOKABLE void applyOrderChanges();
    Q_INVOKABLE void simulateMove(int src, int dst);
    Q_INVOKABLE void remove(int idx);

    //bool lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const override;

Q_SIGNALS:
    void enabledOrderChanged(QVector<int> const& newOrder);
    void itemRemoved(int idx);
    void itemAdded(QString const& saveName);

private:
    QVector<int> m_tempMapping;
};

#endif // LAYOUTLISTENABLEDPROXYMODEL_H
