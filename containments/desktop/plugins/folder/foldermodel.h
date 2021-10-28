/*
    SPDX-FileCopyrightText: 2008 Fredrik Höglund <fredrik@kde.org>
    SPDX-FileCopyrightText: 2011 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2014 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef FOLDERMODEL_H
#define FOLDERMODEL_H

#include <QImage>
#include <QItemSelection>
#include <QPointer>
#include <QQmlParserStatus>
#include <QRegExp>
#include <QSet>
#include <QSortFilterProxyModel>
#include <QStringList>

#include <KAbstractViewAdapter>
#include <KActionCollection>
#include <KDirLister>
#include <KFilePreviewGenerator>

#include <KNewFileMenu>

#include "folderplugin_private_export.h"

class QDrag;
class QItemSelectionModel;
class QQuickItem;

class KFileCopyToMenu;
class KActionCollection;
class KDirModel;
class KDirWatch;
class KFileItem;
class KFileItemActions;
class KJob;
class KNewFileMenu;

namespace KIO
{
class DropJob;
class StatJob;
}

class ScreenMapper;

class DirLister : public KDirLister
{
    Q_OBJECT

public:
    explicit DirLister(QObject *parent = nullptr);
    ~DirLister() override;

Q_SIGNALS:
    void error(const QString &string);

protected:
    void handleError(KIO::Job *job) override;
};

class FOLDERPLUGIN_TESTS_EXPORT FolderModel : public QSortFilterProxyModel, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)

    Q_PROPERTY(QString url READ url WRITE setUrl NOTIFY urlChanged)
    Q_PROPERTY(QString iconName READ iconName NOTIFY iconNameChanged)
    Q_PROPERTY(QUrl resolvedUrl READ resolvedUrl NOTIFY resolvedUrlChanged)
    Q_PROPERTY(Status status READ status NOTIFY statusChanged)
    Q_PROPERTY(QString errorString READ errorString NOTIFY errorStringChanged)
    Q_PROPERTY(bool dragging READ dragging NOTIFY draggingChanged)
    Q_PROPERTY(bool usedByContainment READ usedByContainment WRITE setUsedByContainment NOTIFY usedByContainmentChanged)
    Q_PROPERTY(bool locked READ locked WRITE setLocked NOTIFY lockedChanged)
    Q_PROPERTY(int sortMode READ sortMode WRITE setSortMode NOTIFY sortModeChanged)
    Q_PROPERTY(bool sortDesc READ sortDesc WRITE setSortDesc NOTIFY sortDescChanged)
    Q_PROPERTY(bool sortDirsFirst READ sortDirsFirst WRITE setSortDirsFirst NOTIFY sortDirsFirstChanged)
    Q_PROPERTY(bool parseDesktopFiles READ parseDesktopFiles WRITE setParseDesktopFiles NOTIFY parseDesktopFilesChanged)
    Q_PROPERTY(QObject *viewAdapter READ viewAdapter WRITE setViewAdapter NOTIFY viewAdapterChanged)
    Q_PROPERTY(bool previews READ previews WRITE setPreviews NOTIFY previewsChanged)
    Q_PROPERTY(QStringList previewPlugins READ previewPlugins WRITE setPreviewPlugins NOTIFY previewPluginsChanged)
    Q_PROPERTY(int filterMode READ filterMode WRITE setFilterMode NOTIFY filterModeChanged)
    Q_PROPERTY(QString filterPattern READ filterPattern WRITE setFilterPattern NOTIFY filterPatternChanged)
    Q_PROPERTY(QStringList filterMimeTypes READ filterMimeTypes WRITE setFilterMimeTypes NOTIFY filterMimeTypesChanged)
    Q_PROPERTY(QObject *newMenu READ newMenu CONSTANT)
    Q_PROPERTY(QObject *appletInterface READ appletInterface WRITE setAppletInterface NOTIFY appletInterfaceChanged)

public:
    enum DataRole {
        BlankRole = Qt::UserRole + 1,
        SelectedRole,
        IsDirRole,
        IsLinkRole,
        IsHiddenRole,
        UrlRole,
        LinkDestinationUrl,
        SizeRole,
        TypeRole,
        FileNameRole,
        FileNameWrappedRole,
    };

    enum FilterMode {
        NoFilter = 0,
        FilterShowMatches,
        FilterHideMatches,
    };

    enum Status {
        None,
        Ready,
        Listing,
        Canceled,
    };
    Q_ENUM(Status)

    explicit FolderModel(QObject *parent = nullptr);
    ~FolderModel() override;

    QHash<int, QByteArray> roleNames() const override;
    static QHash<int, QByteArray> staticRoleNames();

    void classBegin() override;
    void componentComplete() override;

    QString url() const;
    void setUrl(const QString &url);

    QString iconName() const;

    QUrl resolvedUrl() const;
    Q_INVOKABLE QUrl resolve(const QString &url);

    Status status() const;

    QString errorString() const;

    bool dragging() const;

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

    QObject *viewAdapter() const;
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

    QObject *appletInterface() const;
    void setAppletInterface(QObject *appletInterface);

    KFileItem rootItem() const;

    Q_INVOKABLE void up();
    Q_INVOKABLE void cd(int row);

    Q_INVOKABLE void run(int row);
    Q_INVOKABLE void runSelected();

    Q_INVOKABLE void rename(int row, const QString &name);
    Q_INVOKABLE int fileExtensionBoundary(int row);

    Q_INVOKABLE bool hasSelection() const;
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
    Q_INVOKABLE void drop(QQuickItem *target, QObject *dropEvent, int row, bool showMenuManually = false);
    Q_INVOKABLE void dropCwd(QObject *dropEvent);

    Q_INVOKABLE bool isBlank(int row) const;

    Q_INVOKABLE QAction *action(const QString &name) const;
    QObject *newMenu() const;
    Q_INVOKABLE void updateActions();
    Q_INVOKABLE void openContextMenu(QQuickItem *visualParent = nullptr, Qt::KeyboardModifiers modifiers = Qt::NoModifier);

    Q_INVOKABLE void linkHere(const QUrl &sourceUrl);

    Q_INVOKABLE void openPropertiesDialog();

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    int indexForUrl(const QUrl &url) const;
    KFileItem itemForIndex(const QModelIndex &index) const;
    bool isDir(const QModelIndex &index, const KDirModel *dirModel) const;
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;
    Qt::DropActions supportedDragActions() const override;
    Qt::DropActions supportedDropActions() const override;

    Q_INVOKABLE void paste();
    Q_INVOKABLE void copy();
    Q_INVOKABLE void cut();
    Q_INVOKABLE void deleteSelected();
    Q_INVOKABLE void openSelected();
    Q_INVOKABLE void undo();
    Q_INVOKABLE void refresh();
    Q_INVOKABLE void createFolder();

    void setScreen(int screen);

Q_SIGNALS:
    void urlChanged() const;
    void listingCompleted() const;
    void listingCanceled() const;
    void iconNameChanged() const;
    void resolvedUrlChanged() const;
    void statusChanged() const;
    void errorStringChanged() const;
    void draggingChanged() const;
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
    void screenChanged() const;
    void appletInterfaceChanged() const;
    void requestRename() const;
    void move(int x, int y, QList<QUrl> urls);
    void popupMenuAboutToShow(KIO::DropJob *dropJob, QMimeData *mimeData, int x, int y);

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
    bool matchMimeType(const KFileItem &item) const;
    bool matchPattern(const KFileItem &item) const;

private Q_SLOTS:
    void dragSelectedInternal(int x, int y);
    void dirListFailed(const QString &error);
    void statResult(KJob *job);
    void evictFromIsDirCache(const KFileItemList &items);
    void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
    void pasteTo();
    void moveSelectedToTrash();
    void emptyTrashBin();
    void restoreSelectedFromTrash();
    void undoTextChanged(const QString &text);
    void invalidateIfComplete();
    void invalidateFilterIfComplete();
    void newFileMenuItemCreated(const QUrl &url);

private:
    struct DragImage {
        int row;
        QRect rect;
        QPoint cursorOffset;
        QImage image;
        bool blank;
    };

    QPoint localMenuPosition() const;
    void createActions();
    void addDragImage(QDrag *drag, int x, int y);
    void setStatus(Status status);
    static bool isTrashEmpty();
    static bool isDeleteCommandShown();
    QList<QUrl> selectedUrls() const;
    KDirModel *m_dirModel;
    KDirWatch *m_dirWatch;
    QString m_url;
    mutable QHash<QUrl, bool> m_isDirCache;
    mutable QHash<QUrl, KIO::StatJob *> m_isDirJobs;
    QItemSelectionModel *m_selectionModel;
    QItemSelection m_pinnedSelection;
    QModelIndexList m_dragIndexes;
    QHash<int, DragImage *> m_dragImages;
    QPoint m_dragHotSpotScrollOffset;
    bool m_dragInProgress;
    bool m_urlChangedWhileDragging;
    // target filename to target position of a drop event, note that this deliberately
    // is not using the URL to easily support desktop:/ URL schemes
    QHash<QString, QPoint> m_dropTargetPositions;
    QTimer *m_dropTargetPositionsCleanup;
    QPointer<KFilePreviewGenerator> m_previewGenerator;
    QPointer<KAbstractViewAdapter> m_viewAdapter;
    KActionCollection m_actionCollection;
    KNewFileMenu *m_newMenu;
    KFileItemActions *m_fileItemActions;
    KFileCopyToMenu *m_copyToMenu;
    Status m_status = Status::None;
    QString m_errorString;
    bool m_usedByContainment;
    bool m_locked;
    int m_sortMode; // FIXME TODO: Enumify.
    bool m_sortDesc;
    bool m_sortDirsFirst;
    bool m_parseDesktopFiles;
    bool m_previews;
    // An empty previewPlugin list means use default.
    // We don't want to leak that fact to the QML side, however, so the property stays empty
    // and internally we operate on effectivePreviewPlugins instead.
    QStringList m_previewPlugins;
    QStringList m_effectivePreviewPlugins;
    FilterMode m_filterMode;
    QString m_filterPattern;
    bool m_filterPatternMatchAll;
    QSet<QString> m_mimeSet;
    QList<QRegExp> m_regExps;
    int m_screen = -1;
    bool m_screenUsed;
    ScreenMapper *m_screenMapper = nullptr;
    QObject *m_appletInterface = nullptr;
    bool m_complete;
    QPoint m_menuPosition;
};

#endif
