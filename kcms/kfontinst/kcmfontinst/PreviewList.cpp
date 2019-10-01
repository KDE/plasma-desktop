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

#include "PreviewList.h"
#include "FontList.h"
#include "Fc.h"
#include "FcEngine.h"
#include <QTextStream>
#include <QPainter>
#include <QStyledItemDelegate>
#include <QApplication>
#include <QHeaderView>
#include <QPixmapCache>
#include <QContextMenuEvent>
#include <QX11Info>

namespace KFI
{

static CFcEngine * theFcEngine= nullptr;

CPreviewList::CPreviewList(QObject *parent)
            : QAbstractItemModel(parent)
{
}

QVariant CPreviewList::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    CPreviewListItem *item=static_cast<CPreviewListItem *>(index.internalPointer());

    if(item)
        switch(role)
        {
            case Qt::DisplayRole:
                return FC::createName(item->name(), item->style());
            default:
                break;
        }
    return QVariant();
}

Qt::ItemFlags CPreviewList::flags(const QModelIndex &) const
{
    return Qt::ItemIsEnabled;
}

QModelIndex CPreviewList::index(int row, int column, const QModelIndex &parent) const
{
    if(!parent.isValid())
    {
        CPreviewListItem *item=itsItems.value(row);

        if(item)
            return createIndex(row, column, item);
    }

    return QModelIndex();
}

QModelIndex CPreviewList::parent(const QModelIndex &) const
{
    return QModelIndex();
}

void CPreviewList::clear()
{
    emit layoutAboutToBeChanged();
    qDeleteAll(itsItems);
    itsItems.clear();
    emit layoutChanged();
}

void CPreviewList::showFonts(const QModelIndexList &fonts)
{
    clear();
    emit layoutAboutToBeChanged();
    QModelIndex index;
    foreach(index, fonts)
    {
        CFontModelItem *mi=static_cast<CFontModelItem *>(index.internalPointer());
        CFontItem      *font=mi->parent()
                            ? static_cast<CFontItem *>(mi)
                            : (static_cast<CFamilyItem *>(mi))->regularFont();

        if(font)
            itsItems.append(new CPreviewListItem(font->family(), font->styleInfo(),
                                                 font->isEnabled() ? QString() : font->fileName(),
                                                 font->index()));
    }

    emit layoutChanged();
}

class CPreviewListViewDelegate : public QStyledItemDelegate
{
    public:

    CPreviewListViewDelegate(QObject *p, int previewSize) : QStyledItemDelegate(p), itsPreviewSize(previewSize) { }
    ~CPreviewListViewDelegate() override { }

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &idx) const override
    {
        CPreviewListItem     *item=static_cast<CPreviewListItem *>(idx.internalPointer());
        QStyleOptionViewItem opt(option);

        opt.rect.adjust(1, constBorder-3, 0, -(1+itsPreviewSize));

        QStyledItemDelegate::paint(painter, opt, idx);

        opt.rect.adjust(constBorder, option.rect.height()-(1+itsPreviewSize), -constBorder, 0);
        painter->save();
        painter->setPen(QApplication::palette().color(QPalette::Text));
        QRect lineRect(opt.rect.adjusted(-1, 3, 0, 2));
        painter->drawLine(lineRect.bottomLeft(), lineRect.bottomRight());
        painter->setClipRect(option.rect.adjusted(constBorder, 0, -constBorder, 0));
        painter->drawPixmap(opt.rect.topLeft(), getPixmap(item));
        painter->restore();
    }

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &idx) const override
    {
        QSize sz(QStyledItemDelegate::sizeHint(option, idx));
        //int   pWidth(getPixmap(static_cast<CPreviewListItem *>(idx.internalPointer())).width());
        int   pWidth(1536);

        return QSize((constBorder*2)+pWidth, sz.height()+1+constBorder+itsPreviewSize);
    }

    QPixmap getPixmap(CPreviewListItem *item) const
    {
        QString key;
        QPixmap pix;
        QColor  text(QApplication::palette().color(QPalette::Text));

        QTextStream(&key) << "kfi-" << item->name() << "-" << item->style() << "-" << text.rgba();

        if(!QPixmapCache::find(key, &pix))
        {
            QColor bgnd(Qt::black);

            bgnd.setAlpha(0);
            // TODO: Ideally, for this preview we want the fonts to be of a set point size
            pix=QPixmap::fromImage(theFcEngine->drawPreview(item->file().isEmpty() ? item->name() : item->file(),
                                                            item->style(),
                                                            item->index(),
                                                            text,
                                                            bgnd,
                                                            itsPreviewSize));
            QPixmapCache::insert(key, pix);
        }

        return pix;
    }

    int itsPreviewSize;
    static const int constBorder=4;
};

CPreviewListView::CPreviewListView(CFcEngine *eng, QWidget *parent)
                : QTreeView(parent)
{
    theFcEngine=eng;

    QFont font;
    int   pixelSize((int)(((font.pointSizeF()*QX11Info::appDpiY())/72.0)+0.5));

    itsModel=new CPreviewList(this);
    setModel(itsModel);
    setItemDelegate(new CPreviewListViewDelegate(this, (pixelSize+12)*3));
    setSelectionMode(NoSelection);
    setVerticalScrollMode(ScrollPerPixel);
    setSortingEnabled(false);
    setAlternatingRowColors(false);
    setAcceptDrops(false);
    setDragEnabled(false);
    header()->setVisible(false);
    setRootIsDecorated(false);
    resizeColumnToContents(0);
}

void CPreviewListView::refreshPreviews()
{
    QPixmapCache::clear();
    repaint();
    resizeColumnToContents(0);
}

void CPreviewListView::showFonts(const QModelIndexList &fonts)
{
    itsModel->showFonts(fonts);            
    resizeColumnToContents(0);
}

void CPreviewListView::contextMenuEvent(QContextMenuEvent *ev)
{
    emit showMenu(ev->pos());
}

}

