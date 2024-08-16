/*
    SPDX-FileCopyrightText: 2024 Evgeny Chesnokov <echesnokov@astralinux.ru>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QAbstractListModel>

class KeyboardConfig;
class LayoutUnit;
class QItemSelectionModel;

class UserLayoutModel final : public QAbstractListModel
{
    Q_OBJECT

    enum Roles {
        LayoutRole = Qt::UserRole + 1,
        LayoutNameRole,
        VariantRole,
        VariantNameRole,
        DisplayNameRole,
        ShortcutRole,
    };

public:
    explicit UserLayoutModel(KeyboardConfig *config, QObject *parent) noexcept;

    Q_INVOKABLE int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    Q_INVOKABLE QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    QHash<int, QByteArray> roleNames() const override;

    void reset();
    void clear();
    Q_INVOKABLE void move(int oldIndex, int newIndex);
    Q_INVOKABLE void remove(int index);

    Q_INVOKABLE void addLayout(const QString &layout, const QString &variant, const QKeySequence &shortcut, const QString &displayName = QString());

private:
    QItemSelectionModel *const m_selectionModel;
    KeyboardConfig *const m_config;
};
