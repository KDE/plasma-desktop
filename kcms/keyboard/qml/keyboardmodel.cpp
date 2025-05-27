/*
    SPDX-FileCopyrightText: 2024 Evgeny Chesnokov <echesnokov@astralinux.ru>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "keyboardmodel.h"

#include <KLocalizedString>

#include "xkb_rules.h"

KeyboardModel::KeyboardModel(QObject *parent) noexcept
    : QAbstractListModel(parent)
{
}

int KeyboardModel::rowCount(const QModelIndex &parent) const
{
    return Rules::self().modelInfos.count();
}

QVariant KeyboardModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.row() >= Rules::self().modelInfos.size())
        return QVariant();

    const ModelInfo modelInfo = Rules::self().modelInfos.at(index.row());

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
