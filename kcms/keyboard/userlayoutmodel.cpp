/*
    SPDX-FileCopyrightText: 2024 Evgeny Chesnokov <echesnokov@astralinux.ru>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "userlayoutmodel.h"

#include <QItemSelectionModel>
#include <utility>

#include "debug.h"
#include "keyboardsettings.h"
#include "layoutunit.h"
#include "xkb_rules.h" // LayoutInfo

UserLayoutModel::UserLayoutModel(KeyboardSettings *settings, QObject *parent) noexcept
    : QAbstractListModel(parent)
    , m_settings(settings)
{
    connect(m_settings, &KeyboardSettings::configureLayoutsChanged, this, [this] {
        // if !configureLayouts, the model will claim rowCount() == 0 without actually dropping data
        beginResetModel();
        endResetModel();
    });
}

void UserLayoutModel::resetToSettings()
{
    beginResetModel();

    const QStringList layoutStrings = m_settings->layoutList();
    const QStringList variants = m_settings->variantList();
    const QStringList names = m_settings->displayNames();

    m_layouts.clear();
    for (int i = 0; i < layoutStrings.size(); ++i) {
        m_layouts.append({layoutStrings[i], variants.value(i, QString())});

        if (const QString name = names.value(i); name != layoutStrings[i]) {
            m_layouts[i].setDisplayName(name);
        }
    }
    // layouts' shortcuts are retrieved by ShortcutHelper's KeyboardLayoutActionCollection

    endResetModel();
}

void UserLayoutModel::reset(const QList<LayoutUnit> &layouts)
{
    beginResetModel();

    m_layouts = layouts;
    applyToSettings();

    endResetModel();
}

void UserLayoutModel::applyToSettings()
{
    QStringList layoutStrings;
    QStringList variants;
    QStringList displayNames;

    for (const LayoutUnit &layoutUnit : std::as_const(m_layouts)) {
        layoutStrings.append(layoutUnit.layout());
        variants.append(layoutUnit.variant());
        displayNames.append(layoutUnit.getRawDisplayName());
    }

    // QStringLists with a single empty string are serialized as "\\0", avoid that
    // by saving them as an empty list instead. This way it can be passed as-is to
    // libxkbcommon/setxkbmap. Before KConfigXT it used QStringList::join(",").
    if (variants.size() == 1 && variants.constFirst().isEmpty()) {
        variants.clear();
    }
    if (displayNames.size() == 1 && displayNames.constFirst().isEmpty()) {
        displayNames.clear();
    }

    m_settings->setLayoutList(layoutStrings);
    m_settings->setVariantList(variants);
    m_settings->setDisplayNames(displayNames);
}

int UserLayoutModel::rowCount(const QModelIndex &parent) const
{
    return m_settings->configureLayouts() && !parent.isValid() ? m_layouts.size() : 0;
}

QVariant UserLayoutModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.row() >= rowCount(index.parent()))
        return QVariant();

    const auto &layout = m_layouts.at(index.row());

    if (role == Roles::LayoutRole) {
        return QVariant::fromValue(layout.layout());
    } else if (role == Roles::LayoutNameRole) {
        const std::optional<LayoutInfo> layoutInfo = Rules::self().getLayoutInfo(layout.layout());
        return layoutInfo ? layoutInfo->description : layout.layout();
    } else if (role == Roles::VariantRole) {
        return QVariant::fromValue(layout.variant());
    } else if (role == Roles::VariantNameRole) {
        if (layout.variant().isEmpty())
            return QString();

        const std::optional<LayoutInfo> layoutInfo = Rules::self().getLayoutInfo(layout.layout());
        if (!layoutInfo)
            return QString();

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

    if (index.row() >= rowCount(index.parent()) || index.data(role) == value) {
        return false;
    }

    LayoutUnit &layoutUnit = m_layouts[index.row()];

    if (role == Roles::DisplayNameRole) {
        const QString displayText = value.toString().left(3);
        if (layoutUnit.getDisplayName() != displayText) {
            layoutUnit.setDisplayName(displayText);
            applyToSettings();
            Q_EMIT dataChanged(index, index, {Roles::DisplayNameRole});
        }
        return true;
    }

    if (role == Roles::ShortcutRole) {
        QKeySequence shortcut(value.toString());
        if (layoutUnit.getShortcut() != shortcut) {
            layoutUnit.setShortcut(shortcut);
            Q_EMIT dataChanged(index, index, {Roles::ShortcutRole});
        }
        return true;
    }

    if (role == Roles::VariantRole) {
        QString variant = value.toString();
        if (layoutUnit.variant() != variant) {
            layoutUnit.setVariant(variant);
            applyToSettings();
            Q_EMIT dataChanged(index, index, {Roles::VariantRole, Roles::VariantNameRole});
        }
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

void UserLayoutModel::move(int oldIndex, int newIndex)
{
    if (beginMoveRows(QModelIndex(), oldIndex, oldIndex, QModelIndex(), oldIndex < newIndex ? newIndex + 1 : newIndex)) {
        m_layouts.move(oldIndex, newIndex);
        applyToSettings();
        endMoveRows();
    }
}

void UserLayoutModel::remove(int index)
{
    if (index >= rowCount()) {
        return;
    }
    beginRemoveRows(QModelIndex(), index, index);

    m_layouts.removeAt(index);
    applyToSettings();

    endRemoveRows();
}

static LayoutUnit makeLayout(const QString &layout, const QString &variant, const QKeySequence &shortcut, const QString &displayName)
{
    LayoutUnit unit(layout, variant);
    unit.setShortcut(shortcut);

    if (!displayName.isEmpty()) {
        unit.setDisplayName(displayName);
    }
    return unit;
}

void UserLayoutModel::addLayout(const QString &layout, const QString &variant, const QKeySequence &shortcut, const QString &displayName)
{
    bool exposingLayouts = m_settings->configureLayouts();
    if (exposingLayouts) {
        beginInsertRows(QModelIndex(), m_layouts.size(), m_layouts.size());
    }

    m_layouts.append(makeLayout(layout, variant, shortcut, displayName));
    applyToSettings();

    if (exposingLayouts) {
        endInsertRows();
    }
}

void UserLayoutModel::setSingleLayout(const QString &layout, const QString &variant, const QKeySequence &shortcut, const QString &displayName)
{
    reset({makeLayout(layout, variant, shortcut, displayName)});
}

LayoutUnit UserLayoutModel::layoutUnit(int row) const
{
    return row < rowCount() ? m_layouts.at(row) : LayoutUnit{QString()};
}

#include "moc_userlayoutmodel.cpp"
