#ifndef __GROUP_LIST_H__
#define __GROUP_LIST_H__

/*
 * KFontInst - KDE Font Installer
 *
 * Copyright 2003-2007 Craig Drummond <craig@kde.org>
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

#include <KIO/Job>
#include <QList>
#include <QTreeView>
#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>
#include "FontList.h"

class QDragEnterEvent;
class QDragLeaveEvent;
class QDropEvent;
class QTextStream;
class QDomElement;

namespace KFI
{

class CGroupList;
class CFontItem;

class CGroupListItem
{
    public:

    enum EType
    {
        ALL,
        PERSONAL,
        SYSTEM,
        UNCLASSIFIED,
        CUSTOM
    };

    union Data
    {
        bool       validated;      //CUSTOM
        CGroupList *parent;        //UNCLASSIFIED
    };

    CGroupListItem(const QString &name);
    CGroupListItem(EType type, CGroupList *p);

    const QString &      name() const                        { return itsName; }
    void                 setName(const QString &n)           { itsName=n; }
    QSet<QString> &      families()                          { return itsFamilies; }
    EType                type() const                        { return itsType; }
    bool                 isCustom() const                    { return CUSTOM==itsType; }
    bool                 isAll() const                       { return ALL==itsType; }
    bool                 isUnclassified() const              { return UNCLASSIFIED==itsType; }
    bool                 isPersonal() const                  { return PERSONAL==itsType; }
    bool                 isSystem() const                    { return SYSTEM==itsType; }
    bool                 validated() const                   { return isCustom() ? itsData.validated : true; }
    void                 setValidated()                      { if(isCustom()) itsData.validated=true; }
    bool                 highlighted() const                 { return itsHighlighted; }
    void                 setHighlighted(bool b)              { itsHighlighted=b; }
    bool                 hasFont(const CFontItem *fnt) const;
    CFamilyItem::EStatus status() const                      { return itsStatus; }
    void                 updateStatus(QSet<QString> &enabled, QSet<QString> &disabled,
                                      QSet<QString> &partial);
    bool                 load(QDomElement &elem);
    bool                 addFamilies(QDomElement &elem);
    void                 save(QTextStream &str);
    void                 addFamily(const QString &family)    { itsFamilies.insert(family); }
    void                 removeFamily(const QString &family) { itsFamilies.remove(family); }
    bool                 hasFamily(const QString &family)    { return itsFamilies.contains(family); }

    private:

    QSet<QString>        itsFamilies;
    QString              itsName;
    EType                itsType;
    Data                 itsData;
    bool                 itsHighlighted;
    CFamilyItem::EStatus itsStatus;
};

class CGroupList : public QAbstractItemModel
{
    Q_OBJECT

    public:

    CGroupList(QWidget *parent = nullptr);
    ~CGroupList() override;

    QVariant        data(const QModelIndex &index, int role) const override;
    bool            setData(const QModelIndex &index, const QVariant &value, int role) override;
    Qt::ItemFlags   flags(const QModelIndex &index) const override;
    QVariant        headerData(int section, Qt::Orientation orientation,
                               int role = Qt::DisplayRole) const override;
    QModelIndex     index(int row, int column,
                          const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex     parent(const QModelIndex &index) const override;
    int             rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int             columnCount(const QModelIndex &parent = QModelIndex()) const override;
    void            update(const QModelIndex &unHighlight, const QModelIndex &highlight);
    void            updateStatus(QSet<QString> &enabled, QSet<QString> &disabled,
                                 QSet<QString> &partial);
    void            setSysMode(bool sys);
    void            rescan();
    void            load();
    bool            load(const QString &file);
    bool            save();
    bool            save(const QString &fileName, CGroupListItem *grp);
    void            merge(const QString &file);
    void            clear();
    QModelIndex     index(CGroupListItem::EType t);
    void            createGroup(const QString &name);
    bool            removeGroup(const QModelIndex &idx);
    void            removeFamily(const QString &family);
    bool            removeFromGroup(CGroupListItem *grp, const QString &family);
    QString         whatsThis() const;

    CGroupListItem * group(CGroupListItem::EType t)
                        { return itsSpecialGroups[t]; }
    bool            exists(const QString &name, bool showDialog=true);

    public Q_SLOTS:

    void            addToGroup(const QModelIndex &group, const QSet<QString> &families);
    void            removeFromGroup(const QModelIndex &group, const QSet<QString> &families);

    Q_SIGNALS:

    void            refresh();

    private:

    void            readGroupsFile();
    void            sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;
    Qt::DropActions supportedDropActions() const override;
    QStringList     mimeTypes() const override;
    CGroupListItem * find(const QString &name);
    QModelIndex     createIdx(int r, int c, void *p) { return createIndex(r, c, p); }

    private:

    QString                                       itsFileName;
    time_t                                        itsTimeStamp;
    bool                                          itsModified;
    QWidget                                       *itsParent;
    QList<CGroupListItem *>                       itsGroups;
    QMap<CGroupListItem::EType, CGroupListItem *> itsSpecialGroups;
    Qt::SortOrder                                 itsSortOrder;

    friend class CGroupListItem;
    friend class CGroupListView;
};

class CGroupListView : public QTreeView
{
    Q_OBJECT

    public:

    CGroupListView(QWidget *parent, CGroupList *model);
    ~CGroupListView() override              { }

    QSize                 sizeHint() const override { return QSize(32, 32); }

    bool                  isCustom()       { return CGroupListItem::CUSTOM==getType(); }
    bool                  isUnclassified() { return CGroupListItem::UNCLASSIFIED==getType(); }
    bool                  isSystem()       { return CGroupListItem::SYSTEM==getType(); }
    bool                  isPersonal()     { return CGroupListItem::PERSONAL==getType(); }
    CGroupListItem::EType getType();
    void                  controlMenu(bool del, bool en, bool dis, bool p, bool exp);

    Q_SIGNALS:

    void                  del();
    void                  print();
    void                  enable();
    void                  disable();
    void                  zip();
    void                  moveFonts();
    void                  info(const QString &str);
    void                  addFamilies(const QModelIndex &group, const QSet<QString> &);
    void                  removeFamilies(const QModelIndex &group, const QSet<QString> &);
    void                  itemSelected(const QModelIndex &);
    void                  unclassifiedChanged();

    private Q_SLOTS:

    void                  selectionChanged(const QItemSelection &selected,
                                           const QItemSelection &deselected) override;
    void                  rename();
    void                  emitMoveFonts();

    private:

    void                  contextMenuEvent(QContextMenuEvent *ev) override;
    void                  dragEnterEvent(QDragEnterEvent *event) override;
    void                  dragMoveEvent(QDragMoveEvent *event) override;
    void                  dragLeaveEvent(QDragLeaveEvent *event) override;
    void                  dropEvent(QDropEvent *event) override;
    void                  drawHighlighter(const QModelIndex &idx);
    bool          viewportEvent(QEvent *event) override;

    private:

    QMenu       *itsMenu;
    QAction     *itsDeleteAct,
                *itsEnableAct,
                *itsDisableAct,
                *itsPrintAct,
                *itsRenameAct,
                *itsExportAct;
    QModelIndex itsCurrentDropItem;
};
}

#endif
