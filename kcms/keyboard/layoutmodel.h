/*
    SPDX-FileCopyrightText: 2024 Evgeny Chesnokov <echesnokov@astralinux.ru>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QAbstractListModel>

class LayoutModel final : public QAbstractListModel
{
    Q_OBJECT
    enum Roles {
        ShortNameRole = Qt::UserRole + 1,
        DescripionRole,
        VariantNameRole,
    };

    struct Data {
        explicit Data(const QString &_name, const QString &_description, const QString &_variantName)
            : name(_name)
            , description(_description)
            , variantName(_variantName)
        {
        }

        QString name;
        QString description;
        QString variantName;
    };

public:
    explicit LayoutModel(QObject *parent) noexcept;

    Q_INVOKABLE int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    Q_INVOKABLE QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

private:
    QList<Data> m_data;
};
