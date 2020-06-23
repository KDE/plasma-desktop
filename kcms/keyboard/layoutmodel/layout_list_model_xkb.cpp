#include "layout_list_model_xkb.h"

#include "../xkb_rules.h"
#include "xkb_layout_config_model.h"
#include "input_sources.h"
#include <memory>
#include <QDebug>

LayoutListModelXkb::LayoutListModelXkb(QObject* parent)
    : QAbstractItemModel(parent),
      m_layouts(new LayoutList)
{
    for (int i = 0; i < XkbRules::self()->layoutInfos.count(); ++i) {
        auto const& layout_info = XkbRules::self()->layoutInfos[i];

        Layout layout;
        layout.id = layout_info->name;
        layout.description = layout_info->description;

        for (auto const& lang : layout_info->languages)
            layout.languages << lang;

        // add rest of the variants in
        for (auto const& var_info : layout_info->variantInfos) {
            LayoutVariant var;
            var.id = var_info->name;
            var.description = var_info->description;
            var.parent_index = i;

            for (auto const& lang : var_info->languages) {
                var.languages << lang;
            }

            layout.variants << var;
        }

        *m_layouts << layout;
    }
}

int LayoutListModelXkb::rowCount(const QModelIndex& parent) const
{
    if (parent.column() > 0) {
        return 0;
    }

    if (!parent.isValid()) {
        return m_layouts->count();
    }

    Item* item = static_cast<Item*>(parent.internalPointer());
    if (Layout* layout = dynamic_cast<Layout*>(item)) {
        return layout->variants.count();
    }

    return 0;
}

int LayoutListModelXkb::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return 1;
}

Qt::ItemFlags LayoutListModelXkb::flags(const QModelIndex& index) const
{
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    }

    return QAbstractItemModel::flags(index);
}

QModelIndex LayoutListModelXkb::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent)) {
        return QModelIndex();
    }

    if (!parent.isValid()) { // it is referring to the top-level element
        if (0 <= row && row < m_layouts->count()) {
            return createIndex(row, column, &(*m_layouts)[row]);
        }
    } else {
        Layout* parent_item = static_cast<Layout*>(parent.internalPointer());
        if (0 <= row && row < parent_item->variants.count()) {
            return createIndex(row, column, &parent_item->variants[row]);
        }
    }

    return QModelIndex();
}

QModelIndex LayoutListModelXkb::parent(const QModelIndex& index) const
{
    if (!index.isValid()) {
        return QModelIndex();
    }

    Item* item = static_cast<Item*>(index.internalPointer());
    if (LayoutVariant* variant = dynamic_cast<LayoutVariant*>(item)) {
        return createIndex(variant->parent_index, 0, &(*m_layouts)[variant->parent_index]);
    }

    return QModelIndex();
}

QVariant LayoutListModelXkb::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    Item* item = static_cast<Item*>(index.internalPointer());
    if (LayoutVariant* variant = dynamic_cast<LayoutVariant*>(item)) {
        switch (role) {
        case Roles::ShortNameRole:
        case Roles::NameRole:
        case Roles::SaveNameRole:
        case Qt::DisplayRole:
            return QString("%1(%2)").arg((*m_layouts)[variant->parent_index].id, variant->id);
        case Roles::DescriptionRole:
            return variant->description;
        case Roles::LanguagesRole:
            return variant->languages;
        }
    } else if (Layout* layout = dynamic_cast<Layout*>(item)) {
        switch (role) {
        case Roles::ShortNameRole:
        case Roles::NameRole:
        case Roles::SaveNameRole:
        case Qt::DisplayRole:
            return layout->id;
        case Roles::DescriptionRole:
            return layout->description;
        case Roles::LanguagesRole:
            return layout->languages;
        }
    }

    switch (role) {
    case Roles::IsConfigurableRole:
        return true;
    case Roles::SourceRole:
        return InputSources::Sources::XkbSource;
    case Roles::ConfigModelRole:
        // TODO: manage memory?
        return QVariant::fromValue(new XkbLayoutConfigModel(XkbRules::self(), nullptr));
    }

    return QVariant();
}
