#ifndef __PREVIEW_LIST_H__
#define __PREVIEW_LIST_H__

/*
 * KFontInst - KDE Font Installer
 *
 * Copyright 2009 Craig Drummond <craig@kde.org>
 *
 * ----
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include <QAbstractItemModel>
#include <QTreeView>

class QContextMenuEvent;

namespace KFI
{

class CFcEngine;

class CPreviewListItem
{
    public:

    CPreviewListItem(const QString &name, quint32 style, const QString &file, int index)
        : itsName(name), itsFile(file), itsStyle(style), itsIndex(index) { }

    const QString & name() const  { return itsName; }
    quint32         style() const { return itsStyle; }
    const QString & file() const  { return itsFile; }
    int             index() const { return itsIndex; }

    private:

    QString itsName,
            itsFile;
    quint32 itsStyle;
    int     itsIndex;
};

class CPreviewList : public QAbstractItemModel
{
    Q_OBJECT

    public:

    CPreviewList(QObject *parent = nullptr);
    ~CPreviewList() override { clear(); }

    QVariant        data(const QModelIndex &index, int role) const override;
    Qt::ItemFlags   flags(const QModelIndex &index) const override;
    QModelIndex     index(int row, int column,
                          const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex     parent(const QModelIndex &index) const override;
    int             rowCount(const QModelIndex &parent = QModelIndex()) const override { Q_UNUSED(parent) return itsItems.count(); }
    int             columnCount(const QModelIndex &parent = QModelIndex()) const override { Q_UNUSED(parent) return 1; }
    void            clear();
    void            showFonts(const QModelIndexList &font);

    private:

    QList<CPreviewListItem *> itsItems;
};

class CPreviewListView : public QTreeView
{
    Q_OBJECT

    public:

    CPreviewListView(CFcEngine *eng, QWidget *parent);
    ~CPreviewListView() override { }

    void refreshPreviews();
    void showFonts(const QModelIndexList &fonts);
    void contextMenuEvent(QContextMenuEvent *ev) override;

    Q_SIGNALS:

    void showMenu(const QPoint &pos);

    private:

    CPreviewList *itsModel;
};

}

#endif
