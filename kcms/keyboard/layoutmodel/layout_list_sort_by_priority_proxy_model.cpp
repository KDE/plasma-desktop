#include "layout_list_sort_by_priority_proxy_model.h"

#include <QDebug>

LayoutListSortByPriorityProxyModel::LayoutListSortByPriorityProxyModel(QObject* parent)
    : QSortFilterProxyModel(parent)
{
    setSortRole(Roles::PriorityRole);
    sort(0);
}

void LayoutListSortByPriorityProxyModel::applyOrderChanges()
{
    if (!m_tempMapping.empty()) {
        qDebug() << "applyorderchanges" << m_tempMapping;
        Q_EMIT enabledOrderChanged(m_tempMapping);
        m_tempMapping.clear();
    }
}

void LayoutListSortByPriorityProxyModel::simulateMove(int src, int dst)
{
    qDebug() << src << dst;
    if (src < 0 || src >= rowCount() || dst < 0 || dst >= rowCount()) {
        return;
    }

    int const modelTo = dst + (dst > src);
    if (beginMoveRows(QModelIndex(), src, src, QModelIndex(), modelTo)) {
        if (m_tempMapping.empty()) {
            m_tempMapping.resize(sourceModel()->rowCount());
            std::iota(m_tempMapping.begin(), m_tempMapping.end(), 0);
        }
        m_tempMapping.move(src, dst);
        qDebug() << m_tempMapping;
        endMoveRows();
    }
}

void LayoutListSortByPriorityProxyModel::remove(int idx)
{
    Q_EMIT itemRemoved(idx);
}
/*
bool LayoutListSortByPriorityProxyModel::lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const
{
    bool result;
    if (m_tempMapping.empty()) {
        int left = sourceModel()->data(source_left, Roles::PriorityRole).toInt();
        int right = sourceModel()->data(source_right, Roles::PriorityRole).toInt();
        result = left < right;
    }
    else {
        result = m_tempMapping[source_left.row()] < m_tempMapping[source_right.row()];
    }

    QString left, right;
    left = sourceModel()->data(source_left, Roles::NameRole).toString();
    right = sourceModel()->data(source_right, Roles::NameRole).toString();
    if (result) {
        qDebug() << left << "<" << right;
    }
    else {
        qDebug() << right << "<" << left;
    }

    return result;
}
*/
