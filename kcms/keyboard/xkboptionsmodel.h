/*
    SPDX-FileCopyrightText: 2024 Evgeny Chesnokov <echesnokov@astralinux.ru>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QAbstractItemModel>

class KeyboardSettings;

class XkbOptionsModel final : public QAbstractItemModel
{
    Q_OBJECT

public:
    XkbOptionsModel(QObject *parent);

    void setXkbOptions(const QStringList &options);
    QStringList xkbOptions() const;

    int columnCount(const QModelIndex &) const override;
    int rowCount(const QModelIndex &parent) const override;
    QModelIndex parent(const QModelIndex &index) const override;
    QModelIndex index(int row, int column, const QModelIndex &parent) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QHash<int, QByteArray> roleNames() const override;

    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    QVariant data(const QModelIndex &index, int role) const override;
    void reset();

    Q_INVOKABLE QString getShortcutName(const QString &group);
    Q_INVOKABLE void clearXkbGroup(const QString &group);
    Q_INVOKABLE void navigateToGroup(const QString &group);

    void populateWithCurrentXkbOptions();

Q_SIGNALS:
    void navigateTo(const QModelIndex &index);

private:
    QStringList m_xkbOptions;
};
