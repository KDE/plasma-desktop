/*
    SPDX-FileCopyrightText: 2024 Evgeny Chesnokov <echesnokov@astralinux.ru>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "keyboardmodel.h"

#include <KLocalizedString>

#include "xkb_rules.h"

KeyboardModel::KeyboardModel(Rules *rules, QObject *parent) noexcept
    : QAbstractListModel(parent)
    , m_rules(rules)
{
}

int KeyboardModel::rowCount(const QModelIndex &parent) const
{
    if (!m_rules)
        return 0;

    return m_rules->modelInfos.count();
}

QVariant KeyboardModel::data(const QModelIndex &index, int role) const
{
    if (!m_rules)
        return QVariant();

    if (!index.isValid())
        return QVariant();

    if (index.row() >= m_rules->modelInfos.size())
        return QVariant();

    const ModelInfo modelInfo = m_rules->modelInfos.at(index.row());

    QString vendor = modelInfo.vendor;
    if (vendor.isEmpty()) {
        vendor = i18nc("unknown keyboard model vendor", "Unknown");
    }

    if (role == Roles::DescriptionRole || role == Qt::DisplayRole) {
        return QVariant::fromValue(i18nc("vendor | keyboard model", "%1 | %2", vendor, modelInfo.description));
    } else if (role == Roles::NameRole) {
        return QVariant::fromValue(modelInfo.name);
    }

    return QVariant();
}

QHash<int, QByteArray> KeyboardModel::roleNames() const
{
    return {
        {Qt::DisplayRole, QByteArrayLiteral("display")},
        {Roles::DescriptionRole, QByteArrayLiteral("description")},
        {Roles::NameRole, QByteArrayLiteral("name")},
    };
}

#include "keyboardmodel.moc"
#include "moc_keyboardmodel.cpp"
