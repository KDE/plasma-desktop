#ifndef __FONT_LIST_H__
#define __FONT_LIST_H__

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

#include <QUrl>
#include <KFileItem>
#include <KIO/Job>
#include <QList>
#include <QSet>
#include <QHash>
#include <QTreeView>
#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>
#include <QSortFilterProxyModel>
#include "Misc.h"
#include "JobRunner.h"
#include "FontFilter.h"
#include "FcQuery.h"
#include "File.h"
#include "Style.h"
#include "Family.h"
#include "FontInst.h"

class KFileItem;
class QMenu;
class QMimeData;
class QTimer;

#define KFI_FONT_DRAG_MIME "kfontinst/fontlist"

namespace KFI
{

class CFontItem;
class CFontItem;
class CFamilyItem;
class CGroupListItem;
class Style;

enum EColumns
{
    COL_FONT,
    COL_STATUS,

    NUM_COLS
};

typedef QList<CFamilyItem *> CFamilyItemCont;
typedef QList<CFontItem *>   CFontItemCont;
typedef QHash<QString, CFamilyItem *> CFamilyItemHash;

class CFontList : public QAbstractItemModel
{
    Q_OBJECT

    private:

    enum EMsgType
    {
        MSG_ADD,
        MSG_DEL,

        NUM_MSGS_TYPES
    };

    public:
    
    static const QStringList fontMimeTypes;

    public:

    static QStringList compact(const QStringList &fonts);

    CFontList(QWidget *parent=nullptr);
    ~CFontList() override;

    QVariant        data(const QModelIndex &index, int role) const override;
    Qt::ItemFlags   flags(const QModelIndex &index) const override;
    Qt::DropActions supportedDropActions() const override;
    QMimeData *     mimeData(const QModelIndexList &indexes) const override;
    QStringList     mimeTypes() const override;
    QVariant        headerData(int section, Qt::Orientation orientation,
                               int role = Qt::DisplayRole) const override;
    QModelIndex     index(int row, int column,
                          const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex     parent(const QModelIndex &index) const override;
    int             rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int             columnCount(const QModelIndex &parent = QModelIndex()) const override;
    int             row(const CFamilyItem *fam) const { return itsFamilies.indexOf((CFamilyItem *)fam); }
    void            forceNewPreviews();
    const CFamilyItemCont & families() const { return itsFamilies; }
    QModelIndex     createIndex(int row, int column, void *data = nullptr) const
                        { return QAbstractItemModel::createIndex(row, column, data); }
    bool            hasFamily(const QString &family)  { return nullptr!=findFamily(family); }
    void            refresh(bool allowSys, bool allowUser);
    bool            allowSys() const      { return itsAllowSys; }
    bool            allowUser() const     { return itsAllowUser; }
    void            getFamilyStats(QSet<QString> &enabled, QSet<QString> &disabled, QSet<QString> &partial);
    void            getFoundries(QSet<QString> &foundries) const;
    QString         whatsThis() const;
    void            setSlowUpdates(bool slow);
    bool            slowUpdates() const   { return itsSlowUpdates; }

    Q_SIGNALS:

    void            listingPercent(int p);

    public Q_SLOTS:

    void            unsetSlowUpdates();
    void            load();

    private Q_SLOTS:

    void            dbusServiceOwnerChanged(const QString &name, const QString &from, const QString &to);
    void            fontList(int pid, const QList<KFI::Families> &families);
    void            fontsAdded(const KFI::Families &families);
    void            fontsRemoved(const KFI::Families &families);

    private:

    void            storeSlowedMessage(const Families &families, EMsgType type);
    void            actionSlowedUpdates(bool sys);
    void            addFonts(const FamilyCont &families, bool sys);
    void            removeFonts(const FamilyCont &families, bool sys);
    CFamilyItem *   findFamily(const QString &familyName);
    
    private:
    
    CFamilyItemCont itsFamilies;
    CFamilyItemHash itsFamilyHash;
    bool            itsBlockSignals,
                    itsAllowSys,
                    itsAllowUser,
                    itsSlowUpdates;
    static int      theirPreviewSize;
    FamilyCont      itsSlowedMsgs[NUM_MSGS_TYPES][FontInst::FOLDER_COUNT];
};

class CFontModelItem
{
    public:

