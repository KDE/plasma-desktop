#include "advanced_model.h"
#include "../xkb_rules.h"
#include <QDebug>

AdvancedModel::AdvancedModel(QObject *parent)
    : QAbstractItemModel(parent)
{
}

int AdvancedModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid()) { // root
        return XkbRules::self()->optionGroupInfos.size();
    }

    if (!parent.parent().isValid()) { // 2nd level
        return XkbRules::self()->optionGroupInfos[parent.row()]->optionInfos.size();
    }

    return 0;
}

int AdvancedModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 1;
}

Qt::ItemFlags AdvancedModel::flags(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    }

    return QAbstractItemModel::flags(index);
}

QModelIndex AdvancedModel::index(int row, int column, const QModelIndex &parent) const
{
    if (column > 0) return QModelIndex();
    if (!parent.isValid()) {
        return createIndex(row, column, quintptr(-1));
    }
    else if (!parent.parent().isValid()){
        return createIndex(row, column, quintptr(parent.row()));
    }
    return QModelIndex();
}

QModelIndex AdvancedModel::parent(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return QModelIndex();
    }

    if (index.internalId() == quintptr(-1)) { // root
        return QModelIndex();
    }

    return createIndex(static_cast<int>(index.internalId()), 0, quintptr(-1));
}

QVariant AdvancedModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    if (!index.parent().isValid()) {
        auto const& infos = XkbRules::self()->optionGroupInfos[index.row()];

        switch (role) {
        case Roles::DescriptionRole:
            return infos->description;
        case Roles::NameRole:
        case Roles::SectionNameRole:
            return infos->name;
        case Roles::SelectedRole:
            return false;
        case Roles::ExclusiveRole:
            return infos->exclusive;
        case Roles::IsGroupRole:
            return true;
        case Roles::SectionNamePlusIsGroupRole:
            return QString(infos->name + "+true");
        }
    }
    else if (!index.parent().parent().isValid()){
        auto const& info = XkbRules::self()->optionGroupInfos[index.parent().row()]->optionInfos[index.row()];

        switch (role) {
        case Roles::DescriptionRole:
            return info->description;
        case Roles::NameRole:
            return info->name;
        case Roles::SectionNameRole:
            return data(index.parent(), Roles::SectionNameRole);
        case Roles::SelectedRole:
            return m_enabledOptions.contains(info->name);
        case Roles::ExclusiveRole:
            return data(index.parent(), Roles::ExclusiveRole);
        case Roles::IsGroupRole:
            return false;
        case Roles::SectionNamePlusIsGroupRole:
            return QString(data(index.parent(), Roles::SectionNameRole).toString() + "+false");
        }
    }

    return QVariant();
}

bool AdvancedModel::setData(const QModelIndex &idx, const QVariant &value, int role)
{
    if (role == Roles::SelectedRole) {
        QString optionName = data(idx, Roles::NameRole).toString();
        bool exclusive = data(idx, Roles::ExclusiveRole).toBool();

        m_enabledOptions.removeAll(optionName);
        if (exclusive) {
            for (int i = 0; i < m_enabledOptions.size(); ++i) {
                QString name = m_enabledOptions[i];
                if (name.split(":")[0] == optionName.split(":")[0]) {
                    m_enabledOptions.removeAt(i);

                    for (int j = 0; j < rowCount(idx.parent()); ++j) {
                        QModelIndex otherIdx = index(j, 0, idx.parent());
                        if (data(otherIdx, Roles::NameRole).toString() == name) {
                            emit dataChanged(otherIdx, otherIdx, { Roles::SelectedRole });
                        }
                    }

                }
            }
        }

        if (value.toBool()) {
            m_enabledOptions << optionName;
        }

        emit dataChanged(idx, idx, { Roles::SelectedRole });
        qDebug() << m_enabledOptions;
        return true;
    }
    return false;
}

QStringList AdvancedModel::enabledOptions() const
{
    return m_enabledOptions;
}

void AdvancedModel::setEnabledOptions(const QStringList &enabledOptions)
{
    m_enabledOptions = enabledOptions;
}
