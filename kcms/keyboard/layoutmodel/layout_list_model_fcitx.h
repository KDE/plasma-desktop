#ifndef LAYOUTLISTMODELFCITX_H
#define LAYOUTLISTMODELFCITX_H

#include <QAbstractItemModel>
#include <QDBusMessage>

#include "layout_list_model_base.h"

class LayoutListModelFcitx : public QAbstractItemModel, public LayoutListModelBase {
    Q_OBJECT

public:
    explicit LayoutListModelFcitx(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;

    int columnCount(const QModelIndex& parent = QModelIndex()) const override;

    Qt::ItemFlags flags(const QModelIndex& index) const override;

    QModelIndex index(int row, int column = 0,
        const QModelIndex& parent = QModelIndex()) const override;

    QModelIndex parent(const QModelIndex& index) const override;

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

private:
    void load();

private:
    struct Layout {
        QString save_name;
        QString layout_id;
        QString description;
        QString language;
        bool enabled;
    };

    QScopedPointer<QVector<Layout>> m_layouts;
    int m_numEnabled = 0;
};

#endif
