/*
    SPDX-FileCopyrightText: 2024 ivan tkachenko <me@ratijas.tk>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "keysmodel.h"

#include <KLocalizedContext>
#include <KLocalizedString>

#include <QKeySequence>
#include <qmetaobject.h>

using namespace Qt::StringLiterals;

KeysModel::KeysModel(QObject *parent)
    : QAbstractListModel(parent)
{
    const auto metaEnum = QMetaEnum::fromType<Qt::Key>();

    m_entries.append(Entry{i18nc("@label No key is selected for a key combination", "None"), static_cast<Qt::Key>(0)});

    for (int i = 0, count = metaEnum.keyCount(); i < count; i++) {
        const auto key = static_cast<Qt::Key>(metaEnum.value(i));
        auto display = QKeySequence(key).toString();

        m_entries.append(Entry{display, key});
    }
}

KeysModel::~KeysModel() = default;

int KeysModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return m_entries.count();
}

QVariant KeysModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.parent().isValid()) {
        return {};
    }

    const auto &entry = m_entries[index.row()];

    switch (role) {
    case Qt::DisplayRole:
        return entry.display;
    case KeyRole:
        return entry.key;
    default:
        return {};
    }
}

QHash<int, QByteArray> KeysModel::roleNames() const
{
    return {
        {Qt::DisplayRole, "display"},
        {KeyRole, "key"},
    };
}

#include "moc_keysmodel.cpp"