    CFontModelItem(CFontModelItem *p) : itsParent(p), itsIsSystem(false) { }
    virtual ~CFontModelItem()                                            { }

    CFontModelItem * parent() const                  { return itsParent; }
    bool             isFamily() const                { return nullptr==itsParent; }
    bool             isFont() const                  { return nullptr!=itsParent; }
    bool             isSystem() const                { return itsIsSystem; }
    void             setIsSystem(bool sys)           { itsIsSystem=sys; }
    virtual int      rowNumber() const = 0;

    protected:

    CFontModelItem *itsParent;
    bool           itsIsSystem;
};

class CFamilyItem : public CFontModelItem
{
    public:

    enum EStatus
    {
        ENABLED,
        PARTIAL,
        DISABLED
    };

    CFamilyItem(CFontList &p, const Family &f, bool sys);
    ~CFamilyItem() override;

    bool operator==(const CFamilyItem &other) const       { return itsName==other.itsName; }

    bool                 addFonts(const StyleCont &styles, bool sys);
    const QString &      name() const                     { return itsName; }
    const CFontItemCont & fonts() const                   { return itsFonts; }
    void                 addFont(CFontItem *font, bool update=true);
    void                 removeFont(CFontItem *font, bool update);
    void                 refresh();
    bool                 updateStatus();
    bool                 updateRegularFont(CFontItem *font);
    CFontItem *          findFont(quint32 style, bool sys);
    int                  rowNumber() const                override { return itsParent.row(this); }
    int                  row(const CFontItem *font) const { return itsFonts.indexOf((CFontItem *)font); }
    EStatus              status() const                   { return itsStatus; }
    EStatus              realStatus() const               { return itsRealStatus; }
    CFontItem *          regularFont()                    { return itsRegularFont; }
    int                  fontCount() const                { return itsFontCount; }
    void                 getFoundries(QSet<QString> &foundries) const;
    bool                 slowUpdates() const              { return itsParent.slowUpdates(); }

    private:

    bool                 usable(const CFontItem *font, bool root);

    private:

    QString       itsName;
    CFontItemCont itsFonts;
    int           itsFontCount;
    EStatus       itsStatus,
                  itsRealStatus;
    CFontItem     *itsRegularFont;  // 'RegularFont' is font nearest to 'Regular' style, and used for previews.
    CFontList     &itsParent;
};

class CFontItem : public CFontModelItem
{
    public:

    CFontItem(CFontModelItem *p, const Style &s, bool sys);
    ~CFontItem() override { }

    void                              refresh();
    QString                           name() const             { return family()+QString::fromLatin1(", ")+itsStyleName; }
    bool                              isEnabled() const        { return itsEnabled; }
    bool                              isHidden() const         { return !itsEnabled; }
    bool                              isBitmap() const         { return !itsStyle.scalable(); }
    const QString &                   fileName() const         { return (*itsStyle.files().begin()).path(); }
    const QString &                   style() const            { return itsStyleName; }
    quint32                           styleInfo() const        { return itsStyle.value(); }
    int                               index() const            { return (*itsStyle.files().begin()).index(); }
    const QString &                   family() const           { return (static_cast<CFamilyItem *>(parent()))->name(); }
    int                               rowNumber() const        override { return (static_cast<CFamilyItem *>(parent()))->row(this); }
    const FileCont &                  files() const            { return itsStyle.files(); }
    qulonglong                        writingSystems() const   { return itsStyle.writingSystems(); }
    QUrl                              url() const              { return CJobRunner::encode(family(), styleInfo(), isSystem()); }
    void                              removeFile(const File &f){ itsStyle.remove(f); }
    void                              removeFiles(const FileCont &f){ itsStyle.removeFiles(f); }
    void                              addFile(const File &f)   { itsStyle.add(f); }
    void                              addFiles(const FileCont &f)   { itsStyle.addFiles(f); }

