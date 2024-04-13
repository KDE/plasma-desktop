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

    Q_PROPERTY(QItemSelectionModel *selectionModel READ selectionModel CONSTANT)

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

    QItemSelectionModel *selectionModel() const;

    Q_INVOKABLE int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    Q_INVOKABLE QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    QHash<int, QByteArray> roleNames() const override;

    void reset();
    Q_INVOKABLE void clear();
    Q_INVOKABLE void moveSelectedLayouts(int shift);

    Q_INVOKABLE void addLayout(const QString &layout, const QString &variant, const QKeySequence &shortcut, const QString &displayName = QString());
    Q_INVOKABLE void removeSelected();

private:
    QItemSelectionModel *const m_selectionModel;
    KeyboardConfig *const m_config;
};
