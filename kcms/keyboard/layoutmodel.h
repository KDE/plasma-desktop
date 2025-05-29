/*
    SPDX-FileCopyrightText: 2024 Evgeny Chesnokov <echesnokov@astralinux.ru>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QAbstractListModel>
#include <qqmlintegration.h>

class LayoutModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT

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
    explicit LayoutModel() noexcept;

    Q_INVOKABLE int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    Q_INVOKABLE QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;
    enum Roles {
        ShortNameRole = Qt::UserRole + 1,
        DescripionRole,
        VariantNameRole,
    };

private:
    QList<Data> m_data;
};
