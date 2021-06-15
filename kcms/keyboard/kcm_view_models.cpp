/*
    SPDX-FileCopyrightText: 2010 Andriy Rysin <rysin@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kcm_view_models.h"

#include <KKeySequenceWidget>
#include <KLocalizedString>

#include <QComboBox>
#include <QKeySequence>
#include <QLineEdit>
#include <QPainter>
#include <QTreeView>

#ifdef DRAG_ENABLED
#include <QMimeData>
#endif

#include "bindings.h"
#include "flags.h"
#include "keyboard_config.h"
#include "x11_helper.h"
#include "xkb_rules.h"

const int LayoutsTableModel::MAP_COLUMN = 0;
const int LayoutsTableModel::LAYOUT_COLUMN = 1;
const int LayoutsTableModel::VARIANT_COLUMN = 2;
const int LayoutsTableModel::DISPLAY_NAME_COLUMN = 3;
const int LayoutsTableModel::SHORTCUT_COLUMN = 4;
static const int COLUMN_COUNT = 5;

LayoutsTableModel::LayoutsTableModel(Rules *rules_, Flags *flags_, KeyboardConfig *keyboardConfig_, QObject *parent)
    : QAbstractTableModel(parent)
    , keyboardConfig(keyboardConfig_)
    , rules(rules_)
    , countryFlags(flags_)
{
}

void LayoutsTableModel::refresh()
{
    beginResetModel();
    endResetModel();
}

int LayoutsTableModel::rowCount(const QModelIndex & /*parent*/) const
{
    return keyboardConfig->layouts.count();
}

int LayoutsTableModel::columnCount(const QModelIndex &) const
{
    return COLUMN_COUNT;
}

Qt::ItemFlags LayoutsTableModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::ItemFlags();

    Qt::ItemFlags flags = QAbstractTableModel::flags(index);

    if (index.column() == DISPLAY_NAME_COLUMN || index.column() == VARIANT_COLUMN || index.column() == SHORTCUT_COLUMN) {
        flags |= Qt::ItemIsEditable;
    }

#ifdef DRAG_ENABLED
    flags |= Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
#endif

    return flags;
}

#ifdef DRAG_ENABLED
QStringList LayoutsTableModel::mimeTypes() const
{
    QStringList types;
    types << "application/keyboard-layout-item";
    return types;
}

QMimeData *LayoutsTableModel::mimeData(const QModelIndexList &indexes) const
{
    QMimeData *mimeData = new QMimeData();
    QByteArray encodedData;

    QDataStream stream(&encodedData, QIODevice::WriteOnly);

    QSet<int> rows;
    foreach (const QModelIndex &index, indexes) {
        if (index.isValid()) {
            rows << index.row();
        }
    }
    foreach (int row, rows) {
        stream << row;
    }

    mimeData->setData("application/keyboard-layout-item", encodedData);
    return mimeData;
}
#endif

