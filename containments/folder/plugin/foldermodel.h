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

class QItemSelectionModel;

class KActionCollection;
class KDirModel;
class KFileItem;
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
    Q_PROPERTY(QString errorString READ errorString NOTIFY errorStringChanged)
    Q_PROPERTY(bool usedByContainment READ usedByContainment WRITE setUsedByContainment NOTIFY usedByContainmentChanged);
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

    public:
        enum FilterMode {
            NoFilter = 0,
            FilterShowMatches,
            FilterHideMatches
        };

        FolderModel(QObject *parent = 0);
        ~FolderModel();

        QHash<int, QByteArray> roleNames() const;

        QString url() const;
        void setUrl(const QString &url);

        QString errorString() const;

        bool usedByContainment() const;
        void setUsedByContainment(bool used);

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

        Q_INVOKABLE void setSelected(int row);
        Q_INVOKABLE void setRangeSelected(int startRow, int endRow);
        Q_INVOKABLE void toggleSelected(int row);
        Q_INVOKABLE void clearSelection();

        Q_INVOKABLE void openContextMenu();

        QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
        KFileItem itemForIndex(const QModelIndex &index) const;
        bool isDir(const QModelIndex &index, const KDirModel *dirModel) const;
        bool lessThan(const QModelIndex &left, const QModelIndex &right) const;

    Q_SIGNALS:
        void urlChanged() const;
        void errorStringChanged() const;
        void usedByContainmentChanged() const;
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

    protected:
        bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;
        bool matchMimeType(const KFileItem &item) const;
        bool matchPattern(const KFileItem &item) const;

    private Q_SLOTS:
        void dirListFailed(const QString &error);
        void selectionChanged(QItemSelection selected, QItemSelection deselected);
        void aboutToShowCreateNew();
        void copy();
        void cut();
        void paste();
        void pasteTo();
        void refresh();
        void moveToTrash(Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers);
        void deleteSelected();
        void emptyTrashBin();

    private:
        void createActions();
        void updatePasteAction();
        QList<QUrl> selectedUrls(bool forTrash) const;
        KDirModel *m_dirModel;
        QItemSelectionModel *m_selectionModel;
        QPointer<KFilePreviewGenerator> m_previewGenerator;
        QPointer<KAbstractViewAdapter> m_viewAdapter;
        KActionCollection m_actionCollection;
        KNewFileMenu *m_newMenu;
        QString m_errorString;
        bool m_usedByContainment;
        int m_sortMode;
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
