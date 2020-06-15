#include "advanced_model.h"
#include "../xkb_rules.h"
#include <QDebug>

AdvancedModel::AdvancedModel(QObject *parent)
    : QAbstractListModel(parent)
{
    for (const auto groupRule : qAsConst(XkbRules::self()->optionGroupInfos)) {
        m_ruleCountByGroupIndex.append(m_ruleCount);
        m_ruleCount += groupRule->optionInfos.size();
    }
}

QHash<int, QByteArray> AdvancedModel::roleNames() const
{
    return {
        { Roles::DescriptionRole, "description" },
        { Roles::NameRole, "name" },
        { Roles::SectionNameRole, "sectionName" },
        { Roles::SectionDescriptionRole, "sectionDescription" },
        { Roles::SelectedRole, "selected" },
        { Roles::ExclusiveRole, "exclusive" },
        { Roles::IsGroupRole, "isGroup" },
    };
}


int AdvancedModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_ruleCount;
}

Qt::ItemFlags AdvancedModel::flags(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    }

    return QAbstractListModel::flags(index);
}

QVariant AdvancedModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }
    int groupIndex = 0;
    int indexInGroup = 0;
    for (int i : m_ruleCountByGroupIndex) {
        if (i + XkbRules::self()->optionGroupInfos[groupIndex]->optionInfos.size() > index.row()) {
            indexInGroup = index.row() - i;
            break;
        }
        groupIndex++;
    }
    
    auto const &groupInfo = XkbRules::self()->optionGroupInfos[groupIndex];
    auto const &info = XkbRules::self()->optionGroupInfos[groupIndex]->optionInfos[indexInGroup];

    switch (role) {
        case Roles::DescriptionRole:
            return info->description;
        case Roles::NameRole:
            return info->name;
        case Roles::SectionNameRole:
            return groupInfo->name;
        case Roles::SectionDescriptionRole:
            return groupInfo->description;
        case Roles::SelectedRole:
            return m_enabledOptions.contains(info->name);
        case Roles::ExclusiveRole:
            return groupInfo->exclusive;
    }

    return QVariant();
}

bool AdvancedModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role == Roles::SelectedRole) {
        int groupIndex = 0;
        int indexInGroup = 0;
        for (int i : m_ruleCountByGroupIndex) {
            if (i + XkbRules::self()->optionGroupInfos[groupIndex]->optionInfos.size() > index.row()) {
                indexInGroup = index.row() - i;
                break;
            }
            groupIndex++;
        }
        auto const &groupInfo = XkbRules::self()->optionGroupInfos[groupIndex];
        auto const &info = XkbRules::self()->optionGroupInfos[groupIndex]->optionInfos[indexInGroup];
        
        if (!value.toBool()) {
            m_enabledOptions.removeAll(info->name);
        } else if (value.toBool()) {
            m_enabledOptions << info->name;
        }

        qDebug() << m_enabledOptions;
        Q_EMIT dataChanged(index, index, { Roles::SelectedRole });
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
