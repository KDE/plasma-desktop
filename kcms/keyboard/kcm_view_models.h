/*
 *  Copyright (C) 2010 Andriy Rysin (rysin@kde.org)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */


#ifndef KCM_VIEW_MODELS_H_
#define KCM_VIEW_MODELS_H_

#include <QAbstractItemModel>
#include <QAbstractTableModel>
#include <QSet>
#include <QStyledItemDelegate>

class QTreeView;
class KeyboardConfig;
class Rules;
class Flags;

class LayoutsTableModel : public QAbstractTableModel
{
     Q_OBJECT

 public:
     LayoutsTableModel(Rules* rules, Flags *flags, KeyboardConfig* keyboardConfig, QObject *parent = 0);

     int columnCount(const QModelIndex&) const;
     Qt::ItemFlags flags(const QModelIndex &index) const;
     QVariant headerData(int section, Qt::Orientation orientation,
                         int role = Qt::DisplayRole) const;

     int rowCount(const QModelIndex &parent = QModelIndex()) const;
     QVariant data(const QModelIndex &index, int role) const;
     bool setData(const QModelIndex &index, const QVariant &value, int role);
#ifdef DRAG_ENABLED
     Qt::DropActions supportedDropActions() const {
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
     KeyboardConfig* keyboardConfig;
     const Rules *rules;
     Flags *countryFlags;
};

class LabelEditDelegate : public QStyledItemDelegate
{
	Q_OBJECT

public:
	explicit LabelEditDelegate(const KeyboardConfig* keyboardConfig, QObject *parent = 0);

	QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
			const QModelIndex &index) const;

//    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;

private:
    const KeyboardConfig* keyboardConfig;
};

class VariantComboDelegate : public QStyledItemDelegate
{
	Q_OBJECT

public:
	VariantComboDelegate(const KeyboardConfig* keyboardConfig, const Rules* rules, QObject *parent = 0);

	QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
			const QModelIndex &index) const;

	void setEditorData(QWidget *editor, const QModelIndex &index) const;
	void setModelData(QWidget *editor, QAbstractItemModel *model,
			const QModelIndex &index) const;

	void updateEditorGeometry(QWidget *editor,
			const QStyleOptionViewItem &option, const QModelIndex &index) const;

private:
	const KeyboardConfig* keyboardConfig;
	const Rules* rules;
};

class KKeySequenceWidgetDelegate : public QStyledItemDelegate
{
	Q_OBJECT

public:
	KKeySequenceWidgetDelegate(const KeyboardConfig* keyboardConfig_, QObject *parent = 0);

	QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
			const QModelIndex &index) const;
//	void setEditorData(QWidget *editor, const QModelIndex &index) const;
	void setModelData(QWidget *editor, QAbstractItemModel *model,
			const QModelIndex &index) const;
	void paint(QPainter* painter, const QStyleOptionViewItem& option,
	                                    const QModelIndex& index) const;

private:
    const KeyboardConfig* keyboardConfig;
    mutable QSet<QModelIndex> itemsBeingEdited;
};


class XkbOptionsTreeModel: public QAbstractItemModel
{
public:
    XkbOptionsTreeModel(Rules* rules_, KeyboardConfig* keyboardConfig_, QObject *parent)
		: QAbstractItemModel(parent),
		  keyboardConfig(keyboardConfig_),
		  rules(rules_) { }

    int columnCount(const QModelIndex& /*parent*/) const { return 1; }
    int rowCount(const QModelIndex& parent) const;
    QModelIndex parent(const QModelIndex& index) const {
        if (!index.isValid() )
            return QModelIndex();
        if( index.internalId() < 100 )
            return QModelIndex();
        return createIndex(((index.internalId() - index.row())/100) - 1, index.column());
    }
    QModelIndex index(int row, int column, const QModelIndex& parent) const {
        if(!parent.isValid()) return createIndex(row, column);
        return createIndex(row, column, (100 * (parent.row()+1)) + row);
    }
    Qt::ItemFlags flags ( const QModelIndex & index ) const {
        if( ! index.isValid() )
            return 0;

        if( !index.parent().isValid() )
            return Qt::ItemIsEnabled;

        return Qt::ItemIsEnabled | Qt::ItemIsUserCheckable;
    }

    bool setData ( const QModelIndex & index, const QVariant & value, int role = Qt::EditRole );
    QVariant data(const QModelIndex& index, int role) const;
    void reset() { QAbstractItemModel::reset(); }
    void gotoGroup(const QString& group, QTreeView* view);

private:
    KeyboardConfig* keyboardConfig;
    Rules *rules;
};

#endif /* KCM_VIEW_MODELS_H_ */
