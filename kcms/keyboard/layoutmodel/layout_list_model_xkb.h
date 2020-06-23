#ifndef LAYOUTLISTMODEL_H
#define LAYOUTLISTMODEL_H

#include <QAbstractItemModel>

#include "layout_list_model_base.h"
#include "../xkb_rules.h"

class LayoutListModelXkb : public QAbstractItemModel, public LayoutListModelBase {
    Q_OBJECT

public:
    explicit LayoutListModelXkb(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;

    int columnCount(const QModelIndex& parent = QModelIndex()) const override;

    Qt::ItemFlags flags(const QModelIndex& index) const override;

    QModelIndex index(int row, int column,
        const QModelIndex& parent = QModelIndex()) const override;

    QModelIndex parent(const QModelIndex& index) const override;

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

private:
    struct Item {
        virtual ~Item() = default;
    };

    struct Layout;
    struct LayoutVariant : Item {
        QString id;
        QString description;
        QVariantList languages;
        int parent_index;
    };

    struct Layout : Item {
        QString id;
        QString description;
        QVariantList languages;
        QList<LayoutVariant> variants;
    };

    using LayoutList = QList<Layout>;

    QScopedPointer<LayoutList> m_layouts;
};

#endif // LAYOUTLISTMODEL_H
