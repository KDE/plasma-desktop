#pragma once

#include <QAbstractListModel>
#include "abstract_advanced_model.h"

class AdvancedModel : public QAbstractListModel
{
    Q_OBJECT

public:

    struct Roles {
        enum {
            DescriptionRole = Qt::DisplayRole,
            NameRole = Qt::UserRole + 1,
            SectionNameRole,
            SectionDescriptionRole,
            ExclusiveRole,
            SelectedRole,
            IsGroupRole,
        };
    };
    
    explicit AdvancedModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    
    Qt::ItemFlags flags(const QModelIndex& index) const override;

    QHash<int, QByteArray> roleNames() const override;
    
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    bool setData(const QModelIndex &index, const QVariant &value, int role) override;

    QStringList enabledOptions() const;
    void setEnabledOptions(const QStringList &enabledOptions);

private:
    QStringList m_enabledOptions;
    
    // cached value for faster data access
    int m_ruleCount = 0;
    QList<int> m_ruleCountByGroupIndex;
};
