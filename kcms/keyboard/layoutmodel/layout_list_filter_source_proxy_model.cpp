#include "layout_list_filter_source_proxy_model.h"

#include "input_sources.h"
#include "layout_list_concat_proxy_model.h"

#include <QDebug>

LayoutListFilterSourceProxyModel::LayoutListFilterSourceProxyModel(QObject* parent)
    : QSortFilterProxyModel(parent)
{
    QObject::connect(InputSources::self(), &InputSources::currentSourceChanged,
        [&] { invalidateFilter(); });
}

bool LayoutListFilterSourceProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex&) const
{
    int src = sourceModel()->data(sourceModel()->index(sourceRow, 0), Roles::SourceRole).toInt();
    if (src == InputSources::Sources::UnknownSource) {
        return true;
    }
    return (src == InputSources::self()->currentSource());
}