    private:

    QString itsStyleName;
    Style   itsStyle;
    bool    itsEnabled;
};

class CFontListSortFilterProxy : public QSortFilterProxyModel
{
    Q_OBJECT

    public:

    CFontListSortFilterProxy(QObject *parent, QAbstractItemModel *model);
    ~CFontListSortFilterProxy() override { }

    QVariant         data(const QModelIndex &idx, int role) const override;
    bool             acceptFont(CFontItem *fnt, bool checkFontText) const;
    bool             acceptFamily(CFamilyItem *fam) const;
    bool             filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
    bool             lessThan(const QModelIndex &left, const QModelIndex &right) const override;
    void             setFilterGroup(CGroupListItem *grp);
    CGroupListItem * filterGroup()   { return itsGroup; }

    void             setFilterText(const QString &text);
    void             setFilterCriteria(CFontFilter::ECriteria crit, qulonglong ws, const QStringList &ft);

    private Q_SLOTS:

    void             timeout();
    void             fcResults();

    Q_SIGNALS:

    void             refresh();

    private:

    QString          filterText() const { return CFontFilter::CRIT_FONTCONFIG==itsFilterCriteria
                                                    ? (itsFcQuery ? itsFcQuery->font() : QString()) : itsFilterText; }
    private:

    CGroupListItem         *itsGroup;
    QString                itsFilterText;
    CFontFilter::ECriteria itsFilterCriteria;
    qulonglong             itsFilterWs;
    QStringList            itsFilterTypes;
    QTimer                 *itsTimer;
    CFcQuery               *itsFcQuery;
};

class CFontListView : public QTreeView
{
    Q_OBJECT

    public:

    CFontListView(QWidget *parent, CFontList *model);
    ~CFontListView() override { }

    void            getFonts(CJobRunner::ItemList &urls, QStringList &fontNames, QSet<Misc::TFont> *fonts,
                             bool selected, bool getEnabled=true, bool getDisabled=true);
    QSet<QString>   getFiles();
    void            getPrintableFonts(QSet<Misc::TFont> &items, bool selected);
    void            setFilterGroup(CGroupListItem *grp);
    void            stats(int &enabled, int &disabled, int &partial);
    void            selectedStatus(bool &enabled, bool &disabled);
    QModelIndexList allFonts();
    void            selectFirstFont();
    QModelIndexList getSelectedItems();

    Q_SIGNALS:

    void            del();
    void            print();
    void            enable();
    void            disable();
    void            fontsDropped(const QSet<QUrl> &);
    void            itemsSelected(const QModelIndexList &);
    void            refresh();
    void            reload();

    public Q_SLOTS:
        
    void            listingPercent(int percent);
    void            refreshFilter();
    void            filterText(const QString &text);
    void            filterCriteria(int crit, qulonglong ws, const QStringList &ft);

    private Q_SLOTS:

    void            setSortColumn(int col);
    void            selectionChanged(const QItemSelection &selected, const QItemSelection &deselected) override;
    void            itemCollapsed(const QModelIndex &index);
    void            view();

    private:

    QModelIndexList allIndexes();
    void            startDrag(Qt::DropActions supportedActions) override;
    void            dragEnterEvent(QDragEnterEvent *event) override;
    void            dropEvent(QDropEvent *event) override;
    void            contextMenuEvent(QContextMenuEvent *ev) override;
    bool    viewportEvent(QEvent *event) override;

    private:

    CFontListSortFilterProxy *itsProxy;
    CFontList                *itsModel;
    QMenu                    *itsMenu;
    QAction                  *itsDeleteAct,
                             *itsEnableAct,
                             *itsDisableAct,
                             *itsPrintAct,
                             *itsViewAct;
    bool                     itsAllowDrops;
};

}

#endif
