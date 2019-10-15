#include "xkb_layout_config_model.h"

#include "../xkb_rules.h"
#include <QDebug>

XkbLayoutConfigModel::XkbLayoutConfigModel(XkbRules* rules, QObject* parent)
    : QAbstractItemModel(parent)
    , m_rules(rules)
{
}

QModelIndex XkbLayoutConfigModel::index(int row, int column, const QModelIndex&) const
{
    return createIndex(row, column, quintptr(row));
}

QModelIndex XkbLayoutConfigModel::parent(const QModelIndex&) const
{
    return QModelIndex();
}

int XkbLayoutConfigModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return m_rules->optionGroupInfos.size();
}

int XkbLayoutConfigModel::columnCount(const QModelIndex& parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return m_rules->optionGroupInfos.size();
}

QVariant XkbLayoutConfigModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    if (role == Roles::DescriptionRole) {
        return m_rules->optionGroupInfos[index.row()]->description;
    }
    return QVariant();
}

QHash<int, QByteArray> XkbLayoutConfigModel::roleNames() const
{
    return {
        { Roles::DescriptionRole, "description" }
    };
}

void XkbLayoutConfigModel::save() const
{
    qDebug() << "save";
}
