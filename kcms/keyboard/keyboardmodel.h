/*
    SPDX-FileCopyrightText: 2024 Evgeny Chesnokov <echesnokov@astralinux.ru>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QAbstractListModel>

class KeyboardModel final : public QAbstractListModel
{
    Q_OBJECT

    enum Roles {
        DescriptionRole = Qt::UserRole + 1,
        NameRole,
    };

public:
    explicit KeyboardModel(QObject *parent) noexcept;

    Q_INVOKABLE int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    Q_INVOKABLE QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;
};
