/*
    SPDX-FileCopyrightText: 2024 Evgeny Chesnokov <echesnokov@astralinux.ru>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "layoutmodel.h"

#include "debug.h"
#include "keyboardsettings.h"
#include "x11_helper.h"
#include "xkb_rules.h"

LayoutModel::LayoutModel(Rules *rules, QObject *parent) noexcept
    : QAbstractListModel(parent)
    , m_rules(rules)
{
    if (m_rules) {
        for (const LayoutInfo &layoutInfo : std::as_const(m_rules->layoutInfos)) {
            m_data.append(Data(layoutInfo.name, layoutInfo.description, QStringLiteral("")));

            for (const VariantInfo &variantInfo : layoutInfo.variantInfos) {
                m_data.append(Data(layoutInfo.name, variantInfo.description, variantInfo.name));
            }
        }
    }
}

int LayoutModel::rowCount(const QModelIndex &parent) const
{
    return m_data.count();
}

QVariant LayoutModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.row() >= m_data.size())
        return QVariant();

    const auto &layout = m_data.at(index.row());

    if (role == Roles::DescripionRole || role == Qt::DisplayRole) {
        return layout.description;
    } else if (role == Roles::ShortNameRole) {
        return layout.name;
    } else if (role == Roles::VariantNameRole) {
        return layout.variantName;
    }

    return QVariant();
}

QHash<int, QByteArray> LayoutModel::roleNames() const
{
    return {
        {Qt::DisplayRole, QByteArrayLiteral("display")},
        {Roles::ShortNameRole, QByteArrayLiteral("shortName")},
        {Roles::DescripionRole, QByteArrayLiteral("description")},
        {Roles::VariantNameRole, QByteArrayLiteral("variantName")},
    };
}

#include "layoutmodel.moc"
#include "moc_layoutmodel.cpp"
