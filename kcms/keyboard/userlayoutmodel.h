/*
    SPDX-FileCopyrightText: 2024 Evgeny Chesnokov <echesnokov@astralinux.ru>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "layoutunit.h"

#include <QAbstractListModel>

class KeyboardSettings;

class UserLayoutModel final : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Roles {
        LayoutRole = Qt::UserRole + 1,
        LayoutNameRole,
        VariantRole,
        VariantNameRole,
        DisplayNameRole,
        ShortcutRole,
    };

    explicit UserLayoutModel(KeyboardSettings *settings, QObject *parent) noexcept;

    void resetToSettings();
    void reset(const QList<LayoutUnit> &layouts);
    void applyToSettings();

    Q_INVOKABLE int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    Q_INVOKABLE QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void move(int oldIndex, int newIndex);
    Q_INVOKABLE void remove(int index);

    Q_INVOKABLE void addLayout(const QString &layout, const QString &variant, const QKeySequence &shortcut, const QString &displayName = QString());
    Q_INVOKABLE void setSingleLayout(const QString &layout, const QString &variant, const QKeySequence &shortcut, const QString &displayName = QString());

    LayoutUnit layoutUnit(int row) const;

private:
    QList<LayoutUnit> m_layouts;
    KeyboardSettings *const m_settings;
};
