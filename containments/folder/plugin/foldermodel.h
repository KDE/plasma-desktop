/***************************************************************************
 *   Copyright (C) 2008 Fredrik Höglund <fredrik@kde.org>                  *
 *   Copyright (C) 2011 Marco Martin <mart@kde.org>                        *
 *   Copyright (C) 2014 by Eike Hein <hein@kde.org>                        *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#ifndef FOLDERMODEL_H
#define FOLDERMODEL_H

#include <QImage>
#include <QItemSelection>
#include <QPointer>
#include <QSortFilterProxyModel>
#include <QStringList>
#include <QSet>
#include <QRegExp>

#include <KAbstractViewAdapter>
#include <KActionCollection>
#include <KFilePreviewGenerator>
#include <KDirLister>

#include <KNewFileMenu>

class QDrag;
class QItemSelectionModel;
class QQuickItem;

class KActionCollection;
class KDirModel;
class KFileItem;
class KFileItemActions;
class KJob;
class KNewFileMenu;

class DirLister : public KDirLister
{
    Q_OBJECT

    public:
        DirLister(QObject *parent = 0);
        ~DirLister();

    Q_SIGNALS:
        void error(const QString &string);

    protected:
        void handleError(KIO::Job *job);
};

class FolderModel : public QSortFilterProxyModel
{
    Q_OBJECT

    Q_PROPERTY(QString url READ url WRITE setUrl NOTIFY urlChanged)
    Q_PROPERTY(QUrl resolvedUrl READ resolvedUrl NOTIFY resolvedUrlChanged)
    Q_PROPERTY(QString errorString READ errorString NOTIFY errorStringChanged)
    Q_PROPERTY(bool usedByContainment READ usedByContainment WRITE setUsedByContainment NOTIFY usedByContainmentChanged);
    Q_PROPERTY(bool locked READ locked WRITE setLocked NOTIFY lockedChanged)
    Q_PROPERTY(int sortMode READ sortMode WRITE setSortMode NOTIFY sortModeChanged)
    Q_PROPERTY(bool sortDesc READ sortDesc WRITE setSortDesc NOTIFY sortDescChanged)
    Q_PROPERTY(bool sortDirsFirst READ sortDirsFirst WRITE setSortDirsFirst NOTIFY sortDirsFirstChanged)
    Q_PROPERTY(bool parseDesktopFiles READ parseDesktopFiles WRITE setParseDesktopFiles NOTIFY parseDesktopFilesChanged)
    Q_PROPERTY(QObject* viewAdapter READ viewAdapter WRITE setViewAdapter NOTIFY viewAdapterChanged)
    Q_PROPERTY(bool previews READ previews WRITE setPreviews NOTIFY previewsChanged)
    Q_PROPERTY(QStringList previewPlugins READ previewPlugins WRITE setPreviewPlugins NOTIFY previewPluginsChanged)
    Q_PROPERTY(int filterMode READ filterMode WRITE setFilterMode NOTIFY filterModeChanged)
    Q_PROPERTY(QString filterPattern READ filterPattern WRITE setFilterPattern NOTIFY filterPatternChanged)
    Q_PROPERTY(QStringList filterMimeTypes READ filterMimeTypes WRITE setFilterMimeTypes NOTIFY filterMimeTypesChanged)
    Q_PROPERTY(QObject* newMenu READ newMenu CONSTANT)

    public:
        enum DataRole {
            BlankRole = Qt::UserRole + 1,
            SelectedRole,
            IsDirRole,
            UrlRole,
            LinkDestinationUrl,
            SizeRole,
            TypeRole,
            FileNameRole
        };

        enum FilterMode {
            NoFilter = 0,
            FilterShowMatches,
            FilterHideMatches
        };

        FolderModel(QObject *parent = 0);
        ~FolderModel();

        QHash<int, QByteArray> roleNames() const;
        static QHash<int, QByteArray> staticRoleNames();

        QString url() const;
        void setUrl(const QString &url);

        QUrl resolvedUrl() const;

        QString errorString() const;

        bool usedByContainment() const;
        void setUsedByContainment(bool used);

        bool locked() const;
        void setLocked(bool locked);

        int sortMode() const;
        void setSortMode(int mode);

        bool sortDesc() const;
        void setSortDesc(bool desc);

        bool sortDirsFirst() const;
        void setSortDirsFirst(bool enable);

        bool parseDesktopFiles() const;
        void setParseDesktopFiles(bool enable);

        QObject* viewAdapter() const;
        void setViewAdapter(QObject *adapter);

        bool previews() const;
        void setPreviews(bool previews);

        QStringList previewPlugins() const;
        void setPreviewPlugins(const QStringList &previewPlugins);

        int filterMode() const;
        void setFilterMode(int filterMode);

        QString filterPattern() const;
        void setFilterPattern(const QString &pattern);

        QStringList filterMimeTypes() const;
        void setFilterMimeTypes(const QStringList &mimeList);

        Q_INVOKABLE void run(int row);

        Q_INVOKABLE void rename(int row, const QString& name);

        Q_INVOKABLE bool hasSelection();
        Q_INVOKABLE bool isSelected(int row);
        Q_INVOKABLE void setSelected(int row);
        Q_INVOKABLE void toggleSelected(int row);
        Q_INVOKABLE void setRangeSelected(int anchor, int to);
        Q_INVOKABLE void updateSelection(const QVariantList &rows, bool toggle);
        Q_INVOKABLE void clearSelection();
        Q_INVOKABLE void pinSelection();
        Q_INVOKABLE void unpinSelection();

        Q_INVOKABLE void addItemDragImage(int row, int x, int y, int width, int height, const QVariant &image);
        Q_INVOKABLE void clearDragImages();
        Q_INVOKABLE void setDragHotSpotScrollOffset(int x, int y); // FIXME TODO: Propify.
        Q_INVOKABLE QPoint dragCursorOffset(int row);
        Q_INVOKABLE void dragSelected(int x, int y);
        Q_INVOKABLE void drop(QQuickItem *target, QObject *dropEvent, int row);

        Q_INVOKABLE bool isBlank(int row) const;

        Q_INVOKABLE QAction* action(const QString& name) const;
        QObject* newMenu() const;
        Q_INVOKABLE void updateActions();
        Q_INVOKABLE void openContextMenu();

        Q_INVOKABLE void linkHere(const QUrl &sourceUrl);

        QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
        int indexForUrl(const QUrl &url) const;
        KFileItem itemForIndex(const QModelIndex &index) const;
        bool isDir(const QModelIndex &index, const KDirModel *dirModel) const;
        bool lessThan(const QModelIndex &left, const QModelIndex &right) const;

    Q_SIGNALS:
        void urlChanged() const;
        void resolvedUrlChanged() const;
        void errorStringChanged() const;
        void usedByContainmentChanged() const;
        void lockedChanged() const;
        void sortModeChanged() const;
        void sortDescChanged() const;
        void sortDirsFirstChanged() const;
        void parseDesktopFilesChanged() const;
        void viewAdapterChanged();
        void previewsChanged() const;
        void previewPluginsChanged() const;
        void filterModeChanged() const;
        void filterPatternChanged() const;
        void filterMimeTypesChanged() const;
        void requestRename() const;
        void move(int x, int y, QList<QUrl> urls);

    protected:
        bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;
        bool matchMimeType(const KFileItem &item) const;
        bool matchPattern(const KFileItem &item) const;

    private Q_SLOTS:
        void dirListFailed(const QString &error);
        void statResult(KJob *job);
        void evictFromIsDirCache(const KFileItemList &items);
        void selectionChanged(QItemSelection selected, QItemSelection deselected);
        void copy();
        void cut();
        void paste();
        void pasteTo();
        void refresh();
        void moveSelectedToTrash();
        void deleteSelected();
        void emptyTrashBin();
        void undoTextChanged(const QString &text);

    private:
        struct DragImage {
            int row;
            QRect rect;
            QPoint cursorOffset;
            QImage image;
            bool blank;
        };

        void createActions();
        void updatePasteAction();
        void addDragImage(QDrag *drag, int x, int y);
        QList<QUrl> selectedUrls(bool forTrash) const;
        KDirModel *m_dirModel;
        QString m_url;
        QHash<QUrl, bool> m_isDirCache;
        QItemSelectionModel *m_selectionModel;
        QItemSelection m_pinnedSelection;
        QModelIndexList m_dragIndexes;
        QHash<int, DragImage *> m_dragImages;
        QPoint m_dragHotSpotScrollOffset;
        bool m_dragInProgress;
        QPointer<KFilePreviewGenerator> m_previewGenerator;
        QPointer<KAbstractViewAdapter> m_viewAdapter;
        KActionCollection m_actionCollection;
        KNewFileMenu *m_newMenu;
        KFileItemActions *m_fileItemActions;
        QString m_errorString;
        bool m_usedByContainment;
        bool m_locked;
        int m_sortMode; // FIXME TODO: Enumify.
        bool m_sortDesc;
        bool m_sortDirsFirst;
        bool m_parseDesktopFiles;
        bool m_previews;
        QStringList m_previewPlugins;
        FilterMode m_filterMode;
        QString m_filterPattern;
        bool m_filterPatternMatchAll;
        QSet<QString> m_mimeSet;
        QList<QRegExp> m_regExps;
};

#endif
