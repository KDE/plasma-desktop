#ifndef ADVANCEDMODEL_H
#define ADVANCEDMODEL_H

#include <QAbstractItemModel>
#include "abstract_advanced_model.h"

class AdvancedModel : public QAbstractItemModel, public AbstractAdvancedModel
{
    Q_OBJECT

public:
    explicit AdvancedModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;

    int columnCount(const QModelIndex& parent = QModelIndex()) const override;

    Qt::ItemFlags flags(const QModelIndex& index) const override;

    QModelIndex index(int row, int column,
        const QModelIndex& parent = QModelIndex()) const override;

    QModelIndex parent(const QModelIndex& index) const override;

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    bool setData(const QModelIndex &index, const QVariant &value, int role) override;

    QStringList enabledOptions() const;
    void setEnabledOptions(const QStringList &enabledOptions);

private:
    QStringList m_enabledOptions;
};

#endif // ADVANCEDMODEL_H
