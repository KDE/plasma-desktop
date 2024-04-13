/*
    SPDX-FileCopyrightText: 2024 Evgeny Chesnokov <echesnokov@astralinux.ru>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "userlayoutmodel.h"

#include <QItemSelectionModel>

#include "debug.h"
#include "keyboard_config.h"
#include "x11_helper.h"
#include "xkb_rules.h"

UserLayoutModel::UserLayoutModel(KeyboardConfig *config, QObject *parent) noexcept
    : QAbstractListModel(parent)
    , m_selectionModel(new QItemSelectionModel(this))
    , m_config(config)
{
}

QItemSelectionModel *UserLayoutModel::selectionModel() const
{
    return m_selectionModel;
}

int UserLayoutModel::rowCount(const QModelIndex &parent) const
{
    return m_config->layouts().count();
}

QVariant UserLayoutModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.row() >= m_config->layouts().size())
        return QVariant();

    const auto &layout = m_config->layouts().at(index.row());

    if (role == Roles::LayoutRole) {
        return QVariant::fromValue(layout.layout());
    } else if (role == Roles::LayoutNameRole) {
        const std::optional<LayoutInfo> layoutInfo = Rules::self().getLayoutInfo(layout.layout());
        return layoutInfo ? layoutInfo->description : layout.layout();
    } else if (role == Roles::VariantRole) {
        return QVariant::fromValue(layout.variant());
    } else if (role == Roles::VariantNameRole) {
        if (layout.variant().isEmpty())
            return QVariant();

        const std::optional<LayoutInfo> layoutInfo = Rules::self().getLayoutInfo(layout.layout());
        if (!layoutInfo)
            return QVariant();

        const std::optional<VariantInfo> variantInfo = layoutInfo->getVariantInfo(layout.variant());
        return variantInfo ? variantInfo->description : layout.variant();
    } else if (role == Roles::DisplayNameRole) {
        return QVariant::fromValue(layout.getDisplayName());
    } else if (role == Roles::ShortcutRole) {
        return QVariant::fromValue(layout.getShortcut());
    }

    return QVariant();
}

bool UserLayoutModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role != Roles::DisplayNameRole && role != Roles::ShortcutRole && role != Roles::VariantRole) {
        return false;
    }

    if (index.row() >= m_config->layouts().size() || index.data(role) == value) {
        return false;
    }

    LayoutUnit &layoutUnit = m_config->layouts()[index.row()];

    if (role == Roles::DisplayNameRole) {
        QString displayText = value.toString().left(3);
        layoutUnit.setDisplayName(displayText);
        Q_EMIT dataChanged(index, index, {Roles::DisplayNameRole});
        return true;
    }

    if (role == Roles::ShortcutRole) {
        layoutUnit.setShortcut(QKeySequence(value.toString()));
        Q_EMIT dataChanged(index, index, {Roles::ShortcutRole});
        return true;
    }

    if (role == Roles::VariantRole) {
        layoutUnit.setVariant(value.toString());
        Q_EMIT dataChanged(index, index, {Roles::VariantRole, Roles::VariantNameRole});
        return true;
    }

    return false;
}

QHash<int, QByteArray> UserLayoutModel::roleNames() const
{
    return {
        {Roles::LayoutRole, QByteArrayLiteral("layout")},
        {Roles::LayoutNameRole, QByteArrayLiteral("layoutName")},
        {Roles::VariantRole, QByteArrayLiteral("variant")},
        {Roles::VariantNameRole, QByteArrayLiteral("variantName")},
        {Roles::DisplayNameRole, QByteArrayLiteral("displayName")},
        {Roles::ShortcutRole, QByteArrayLiteral("shortcut")},
    };
}

void UserLayoutModel::reset()
{
    m_selectionModel->clear();

    beginResetModel();
    endResetModel();
}

void UserLayoutModel::clear()
{
    m_config->layouts().clear();
    reset();
}

void UserLayoutModel::moveSelectedLayouts(int shift)
{
    if (!m_selectionModel->hasSelection()) {
        return;
    }

    const QModelIndexList &selected = m_selectionModel->selectedRows();

    QList<int> rows(selected.size());
    std::transform(selected.cbegin(), selected.cend(), rows.begin(), [](const QModelIndex &index) -> int {
        return index.row();
    });

    std::sort(rows.begin(), rows.end(), [shift](int left, int right) -> bool {
        return shift < 0 ? left < right : left > right;
    });

    const int newFirstRow = (shift > 0 ? rows.first() : rows.last()) + shift;
    const int newLastRow = (shift > 0 ? rows.last() : rows.first()) + shift;

    if (newFirstRow < 0 || newLastRow >= m_config->layouts().size()) {
        return;
    }

    m_selectionModel->clearSelection();
    for (const int row : rows) {
        const int destinationRow = row + shift;

        if (beginMoveRows(QModelIndex(), row, row, QModelIndex(), row < destinationRow ? destinationRow + 1 : destinationRow)) {
            m_config->layouts().move(row, destinationRow);
            endMoveRows();
        }

        m_selectionModel->select(UserLayoutModel::index(destinationRow, 0), QItemSelectionModel::SelectionFlag::Select);
    }
}

void UserLayoutModel::addLayout(const QString &layout, const QString &variant, const QKeySequence &shortcut, const QString &displayName)
{
    LayoutUnit unit = LayoutUnit(layout, variant);
    unit.setShortcut(shortcut);

    if (!displayName.isEmpty()) {
        unit.setDisplayName(displayName);
    }

    beginInsertRows(QModelIndex(), m_config->layouts().count(), m_config->layouts().count());
    m_config->layouts().append(unit);
    endInsertRows();
}

void UserLayoutModel::removeSelected()
{
    if (!m_selectionModel->hasSelection()) {
        return;
    }

    const QModelIndexList &selected = m_selectionModel->selectedRows();

    QList<int> rows(selected.size());
    std::transform(selected.cbegin(), selected.cend(), rows.begin(), [](const QModelIndex &index) -> int {
        return index.row();
    });

    std::sort(rows.begin(), rows.end(), [](int left, int right) -> bool {
        return left > right;
    });

    for (const int row : rows) {
        beginRemoveRows(QModelIndex(), row, row);
        m_config->layouts().removeAt(row);
        endRemoveRows();
    }
}

#include "moc_userlayoutmodel.cpp"
#include "userlayoutmodel.moc"