QVariant LayoutsTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.row() >= keyboardConfig->layouts.size())
        return QVariant();

    const LayoutUnit &layoutUnit = keyboardConfig->layouts.at(index.row());

    if (role == Qt::DecorationRole) {
        switch (index.column()) {
        case DISPLAY_NAME_COLUMN: {
            // if(keyboardConfig->isFlagShown()) {
            return countryFlags->getIcon(layoutUnit.layout());
            // }
        }
        // TODO: show the cells are editable
        //    	 case VARIANT_COLUMN: {
        //    	 case DISPLAY_NAME_COLUMN: {
        //    		 int sz = 5;
        //    		 QPixmap pm = QPixmap(sz, sz+5);
        //    		 pm.fill(Qt::transparent);
        //    		 QPainter p(&pm);
        //    		 QPoint points[] = { QPoint(0, 0), QPoint(0, sz), QPoint(sz, 0) };
        //    		 p.drawPolygon(points, 3);
        //    		 return pm;
        //    	 }
        break;
        }
    } else if (role == Qt::BackgroundRole) {
        if (keyboardConfig->layoutLoopCount != KeyboardConfig::NO_LOOPING && index.row() >= keyboardConfig->layoutLoopCount) {
            return QBrush(Qt::lightGray);
        }
    } else if (role == Qt::DisplayRole) {
        switch (index.column()) {
        case MAP_COLUMN:
            return layoutUnit.layout();
            break;
        case LAYOUT_COLUMN: {
            const LayoutInfo *layoutInfo = rules->getLayoutInfo(layoutUnit.layout());
            return layoutInfo != nullptr ? layoutInfo->description : layoutUnit.layout();
        }
        case VARIANT_COLUMN: {
            if (layoutUnit.variant().isEmpty())
                return QVariant();
            const LayoutInfo *layoutInfo = rules->getLayoutInfo(layoutUnit.layout());
            if (layoutInfo == nullptr)
                return QVariant();
            const VariantInfo *variantInfo = layoutInfo->getVariantInfo(layoutUnit.variant());
            return variantInfo != nullptr ? variantInfo->description : layoutUnit.variant();
        } break;
        case DISPLAY_NAME_COLUMN:
            // 			if( keyboardConfig->indicatorType == KeyboardConfig::SHOW_LABEL ) {
            // 				return layoutUnit.getDisplayName();
            // 			}
            break;
        case SHORTCUT_COLUMN: {
            return layoutUnit.getShortcut().toString();
        } break;
        }
    } else if (role == Qt::EditRole) {
        switch (index.column()) {
        case DISPLAY_NAME_COLUMN:
            return layoutUnit.getDisplayName();
            break;
        case VARIANT_COLUMN:
            return layoutUnit.variant();
            break;
        case SHORTCUT_COLUMN:
            return layoutUnit.getShortcut().toString();
            break;
        default:;
        }
    } else if (role == Qt::TextAlignmentRole) {
        switch (index.column()) {
        case MAP_COLUMN:
        case DISPLAY_NAME_COLUMN:
        case SHORTCUT_COLUMN:
            return Qt::AlignCenter;
            break;
        default:;
        }
    }
    return QVariant();
}

QVariant LayoutsTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal) {
        const QString headers[] = {i18nc("layout map name", "Map"), i18n("Layout"), i18n("Variant"), i18n("Label"), i18n("Shortcut")};
        return headers[section];
    }

    return QVariant();
}

bool LayoutsTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role != Qt::EditRole || (index.column() != DISPLAY_NAME_COLUMN && index.column() != VARIANT_COLUMN && index.column() != SHORTCUT_COLUMN))
        return false;

    if (index.row() >= keyboardConfig->layouts.size() || index.data(role) == value)
        return false;

    LayoutUnit &layoutUnit = keyboardConfig->layouts[index.row()];

    switch (index.column()) {
    case DISPLAY_NAME_COLUMN: {
        QString displayText = value.toString().left(3);
        layoutUnit.setDisplayName(displayText);
    } break;
    case VARIANT_COLUMN: {
        layoutUnit.setVariant(value.toString());
    } break;
    case SHORTCUT_COLUMN: {
        layoutUnit.setShortcut(QKeySequence(value.toString()));
    } break;
    }
    Q_EMIT dataChanged(index, index);

    return true;
}

//
// LabelEditDelegate
//
LabelEditDelegate::LabelEditDelegate(const KeyboardConfig *keyboardConfig_, QObject *parent)
    : QStyledItemDelegate(parent)
    , keyboardConfig(keyboardConfig_)
{
}

QWidget *LabelEditDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QWidget *widget = QStyledItemDelegate::createEditor(parent, option, index);
    QLineEdit *lineEdit = static_cast<QLineEdit *>(widget);
    if (lineEdit != nullptr) {
        lineEdit->setMaxLength(LayoutUnit::MAX_LABEL_LENGTH);
        connect(lineEdit, &QLineEdit::textEdited, this, [this, lineEdit]() {
            Q_EMIT const_cast<LabelEditDelegate *>(this)->commitData(lineEdit);
        });
    }
    return widget;
}

void LabelEditDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex & /* index */) const
{
    editor->setGeometry(option.rect);
}

//
// VariantComboDelegate
//
// TODO: reuse this function in kcm_add_layout_dialog.cpp
static void populateComboWithVariants(QComboBox *combo, const QString &layout, const Rules *rules)
{
    combo->clear();
    const LayoutInfo *layoutInfo = rules->getLayoutInfo(layout);
    foreach (const VariantInfo *variantInfo, layoutInfo->variantInfos) {
        combo->addItem(variantInfo->description, variantInfo->name);
    }
    combo->model()->sort(0);
    combo->insertItem(0, i18nc("variant", "Default"), "");
    combo->setCurrentIndex(0);
}

VariantComboDelegate::VariantComboDelegate(const KeyboardConfig *keyboardConfig_, const Rules *rules_, QObject *parent)
    : QStyledItemDelegate(parent)
    , keyboardConfig(keyboardConfig_)
    , rules(rules_)
{
}

QWidget *VariantComboDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem & /* option */, const QModelIndex &index) const
{
    QComboBox *editor = new QComboBox(parent);
    const LayoutUnit &layoutUnit = keyboardConfig->layouts[index.row()];
    populateComboWithVariants(editor, layoutUnit.layout(), rules);
    connect(editor, &QComboBox::currentTextChanged, this, [this, editor]() {
        Q_EMIT const_cast<VariantComboDelegate *>(this)->commitData(editor);
    });
    return editor;
}

void VariantComboDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QComboBox *combo = static_cast<QComboBox *>(editor);
    QString variant = index.model()->data(index, Qt::EditRole).toString();
    int itemIndex = combo->findData(variant);
    if (itemIndex == -1) {
        itemIndex = 0;
    }
    combo->setCurrentIndex(itemIndex);
}

void VariantComboDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    QComboBox *combo = static_cast<QComboBox *>(editor);
    QString variant = combo->itemData(combo->currentIndex()).toString();
    model->setData(index, variant, Qt::EditRole);
}

void VariantComboDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex & /* index */) const
{
    editor->setGeometry(option.rect);
}

//
// KKeySequenceWidgetDelegate
//
KKeySequenceWidgetDelegate::KKeySequenceWidgetDelegate(const KeyboardConfig *keyboardConfig_, QObject *parent)
    : QStyledItemDelegate(parent)
    , keyboardConfig(keyboardConfig_)
{
}

QWidget *KKeySequenceWidgetDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem & /*option*/, const QModelIndex &index) const
{
    itemsBeingEdited.insert(index);

    KKeySequenceWidget *editor = new KKeySequenceWidget(parent);
    editor->setFocusPolicy(Qt::StrongFocus);
    editor->setModifierlessAllowed(false);

    const LayoutUnit &layoutUnit = keyboardConfig->layouts[index.row()];
    editor->setKeySequence(layoutUnit.getShortcut());

    editor->captureKeySequence();
    connect(editor, &KKeySequenceWidget::keySequenceChanged, this, [this, editor]() {
        Q_EMIT const_cast<KKeySequenceWidgetDelegate *>(this)->commitData(editor);
    });

    return editor;
}

void KKeySequenceWidgetDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    KKeySequenceWidget *kkeysequencewidget = static_cast<KKeySequenceWidget *>(editor);
    QString shortcut = kkeysequencewidget->keySequence().toString();
    model->setData(index, shortcut, Qt::EditRole);
    itemsBeingEdited.remove(index);
}

void KKeySequenceWidgetDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (itemsBeingEdited.contains(index)) {
        //      StyledBackgroundPainter::drawBackground(painter,option,index);
    } else {
        QStyledItemDelegate::paint(painter, option, index);
    }
}
//
// Xkb Options Tree View
//

int XkbOptionsTreeModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid())
        return rules->optionGroupInfos.count();
    if (!parent.parent().isValid())
        return rules->optionGroupInfos[parent.row()]->optionInfos.count();
    return 0;
}

QVariant XkbOptionsTreeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    int row = index.row();

    if (role == Qt::DisplayRole) {
        if (!index.parent().isValid()) {
            return rules->optionGroupInfos[row]->description;
        } else {
            int groupRow = index.parent().row();
            const OptionGroupInfo *xkbGroup = rules->optionGroupInfos[groupRow];
            return xkbGroup->optionInfos[row]->description;
        }
    } else if (role == Qt::CheckStateRole) {
        if (index.parent().isValid()) {
            int groupRow = index.parent().row();
            const OptionGroupInfo *xkbGroup = rules->optionGroupInfos[groupRow];
            const QString &xkbOptionName = xkbGroup->optionInfos[row]->name;
            return keyboardConfig->xkbOptions.indexOf(xkbOptionName) == -1 ? Qt::Unchecked : Qt::Checked;
        } else {
            int groupRow = index.row();
            const OptionGroupInfo *xkbGroup = rules->optionGroupInfos[groupRow];
            foreach (const OptionInfo *optionInfo, xkbGroup->optionInfos) {
                if (keyboardConfig->xkbOptions.indexOf(optionInfo->name) != -1)
                    return Qt::PartiallyChecked;
            }
            return Qt::Unchecked;
        }
    }
    return QVariant();
}

bool XkbOptionsTreeModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    int groupRow = index.parent().row();
    if (groupRow < 0)
        return false;

    const OptionGroupInfo *xkbGroup = rules->optionGroupInfos[groupRow];
    const OptionInfo *option = xkbGroup->optionInfos[index.row()];

    if (value.toInt() == Qt::Checked) {
        if (xkbGroup->exclusive) {
            // clear if exclusive (TODO: radiobutton)
            int idx = keyboardConfig->xkbOptions.indexOf(QRegExp(xkbGroup->name + ".*"));
            if (idx >= 0) {
                for (int i = 0; i < xkbGroup->optionInfos.count(); i++)
                    if (xkbGroup->optionInfos[i]->name == keyboardConfig->xkbOptions[idx]) {
                        setData(createIndex(i, index.column(), static_cast<quint32>(index.internalId()) - index.row() + i), Qt::Unchecked, role);
                        break;
                    }
                //    m_kxkbConfig->m_options.removeAt(idx);
                //    idx = m_kxkbConfig->m_options.indexOf(QRegExp(xkbGroupNm+".*"));
            }
        }
        if (keyboardConfig->xkbOptions.indexOf(option->name) < 0) {
            keyboardConfig->xkbOptions.append(option->name);
        }
    } else {
        keyboardConfig->xkbOptions.removeAll(option->name);
    }

    Q_EMIT dataChanged(index, index);
    Q_EMIT dataChanged(index.parent(), index.parent());
    return true;
}

void XkbOptionsTreeModel::gotoGroup(const QString &groupName, QTreeView *view)
{
    const OptionGroupInfo *optionGroupInfo = rules->getOptionGroupInfo(groupName);
    int index = rules->optionGroupInfos.indexOf(const_cast<OptionGroupInfo *>(optionGroupInfo));
    if (index != -1) {
        QModelIndex modelIdx = createIndex(index, 0);
        //            view->selectionModel()->setCurrentIndex(createIndex(index,0), QItemSelectionModel::NoUpdate);
        view->setExpanded(modelIdx, true);
        view->scrollTo(modelIdx, QAbstractItemView::PositionAtTop);
        view->selectionModel()->setCurrentIndex(modelIdx, QItemSelectionModel::Current);
        view->setFocus(Qt::OtherFocusReason);
    }
}
