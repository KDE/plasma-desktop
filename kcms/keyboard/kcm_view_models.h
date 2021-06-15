/*
    SPDX-FileCopyrightText: 2010 Andriy Rysin <rysin@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KCM_VIEW_MODELS_H_
#define KCM_VIEW_MODELS_H_

#include <QAbstractItemModel>
#include <QAbstractTableModel>
#include <QSet>
#include <QStyledItemDelegate>

class QTreeView;
class KeyboardConfig;
struct Rules;
class Flags;

class LayoutsTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    LayoutsTableModel(Rules *rules, Flags *flags, KeyboardConfig *keyboardConfig, QObject *parent = nullptr);

    int columnCount(const QModelIndex &) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
#ifdef DRAG_ENABLED
    Qt::DropActions supportedDropActions() const
    {
        return Qt::MoveAction;
    }
    QStringList mimeTypes() const;
    QMimeData *mimeData(const QModelIndexList &indexes) const;
#endif
    void refresh();

    static const int MAP_COLUMN;
    static const int LAYOUT_COLUMN;
    static const int VARIANT_COLUMN;
    static const int DISPLAY_NAME_COLUMN;
    static const int SHORTCUT_COLUMN;

private:
    KeyboardConfig *keyboardConfig;
    const Rules *rules;
    Flags *countryFlags;
};

class LabelEditDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit LabelEditDelegate(const KeyboardConfig *keyboardConfig, QObject *parent = nullptr);

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
    const KeyboardConfig *keyboardConfig;
};

class VariantComboDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    VariantComboDelegate(const KeyboardConfig *keyboardConfig, const Rules *rules, QObject *parent = nullptr);

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;

    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
    const KeyboardConfig *keyboardConfig;
    const Rules *rules;
};

class KKeySequenceWidgetDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    KKeySequenceWidgetDelegate(const KeyboardConfig *keyboardConfig_, QObject *parent = nullptr);

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
    const KeyboardConfig *keyboardConfig;
    mutable QSet<QModelIndex> itemsBeingEdited;
};

class XkbOptionsTreeModel : public QAbstractItemModel
{
public:
    XkbOptionsTreeModel(Rules *rules_, KeyboardConfig *keyboardConfig_, QObject *parent)
        : QAbstractItemModel(parent)
        , keyboardConfig(keyboardConfig_)
        , rules(rules_)
    {
    }

    int columnCount(const QModelIndex & /*parent*/) const override
    {
        return 1;
    }
    int rowCount(const QModelIndex &parent) const override;
    QModelIndex parent(const QModelIndex &index) const override
    {
        if (!index.isValid())
            return QModelIndex();
        if (index.internalId() < 100)
            return QModelIndex();
        return createIndex(((index.internalId() - index.row()) / 100) - 1, index.column());
    }
    QModelIndex index(int row, int column, const QModelIndex &parent) const override
    {
        if (!parent.isValid())
            return createIndex(row, column);
        return createIndex(row, column, (100 * (parent.row() + 1)) + row);
    }
    Qt::ItemFlags flags(const QModelIndex &index) const override
    {
        if (!index.isValid())
            return Qt::ItemFlags();

        if (!index.parent().isValid())
            return Qt::ItemIsEnabled | Qt::ItemIsSelectable;

        return Qt::ItemIsEnabled | Qt::ItemIsUserCheckable | Qt::ItemIsSelectable;
    }

    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    QVariant data(const QModelIndex &index, int role) const override;
    void reset()
    {
        beginResetModel();
        endResetModel();
    }
    void gotoGroup(const QString &group, QTreeView *view);

private:
    KeyboardConfig *keyboardConfig;
    Rules *rules;
};

#endif /* KCM_VIEW_MODELS_H_ */
