/*
    SPDX-FileCopyrightText: 2024 ivan tkachenko <me@ratijas.tk>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QAbstractListModel>
#include <qnamespace.h>

class KeysModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum Roles {
        KeyRole = Qt::UserRole,
    };
    Q_ENUM(Roles)

    explicit KeysModel(QObject *parent = nullptr);
    virtual ~KeysModel() override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

private:
    struct Entry {
        QString display;
        Qt::Key key;
    };
    QList<Entry> m_entries;
};
