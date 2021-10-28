/*
    SPDX-FileCopyrightText: 2006 David Faure <faure@kde.org>
    SPDX-FileCopyrightText: 2008 Fredrik Höglund <fredrik@kde.org>
    SPDX-FileCopyrightText: 2008 Rafael Fernández López <ereslibre@kde.org>
    SPDX-FileCopyrightText: 2011 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2014 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "foldermodel.h"
#include "itemviewadapter.h"
#include "positioner.h"
#include "removeaction.h"
#include "screenmapper.h"

#include <QApplication>
#include <QClipboard>
#include <QCollator>
#include <QDesktopWidget>
#include <QDrag>
#include <QImage>
#include <QItemSelectionModel>
#include <QLoggingCategory>
#include <QMenu>
#include <QMimeData>
#include <QMimeDatabase>
#include <QPainter>
#include <QPixmap>
#include <QQuickItem>
#include <QQuickWindow>
#include <QScreen>
#include <QTimer>
#include <qplatformdefs.h>

#include <KAuthorized>
#include <KConfigGroup>
#include <KDirWatch>
#include <KFileCopyToMenu>
#include <KFileItemActions>
#include <KFileItemListProperties>
#include <KIO/DeleteJob>
#include <KIO/DropJob>
#include <KIO/EmptyTrashJob>
#include <KIO/FileUndoManager>
#include <KIO/JobUiDelegate>
#include <KIO/Paste>
#include <KIO/PasteJob>
#include <KIO/RestoreJob>
#include <KLocalizedString>
#include <KPropertiesDialog>
#include <KSharedConfig>
#include <KShell>

#include <KCoreDirLister>
#include <KDesktopFile>
#include <KDirModel>
#include <KIO/CopyJob>
#include <KIO/Job>
#include <KIO/PreviewJob>
#include <KProtocolInfo>
#include <KRun>
#include <KStringHandler>

#include <Plasma/Applet>
#include <Plasma/Containment>
#include <Plasma/Corona>

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

Q_LOGGING_CATEGORY(FOLDERMODEL, "plasma.containments.desktop.folder.foldermodel")

DirLister::DirLister(QObject *parent)
    : KDirLister(parent)
{
}

DirLister::~DirLister()
{
}

void DirLister::handleError(KIO::Job *job)
{
    if (!autoErrorHandlingEnabled()) {
        Q_EMIT error(job->errorString());
        return;
    }

    KDirLister::handleError(job);
}

FolderModel::FolderModel(QObject *parent)
    : QSortFilterProxyModel(parent)
    , m_dirWatch(nullptr)
    , m_dragInProgress(false)
    , m_urlChangedWhileDragging(false)
    , m_dropTargetPositionsCleanup(new QTimer(this))
    , m_previewGenerator(nullptr)
    , m_viewAdapter(nullptr)
    , m_actionCollection(this)
    , m_newMenu(nullptr)
    , m_fileItemActions(nullptr)
    , m_usedByContainment(false)
    , m_locked(true)
    , m_sortMode(0)
    , m_sortDesc(false)
    , m_sortDirsFirst(true)
    , m_parseDesktopFiles(false)
    , m_previews(false)
    , m_filterMode(NoFilter)
    , m_filterPatternMatchAll(true)
    , m_screenUsed(false)
    , m_screenMapper(ScreenMapper::instance())
    , m_complete(false)
{
    // needed to pass the job around with qml
    qmlRegisterType<KIO::DropJob>();
    DirLister *dirLister = new DirLister(this);
    dirLister->setDelayedMimeTypes(true);
    dirLister->setAutoErrorHandlingEnabled(false, nullptr);
    connect(dirLister, &DirLister::error, this, &FolderModel::dirListFailed);
    connect(dirLister, &KCoreDirLister::itemsDeleted, this, &FolderModel::evictFromIsDirCache);

    connect(dirLister, &KCoreDirLister::started, this, std::bind(&FolderModel::setStatus, this, Status::Listing));

    void (KCoreDirLister::*myCompletedSignal)() = &KCoreDirLister::completed;
    QObject::connect(dirLister, myCompletedSignal, this, [this] {
        setStatus(Status::Ready);
        Q_EMIT listingCompleted();
    });

    void (KCoreDirLister::*myCanceledSignal)() = &KCoreDirLister::canceled;
    QObject::connect(dirLister, myCanceledSignal, this, [this] {
        setStatus(Status::Canceled);
        Q_EMIT listingCanceled();
    });

    m_dirModel = new KDirModel(this);
    m_dirModel->setDirLister(dirLister);
    m_dirModel->setDropsAllowed(KDirModel::DropOnDirectory | KDirModel::DropOnLocalExecutable);

    // If we have dropped items queued for moving, go unsorted now.
    connect(this, &QAbstractItemModel::rowsAboutToBeInserted, this, [this]() {
        if (!m_dropTargetPositions.isEmpty()) {
            setSortMode(-1);
        }
    });

    // Position dropped items at the desired target position.
    connect(this, &QAbstractItemModel::rowsInserted, this, [this](const QModelIndex &parent, int first, int last) {
        for (int i = first; i <= last; ++i) {
            const auto idx = index(i, 0, parent);
            const auto url = itemForIndex(idx).url();
            auto it = m_dropTargetPositions.find(url.fileName());
            if (it != m_dropTargetPositions.end()) {
                const auto pos = it.value();
                m_dropTargetPositions.erase(it);
                Q_EMIT move(pos.x(), pos.y(), {url});
            }
        }
    });

    /*
     * Dropped files may not actually show up as new files, e.g. when we overwrite
     * an existing file. Or files that fail to be listed by the dirLister, or...
     * To ensure we don't grow the map indefinitely, clean it up periodically.
     * The cleanup timer is (re)started whenever we modify the map. We use a quite
     * high interval of 10s. This should ensure, that we don't accidentally wipe
     * the mapping when we actually still want to use it. Since the time between
     * adding an entry in the map and it showing up in the model should be
     * small, this should rarely, if ever happen.
     */
    m_dropTargetPositionsCleanup->setInterval(10000);
    m_dropTargetPositionsCleanup->setSingleShot(true);
    connect(m_dropTargetPositionsCleanup, &QTimer::timeout, this, [this]() {
        if (!m_dropTargetPositions.isEmpty()) {
            qCDebug(FOLDERMODEL) << "clearing drop target positions after timeout:" << m_dropTargetPositions;
            m_dropTargetPositions.clear();
        }
    });

    m_selectionModel = new QItemSelectionModel(this, this);
    connect(m_selectionModel, &QItemSelectionModel::selectionChanged, this, &FolderModel::selectionChanged);

    setSourceModel(m_dirModel);

    setSortLocaleAware(true);
    setFilterCaseSensitivity(Qt::CaseInsensitive);
    setDynamicSortFilter(true);

    sort(m_sortMode, m_sortDesc ? Qt::DescendingOrder : Qt::AscendingOrder);

    createActions();
}

FolderModel::~FolderModel()
{
    if (m_usedByContainment) {
        // disconnect so we don't handle signals from the screen mapper when
        // removeScreen is called
        m_screenMapper->disconnect(this);
        m_screenMapper->removeScreen(m_screen, resolvedUrl());
    }
}

QHash<int, QByteArray> FolderModel::roleNames() const
{
    return staticRoleNames();
}

QHash<int, QByteArray> FolderModel::staticRoleNames()
{
    QHash<int, QByteArray> roleNames;
    roleNames[Qt::DisplayRole] = "display";
    roleNames[Qt::DecorationRole] = "decoration";
    roleNames[BlankRole] = "blank";
    roleNames[SelectedRole] = "selected";
    roleNames[IsDirRole] = "isDir";
    roleNames[IsLinkRole] = "isLink";
    roleNames[IsHiddenRole] = "isHidden";
    roleNames[UrlRole] = "url";
    roleNames[LinkDestinationUrl] = "linkDestinationUrl";
    roleNames[SizeRole] = "size";
    roleNames[TypeRole] = "type";
    roleNames[FileNameWrappedRole] = "displayWrapped";

    return roleNames;
}

QPoint FolderModel::localMenuPosition() const
{
    QScreen *screen = nullptr;
    for (auto *s : qApp->screens()) {
        if (s->geometry().contains(m_menuPosition)) {
            screen = s;
            break;
        }
    }
    if (screen) {
        return m_menuPosition - screen->geometry().topLeft();
    }
    return m_menuPosition;
}

void FolderModel::classBegin()
{
}

void FolderModel::componentComplete()
{
    m_complete = true;
    invalidate();
}

void FolderModel::invalidateIfComplete()
{
    if (!m_complete) {
        return;
    }

    invalidate();
}

void FolderModel::invalidateFilterIfComplete()
{
    if (!m_complete) {
        return;
    }

    invalidateFilter();
}

void FolderModel::newFileMenuItemCreated(const QUrl &url)
{
    if (m_usedByContainment && !m_screenMapper->sharedDesktops()) {
        m_screenMapper->addMapping(url, m_screen, ScreenMapper::DelayedSignal);
        m_dropTargetPositions.insert(url.fileName(), localMenuPosition());
        m_menuPosition = {};
        m_dropTargetPositionsCleanup->start();
    }
}

QString FolderModel::url() const
{
    return m_url;
}

void FolderModel::setUrl(const QString &url)
{
    const QUrl &resolvedNewUrl = resolve(url);

    if (url == m_url) {
        m_dirModel->dirLister()->updateDirectory(resolvedNewUrl);
        return;
    }

    const auto oldUrl = resolvedUrl();

    beginResetModel();
    m_url = url;
    m_isDirCache.clear();
    m_dirModel->dirLister()->openUrl(resolvedNewUrl);
    clearDragImages();
    m_dragIndexes.clear();
    endResetModel();

    Q_EMIT urlChanged();
    Q_EMIT resolvedUrlChanged();

    m_errorString.clear();
    Q_EMIT errorStringChanged();

    if (m_dirWatch) {
        delete m_dirWatch;
        m_dirWatch = nullptr;
    }

    if (resolvedNewUrl.isValid()) {
        m_dirWatch = new KDirWatch(this);
        connect(m_dirWatch, &KDirWatch::created, this, &FolderModel::iconNameChanged);
        connect(m_dirWatch, &KDirWatch::dirty, this, &FolderModel::iconNameChanged);
        m_dirWatch->addFile(resolvedNewUrl.toLocalFile() + QLatin1String("/.directory"));
    }

    if (m_dragInProgress) {
        m_urlChangedWhileDragging = true;
    }

    Q_EMIT iconNameChanged();

    if (m_usedByContainment && !m_screenMapper->sharedDesktops()) {
        m_screenMapper->removeScreen(m_screen, oldUrl);
        m_screenMapper->addScreen(m_screen, resolvedUrl());
    }
}

QUrl FolderModel::resolvedUrl() const
{
    return m_dirModel->dirLister()->url();
}

QUrl FolderModel::resolve(const QString &url)
{
    QUrl resolvedUrl;

    if (url.startsWith(QLatin1Char('~'))) {
        resolvedUrl = QUrl::fromLocalFile(KShell::tildeExpand(url));
    } else {
        resolvedUrl = QUrl::fromUserInput(url);
    }

    return resolvedUrl;
}

QString FolderModel::iconName() const
{
    const KFileItem rootItem(m_dirModel->dirLister()->url());

    if (!rootItem.isFinalIconKnown()) {
        rootItem.determineMimeType();
    }

    return rootItem.iconName();
}

FolderModel::Status FolderModel::status() const
{
    return m_status;
}

void FolderModel::setStatus(Status status)
{
    if (m_status != status) {
        m_status = status;
        Q_EMIT statusChanged();
    }
}

QString FolderModel::errorString() const
{
    return m_errorString;
}

bool FolderModel::dragging() const
{
    return m_dragInProgress;
}

bool FolderModel::usedByContainment() const
{
    return m_usedByContainment;
}

void FolderModel::setUsedByContainment(bool used)
{
    if (m_usedByContainment != used) {
        m_usedByContainment = used;

        QAction *action = m_actionCollection.action(QStringLiteral("refresh"));

        if (action) {
            action->setText(m_usedByContainment ? i18n("&Refresh Desktop") : i18n("&Refresh View"));
            action->setIcon(m_usedByContainment ? QIcon::fromTheme(QStringLiteral("user-desktop")) : QIcon::fromTheme(QStringLiteral("view-refresh")));
        }

        m_screenMapper->disconnect(this);
        connect(m_screenMapper, &ScreenMapper::screensChanged, this, &FolderModel::invalidateFilterIfComplete);
        connect(m_screenMapper, &ScreenMapper::screenMappingChanged, this, &FolderModel::invalidateFilterIfComplete);

        Q_EMIT usedByContainmentChanged();
    }
}

bool FolderModel::locked() const
{
    return m_locked;
}

void FolderModel::setLocked(bool locked)
{
    if (m_locked != locked) {
        m_locked = locked;

        Q_EMIT lockedChanged();
    }
}

void FolderModel::dirListFailed(const QString &error)
{
    m_errorString = error;
    Q_EMIT errorStringChanged();
}

int FolderModel::sortMode() const
{
    return m_sortMode;
}

void FolderModel::setSortMode(int mode)
{
    if (m_sortMode != mode) {
        m_sortMode = mode;

        if (mode == -1 /* Unsorted */) {
            setDynamicSortFilter(false);
        } else {
            invalidateIfComplete();
            sort(m_sortMode, m_sortDesc ? Qt::DescendingOrder : Qt::AscendingOrder);
            setDynamicSortFilter(true);
        }

        Q_EMIT sortModeChanged();
    }
}

bool FolderModel::sortDesc() const
{
    return m_sortDesc;
}

void FolderModel::setSortDesc(bool desc)
{
    if (m_sortDesc != desc) {
        m_sortDesc = desc;

        if (m_sortMode != -1 /* Unsorted */) {
            invalidateIfComplete();
            sort(m_sortMode, m_sortDesc ? Qt::DescendingOrder : Qt::AscendingOrder);
        }

        Q_EMIT sortDescChanged();
    }
}

bool FolderModel::sortDirsFirst() const
{
    return m_sortDirsFirst;
}

void FolderModel::setSortDirsFirst(bool enable)
{
    if (m_sortDirsFirst != enable) {
        m_sortDirsFirst = enable;

        if (m_sortMode != -1 /* Unsorted */) {
            invalidateIfComplete();
            sort(m_sortMode, m_sortDesc ? Qt::DescendingOrder : Qt::AscendingOrder);
        }

        Q_EMIT sortDirsFirstChanged();
    }
}

bool FolderModel::parseDesktopFiles() const
{
    return m_parseDesktopFiles;
}

void FolderModel::setParseDesktopFiles(bool enable)
{
    if (m_parseDesktopFiles != enable) {
        m_parseDesktopFiles = enable;
        Q_EMIT parseDesktopFilesChanged();
    }
}

QObject *FolderModel::viewAdapter() const
{
    return m_viewAdapter;
}

void FolderModel::setViewAdapter(QObject *adapter)
{
    if (m_viewAdapter != adapter) {
        KAbstractViewAdapter *abstractViewAdapter = dynamic_cast<KAbstractViewAdapter *>(adapter);

        m_viewAdapter = abstractViewAdapter;

        if (m_viewAdapter && !m_previewGenerator) {
            m_previewGenerator = new KFilePreviewGenerator(abstractViewAdapter, this);
            m_previewGenerator->setPreviewShown(m_previews);
            m_previewGenerator->setEnabledPlugins(m_effectivePreviewPlugins);
        }

        Q_EMIT viewAdapterChanged();
    }
}

bool FolderModel::previews() const
{
    return m_previews;
}

void FolderModel::setPreviews(bool previews)
{
    if (m_previews != previews) {
        m_previews = previews;

        if (m_previewGenerator) {
            m_previewGenerator->setPreviewShown(m_previews);
        }

        Q_EMIT previewsChanged();
    }
}

QStringList FolderModel::previewPlugins() const
{
    return m_previewPlugins;
}

void FolderModel::setPreviewPlugins(const QStringList &previewPlugins)
{
    QStringList effectivePlugins = previewPlugins;
    if (effectivePlugins.isEmpty()) {
        effectivePlugins = KIO::PreviewJob::defaultPlugins();
    }

    if (m_effectivePreviewPlugins != effectivePlugins) {
        m_effectivePreviewPlugins = effectivePlugins;

        if (m_previewGenerator) {
            m_previewGenerator->setPreviewShown(false);
            m_previewGenerator->setEnabledPlugins(m_effectivePreviewPlugins);
            m_previewGenerator->setPreviewShown(true);
        }
    }

    if (m_previewPlugins != previewPlugins) {
        m_previewPlugins = previewPlugins;
        Q_EMIT previewPluginsChanged();
    }
}

int FolderModel::filterMode() const
{
    return m_filterMode;
}

void FolderModel::setFilterMode(int filterMode)
{
    if (m_filterMode != (FilterMode)filterMode) {
        m_filterMode = (FilterMode)filterMode;

        invalidateFilterIfComplete();

        Q_EMIT filterModeChanged();
    }
}

QString FolderModel::filterPattern() const
{
    return m_filterPattern;
}

void FolderModel::setFilterPattern(const QString &pattern)
{
    if (m_filterPattern == pattern) {
        return;
    }

    m_filterPattern = pattern;
    m_filterPatternMatchAll = (pattern == QLatin1String("*"));

    const QStringList patterns = pattern.split(QLatin1Char(' '));
    m_regExps.clear();
    m_regExps.reserve(patterns.count());

    foreach (const QString &pattern, patterns) {
        QRegExp rx(pattern);
        rx.setPatternSyntax(QRegExp::Wildcard);
        rx.setCaseSensitivity(Qt::CaseInsensitive);
        m_regExps.append(rx);
    }

    invalidateFilterIfComplete();

    Q_EMIT filterPatternChanged();
}

QStringList FolderModel::filterMimeTypes() const
{
    return m_mimeSet.values();
}

void FolderModel::setFilterMimeTypes(const QStringList &mimeList)
{
    const QSet<QString> set(mimeList.constBegin(), mimeList.constEnd());

    if (m_mimeSet != set) {
        m_mimeSet = set;

        invalidateFilterIfComplete();

        Q_EMIT filterMimeTypesChanged();
    }
}

void FolderModel::setScreen(int screen)
{
    m_screenUsed = (screen != -1);

    if (!m_screenUsed || m_screen == screen)
        return;

    m_screen = screen;
    if (m_usedByContainment && !m_screenMapper->sharedDesktops()) {
        m_screenMapper->addScreen(screen, resolvedUrl());
    }
    Q_EMIT screenChanged();
}

KFileItem FolderModel::rootItem() const
{
    return m_dirModel->dirLister()->rootItem();
}

void FolderModel::up()
{
    const QUrl &up = KIO::upUrl(resolvedUrl());

    if (up.isValid()) {
        setUrl(up.toString());
    }
}

void FolderModel::cd(int row)
{
    if (row < 0) {
        return;
    }

    const QModelIndex idx = index(row, 0);
    bool isDir = data(idx, IsDirRole).toBool();

    if (isDir) {
        const KFileItem item = itemForIndex(idx);
        if (m_parseDesktopFiles && item.isDesktopFile()) {
            const KDesktopFile file(item.targetUrl().path());
            if (file.hasLinkType()) {
                setUrl(file.readUrl());
            }
        } else {
            setUrl(item.targetUrl().toString());
        }
    }
}

void FolderModel::run(int row)
{
    if (row < 0) {
        return;
    }

    KFileItem item = itemForIndex(index(row, 0));

    QUrl url(item.targetUrl());

    // FIXME TODO: This can go once we depend on a KIO w/ fe1f50caaf2.
    if (url.scheme().isEmpty()) {
        url.setScheme(QStringLiteral("file"));
    }

    KRun *run = new KRun(url, nullptr);
    // On desktop:/ we want to be able to run .desktop files right away,
    // otherwise ask for security reasons. We also don't use the targetUrl()
    // from above since we don't want the resolved /home/foo/Desktop URL.
    run->setShowScriptExecutionPrompt(item.url().scheme() != QLatin1String("desktop")
                                      || item.url().adjusted(QUrl::RemoveFilename).path() != QLatin1String("/")
                                      || !item.isDesktopFile());
}

void FolderModel::runSelected()
{
    if (!m_selectionModel->hasSelection()) {
        return;
    }

    if (m_selectionModel->selectedIndexes().count() == 1) {
        run(m_selectionModel->selectedIndexes().constFirst().row());
        return;
    }

    KFileItemActions fileItemActions(this);
    KFileItemList items;

    foreach (const QModelIndex &index, m_selectionModel->selectedIndexes()) {
        // Skip over directories.
        if (!index.data(IsDirRole).toBool()) {
            items << itemForIndex(index);
        }
    }

    fileItemActions.runPreferredApplications(items, QString());
}

void FolderModel::rename(int row, const QString &name)
{
    if (row < 0) {
        return;
    }

    QModelIndex idx = index(row, 0);
    m_dirModel->setData(mapToSource(idx), name, Qt::EditRole);
}

int FolderModel::fileExtensionBoundary(int row)
{
    const QModelIndex idx = index(row, 0);
    const QString &name = data(idx, Qt::DisplayRole).toString();

    int boundary = name.length();

    if (data(idx, IsDirRole).toBool()) {
        return boundary;
    }

    QMimeDatabase db;
    const QString &ext = db.suffixForFileName(name);

    if (ext.isEmpty()) {
        boundary = name.lastIndexOf(QLatin1Char('.'));

        if (boundary < 1) {
            boundary = name.length();
        }
    } else {
        boundary -= ext.length() + 1;
    }

    return boundary;
}

bool FolderModel::hasSelection() const
{
    return m_selectionModel->hasSelection();
}

bool FolderModel::isSelected(int row)
{
    if (row < 0) {
        return false;
    }

    return m_selectionModel->isSelected(index(row, 0));
}

void FolderModel::setSelected(int row)
{
    if (row < 0) {
        return;
    }

    m_selectionModel->select(index(row, 0), QItemSelectionModel::Select);
}

void FolderModel::toggleSelected(int row)
{
    if (row < 0) {
        return;
    }

    m_selectionModel->select(index(row, 0), QItemSelectionModel::Toggle);
}

void FolderModel::setRangeSelected(int anchor, int to)
{
    if (anchor < 0 || to < 0) {
        return;
    }

    QItemSelection selection(index(anchor, 0), index(to, 0));
    m_selectionModel->select(selection, QItemSelectionModel::ClearAndSelect);
}

void FolderModel::updateSelection(const QVariantList &rows, bool toggle)
{
    QItemSelection newSelection;

    int iRow = -1;

    foreach (const QVariant &row, rows) {
        iRow = row.toInt();

        if (iRow < 0) {
            return;
        }

        const QModelIndex &idx = index(iRow, 0);
        newSelection.select(idx, idx);
    }

    if (toggle) {
        QItemSelection pinnedSelection = m_pinnedSelection;
        pinnedSelection.merge(newSelection, QItemSelectionModel::Toggle);
        m_selectionModel->select(pinnedSelection, QItemSelectionModel::ClearAndSelect);
    } else {
        m_selectionModel->select(newSelection, QItemSelectionModel::ClearAndSelect);
    }
}

void FolderModel::clearSelection()
{
    if (m_selectionModel->hasSelection()) {
        m_selectionModel->clear();
    }
}

void FolderModel::pinSelection()
{
    m_pinnedSelection = m_selectionModel->selection();
}

void FolderModel::unpinSelection()
{
    m_pinnedSelection = QItemSelection();
}

void FolderModel::addItemDragImage(int row, int x, int y, int width, int height, const QVariant &image)
{
    if (row < 0) {
        return;
    }

    delete m_dragImages.take(row);

    DragImage *dragImage = new DragImage();
    dragImage->row = row;
    dragImage->rect = QRect(x, y, width, height);
    dragImage->image = image.value<QImage>();
    dragImage->blank = false;

    m_dragImages.insert(row, dragImage);
}

void FolderModel::clearDragImages()
{
    qDeleteAll(m_dragImages);
    m_dragImages.clear();
}

void FolderModel::setDragHotSpotScrollOffset(int x, int y)
{
    m_dragHotSpotScrollOffset.setX(x);
    m_dragHotSpotScrollOffset.setY(y);
}

QPoint FolderModel::dragCursorOffset(int row)
{
    DragImage *image = m_dragImages.value(row);
    if (!image) {
        return QPoint(0, 0);
    }

    return image->cursorOffset;
}

void FolderModel::addDragImage(QDrag *drag, int x, int y)
{
    if (!drag || m_dragImages.isEmpty()) {
        return;
    }

    QRegion region;

    foreach (DragImage *image, m_dragImages) {
        image->blank = isBlank(image->row);
        image->rect.translate(-m_dragHotSpotScrollOffset.x(), -m_dragHotSpotScrollOffset.y());
        if (!image->blank && !image->image.isNull()) {
            region = region.united(image->rect);
        }
    }

    QRect rect = region.boundingRect();
    QPoint offset = rect.topLeft();
    rect.translate(-offset.x(), -offset.y());

    QImage dragImage(rect.size(), QImage::Format_RGBA8888);
    dragImage.fill(Qt::transparent);

    QPainter painter(&dragImage);

    QPoint pos;

    foreach (DragImage *image, m_dragImages) {
        if (!image->blank && !image->image.isNull()) {
            pos = image->rect.translated(-offset.x(), -offset.y()).topLeft();
            image->cursorOffset.setX(pos.x() - (x - offset.x()));
            image->cursorOffset.setY(pos.y() - (y - offset.y()));

            painter.drawImage(pos, image->image);
        }

        // FIXME HACK: Operate on copy.
        image->rect.translate(m_dragHotSpotScrollOffset.x(), m_dragHotSpotScrollOffset.y());
    }

    drag->setPixmap(QPixmap::fromImage(dragImage));
    drag->setHotSpot(QPoint(x - offset.x(), y - offset.y()));
}

void FolderModel::dragSelected(int x, int y)
{
    if (m_dragInProgress) {
        return;
    }

    m_dragInProgress = true;
    Q_EMIT draggingChanged();
    m_urlChangedWhileDragging = false;

    // Avoid starting a drag synchronously in a mouse handler or interferes with
    // child event filtering in parent items (and thus e.g. press-and-hold hand-
    // ling in a containment).
    QMetaObject::invokeMethod(this, "dragSelectedInternal", Qt::QueuedConnection, Q_ARG(int, x), Q_ARG(int, y));
}

void FolderModel::dragSelectedInternal(int x, int y)
{
    if (!m_viewAdapter || !m_selectionModel->hasSelection()) {
        m_dragInProgress = false;
        Q_EMIT draggingChanged();
        return;
    }

    ItemViewAdapter *adapter = qobject_cast<ItemViewAdapter *>(m_viewAdapter);
    QQuickItem *item = qobject_cast<QQuickItem *>(adapter->adapterView());

    QDrag *drag = new QDrag(item);

    addDragImage(drag, x, y);

    m_dragIndexes = m_selectionModel->selectedIndexes();

    std::sort(m_dragIndexes.begin(), m_dragIndexes.end());

    // TODO: Optimize to Q_EMIT contiguous groups.
    Q_EMIT dataChanged(m_dragIndexes.first(), m_dragIndexes.last(), QVector<int>() << BlankRole);

    QModelIndexList sourceDragIndexes;
    sourceDragIndexes.reserve(m_dragIndexes.count());
    foreach (const QModelIndex &index, m_dragIndexes) {
        sourceDragIndexes.append(mapToSource(index));
    }

    drag->setMimeData(m_dirModel->mimeData(sourceDragIndexes));

    // Due to spring-loading (aka auto-expand), the URL might change
    // while the drag is in-flight - in that case we don't want to
    // unnecessarily Q_EMIT dataChanged() for (possibly invalid) indices
    // after it ends.
    const QUrl currentUrl(m_dirModel->dirLister()->url());

    item->grabMouse();
    drag->exec(supportedDragActions());

    item->ungrabMouse();

    m_dragInProgress = false;
    Q_EMIT draggingChanged();
    m_urlChangedWhileDragging = false;

    if (m_dirModel->dirLister()->url() == currentUrl) {
        const QModelIndex first(m_dragIndexes.first());
        const QModelIndex last(m_dragIndexes.last());
        m_dragIndexes.clear();
        // TODO: Optimize to Q_EMIT contiguous groups.
        Q_EMIT dataChanged(first, last, QVector<int>() << BlankRole);
    }
}

static bool isDropBetweenSharedViews(const QList<QUrl> &urls, const QUrl &folderUrl)
{
    for (const auto &url : urls) {
        if (folderUrl.adjusted(QUrl::StripTrailingSlash) != url.adjusted(QUrl::RemoveFilename | QUrl::StripTrailingSlash)) {
            return false;
        }
    }
    return true;
}

static const char *s_ark_dndextract_service = "application/x-kde-ark-dndextract-service";
static const char *s_ark_dndextract_path = "application/x-kde-ark-dndextract-path";

static QString arkDbusServiceName(const QMimeData *mimeData)
{
    return QString::fromUtf8(mimeData->data(QString::fromLatin1(s_ark_dndextract_service)));
}

static QString arkDbusPath(const QMimeData *mimeData)
{
    return QString::fromUtf8(mimeData->data(QString::fromLatin1(s_ark_dndextract_path)));
}

static bool isMimeDataArkDnd(const QMimeData *mimeData)
{
    return mimeData->hasFormat(QString::fromLatin1(s_ark_dndextract_service)) //
        && mimeData->hasFormat(QString::fromLatin1(s_ark_dndextract_path));
}

void FolderModel::drop(QQuickItem *target, QObject *dropEvent, int row, bool showMenuManually)
{
    QMimeData *mimeData = qobject_cast<QMimeData *>(dropEvent->property("mimeData").value<QObject *>());

    if (!mimeData) {
        return;
    }

    QModelIndex idx;
    KFileItem item;

    if (row > -1 && row < rowCount()) {
        idx = index(row, 0);
        item = itemForIndex(idx);
    }

    QUrl dropTargetUrl;

    // So we get to run mostLocalUrl() over the current URL.
    if (item.isNull()) {
        item = rootItem();
    }

    if (item.isNull()) {
        dropTargetUrl = m_dirModel->dirLister()->url();
    } else if (m_parseDesktopFiles && item.isDesktopFile()) {
        const KDesktopFile file(item.targetUrl().path());

        if (file.hasLinkType()) {
            dropTargetUrl = QUrl(file.readUrl());
        } else {
            dropTargetUrl = item.mostLocalUrl();
        }
    } else {
        dropTargetUrl = item.mostLocalUrl();
    }

    auto dropTargetFolderUrl = dropTargetUrl;
    if (dropTargetFolderUrl.fileName() == QLatin1Char('.')) {
        // the target URL for desktop:/ is e.g. 'file://home/user/Desktop/.'
        dropTargetFolderUrl = dropTargetFolderUrl.adjusted(QUrl::RemoveFilename);
    }

    // use dropTargetUrl to resolve desktop:/ to the actual file location which is also used by the mime data
    /* QMimeData operates on local URLs, but the dir lister and thus screen mapper and positioner may
     * use a fancy scheme like desktop:/ instead. Ensure we always use the latter to properly map URLs,
     * i.e. go from file:///home/user/Desktop/file to desktop:/file
     */
    auto mappableUrl = [this, dropTargetFolderUrl](const QUrl &url) -> QUrl {
        if (dropTargetFolderUrl != m_dirModel->dirLister()->url()) {
            QString mappedUrl = url.toString();
            const auto local = dropTargetFolderUrl.toString();
            const auto internal = m_dirModel->dirLister()->url().toString();
            if (mappedUrl.startsWith(local)) {
                mappedUrl.replace(0, local.size(), internal);
            }
            return ScreenMapper::stringToUrl(mappedUrl);
        }
        return url;
    };

    const int x = dropEvent->property("x").toInt();
    const int y = dropEvent->property("y").toInt();
    const QPoint dropPos = {x, y};

    if (m_dragInProgress && row == -1 && !m_urlChangedWhileDragging) {
        if (m_locked || mimeData->urls().isEmpty()) {
            return;
        }

        setSortMode(-1);

        for (const auto &url : mimeData->urls()) {
            m_dropTargetPositions.insert(url.fileName(), dropPos);
            m_screenMapper->addMapping(mappableUrl(url), m_screen, ScreenMapper::DelayedSignal);
            m_screenMapper->removeItemFromDisabledScreen(mappableUrl(url));
        }
        Q_EMIT move(x, y, mimeData->urls());

        return;
    }

    if (isMimeDataArkDnd(mimeData)) {
        QDBusMessage message = QDBusMessage::createMethodCall(arkDbusServiceName(mimeData),
                                                              arkDbusPath(mimeData),
                                                              QStringLiteral("org.kde.ark.DndExtract"),
                                                              QStringLiteral("extractSelectedFilesTo"));
        message.setArguments({dropTargetUrl.toDisplayString(QUrl::PreferLocalFile)});

        QDBusConnection::sessionBus().call(message, QDBus::NoBlock);

        return;
    }

    if (idx.isValid() && !(flags(idx) & Qt::ItemIsDropEnabled)) {
        return;
    }

    // Catch drops from a Task Manager and convert to usable URL.
    if (!mimeData->hasUrls() && mimeData->hasFormat(QStringLiteral("text/x-orgkdeplasmataskmanager_taskurl"))) {
        QList<QUrl> urls = {QUrl(QString::fromUtf8(mimeData->data(QStringLiteral("text/x-orgkdeplasmataskmanager_taskurl"))))};
        mimeData->setUrls(urls);
    }

    if (m_usedByContainment && !m_screenMapper->sharedDesktops()) {
        if (isDropBetweenSharedViews(mimeData->urls(), dropTargetFolderUrl)) {
            setSortMode(-1);
            const QList<QUrl> urls = mimeData->urls();
            for (const auto &url : urls) {
                m_dropTargetPositions.insert(url.fileName(), dropPos);
                m_screenMapper->addMapping(mappableUrl(url), m_screen, ScreenMapper::DelayedSignal);
                m_screenMapper->removeItemFromDisabledScreen(mappableUrl(url));
            }
            m_dropTargetPositionsCleanup->start();
            return;
        }
    }

    Qt::DropAction proposedAction((Qt::DropAction)dropEvent->property("proposedAction").toInt());
    Qt::DropActions possibleActions(dropEvent->property("possibleActions").toInt());
    Qt::MouseButtons buttons(dropEvent->property("buttons").toInt());
    Qt::KeyboardModifiers modifiers(dropEvent->property("modifiers").toInt());

    auto pos = target->mapToScene(dropPos).toPoint();
    pos = target->window()->mapToGlobal(pos);
    QDropEvent ev(pos, possibleActions, mimeData, buttons, modifiers);
    ev.setDropAction(proposedAction);

    KIO::DropJobFlag flag = showMenuManually ? KIO::ShowMenuManually : KIO::DropJobDefaultFlags;
    KIO::DropJob *dropJob = KIO::drop(&ev, dropTargetUrl, flag);
    dropJob->uiDelegate()->setAutoErrorHandlingEnabled(true);

    // The QMimeData we extract from the DropArea's drop event is deleted as soon as this method
    // ends but we need to keep a copy for when popupMenuAboutToShow fires.
    QMimeData *mimeCopy = new QMimeData();
    const QStringList formats = mimeData->formats();
    for (const QString &format : formats) {
        mimeCopy->setData(format, mimeData->data(format));
    }

    connect(dropJob, &KIO::DropJob::popupMenuAboutToShow, this, [this, mimeCopy, x, y, dropJob](const KFileItemListProperties &) {
        Q_EMIT popupMenuAboutToShow(dropJob, mimeCopy, x, y);
        mimeCopy->deleteLater();
    });

    /*
     * Position files that come from a drag'n'drop event at the drop event
     * target position. To do so, we first listen to copy job to figure out
     * the target URL. Then we store the position of this drop event in the
     * hash and eventually trigger a move request when we get notified about
     * the new file event from the source model.
     */
    connect(dropJob, &KIO::DropJob::copyJobStarted, this, [this, dropPos, dropTargetUrl](KIO::CopyJob *copyJob) {
        auto map = [this, dropPos, dropTargetUrl](const QUrl &targetUrl) {
            m_dropTargetPositions.insert(targetUrl.fileName(), dropPos);
            m_dropTargetPositionsCleanup->start();

            if (m_usedByContainment && !m_screenMapper->sharedDesktops()) {
                // assign a screen for the item before the copy is actually done, so
                // filterAcceptsRow doesn't assign the default screen to it
                QUrl url = resolvedUrl();
                // if the folderview's folder is a standard path, just use the targetUrl for mapping
                if (targetUrl.toString().startsWith(url.toString())) {
                    m_screenMapper->addMapping(targetUrl, m_screen, ScreenMapper::DelayedSignal);
                } else if (targetUrl.toString().startsWith(dropTargetUrl.toString())) {
                    // if the folderview's folder is a special path, like desktop:// , we need to convert
                    // the targetUrl file:// path to a desktop:/ path for mapping
                    auto destPath = dropTargetUrl.path();
                    auto filePath = targetUrl.path();
                    if (filePath.startsWith(destPath)) {
                        url.setPath(filePath.remove(0, destPath.length()));
                        m_screenMapper->addMapping(url, m_screen, ScreenMapper::DelayedSignal);
                    }
                }
            }
        };
        // remember drop target position for target URL and forget about the source URL
        connect(copyJob, &KIO::CopyJob::copyingDone, this, [map](KIO::Job *, const QUrl &, const QUrl &targetUrl, const QDateTime &, bool, bool) {
            map(targetUrl);
        });
        connect(copyJob, &KIO::CopyJob::copyingLinkDone, this, [map](KIO::Job *, const QUrl &, const QString &, const QUrl &targetUrl) {
            map(targetUrl);
        });
    });
}

void FolderModel::dropCwd(QObject *dropEvent)
{
    QMimeData *mimeData = qobject_cast<QMimeData *>(dropEvent->property("mimeData").value<QObject *>());

    if (!mimeData) {
        return;
    }

    if (isMimeDataArkDnd(mimeData)) {
        QDBusMessage message = QDBusMessage::createMethodCall(arkDbusServiceName(mimeData),
                                                              arkDbusPath(mimeData),
                                                              QStringLiteral("org.kde.ark.DndExtract"),
                                                              QStringLiteral("extractSelectedFilesTo"));
        message.setArguments(QVariantList() << m_dirModel->dirLister()->url().adjusted(QUrl::PreferLocalFile).toString());

        QDBusConnection::sessionBus().call(message, QDBus::NoBlock);
    } else {
        Qt::DropAction proposedAction((Qt::DropAction)dropEvent->property("proposedAction").toInt());
        Qt::DropActions possibleActions(dropEvent->property("possibleActions").toInt());
        Qt::MouseButtons buttons(dropEvent->property("buttons").toInt());
        Qt::KeyboardModifiers modifiers(dropEvent->property("modifiers").toInt());

        QDropEvent ev(QPoint(), possibleActions, mimeData, buttons, modifiers);
        ev.setDropAction(proposedAction);

        KIO::DropJob *dropJob = KIO::drop(&ev, m_dirModel->dirLister()->url().adjusted(QUrl::PreferLocalFile));
        dropJob->uiDelegate()->setAutoErrorHandlingEnabled(true);
    }
}

void FolderModel::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    QModelIndexList indices = selected.indexes();
    indices.append(deselected.indexes());

    QVector<int> roles;
    roles.append(SelectedRole);

    foreach (const QModelIndex &index, indices) {
        Q_EMIT dataChanged(index, index, roles);
    }

    if (!m_selectionModel->hasSelection()) {
        clearDragImages();
    } else {
        foreach (const QModelIndex &idx, deselected.indexes()) {
            delete m_dragImages.take(idx.row());
        }
    }

    updateActions();
}

bool FolderModel::isBlank(int row) const
{
    if (row < 0) {
        return true;
    }

    return data(index(row, 0), BlankRole).toBool();
}

QVariant FolderModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    if (role == BlankRole) {
        return m_dragIndexes.contains(index);
    } else if (role == SelectedRole) {
        return m_selectionModel->isSelected(index);
    } else if (role == IsDirRole) {
        return isDir(mapToSource(index), m_dirModel);
    } else if (role == IsLinkRole) {
        const KFileItem item = itemForIndex(index);
        return item.isLink();
    } else if (role == IsHiddenRole) {
        const KFileItem item = itemForIndex(index);
        return item.isHidden();
    } else if (role == UrlRole) {
        return itemForIndex(index).url();
    } else if (role == LinkDestinationUrl) {
        const KFileItem item = itemForIndex(index);

        if (m_parseDesktopFiles && item.isDesktopFile()) {
            const KDesktopFile file(item.targetUrl().path());

            if (file.hasLinkType()) {
                return file.readUrl();
            }
        }

        return item.targetUrl();
    } else if (role == SizeRole) {
        bool isDir = data(index, IsDirRole).toBool();

        if (!isDir) {
            return m_dirModel->data(mapToSource(QSortFilterProxyModel::index(index.row(), 1)), Qt::DisplayRole);
        }
    } else if (role == TypeRole) {
        return m_dirModel->data(mapToSource(QSortFilterProxyModel::index(index.row(), 6)), Qt::DisplayRole);
    } else if (role == FileNameRole) {
        return itemForIndex(index).url().fileName();
    } else if (role == FileNameWrappedRole) {
        return KStringHandler::preProcessWrap(itemForIndex(index).text());
    }

    return QSortFilterProxyModel::data(index, role);
}

int FolderModel::indexForUrl(const QUrl &url) const
{
    return mapFromSource(m_dirModel->indexForUrl(url)).row();
}

KFileItem FolderModel::itemForIndex(const QModelIndex &index) const
{
    return m_dirModel->itemForIndex(mapToSource(index));
}

bool FolderModel::isDir(const QModelIndex &index, const KDirModel *dirModel) const
{
    KFileItem item = dirModel->itemForIndex(index);
    if (item.isDir()) {
        return true;
    }

    auto it = m_isDirCache.constFind(item.url());
    if (it != m_isDirCache.constEnd()) {
        return *it;
    }

    if (m_parseDesktopFiles && item.isDesktopFile()) {
        // Check if the desktop file is a link to a directory
        KDesktopFile file(item.targetUrl().path());

        if (!file.hasLinkType()) {
            return false;
        }

        const QUrl url(file.readUrl());

        // Check if we already have a running StatJob for this URL.
        if (m_isDirJobs.contains(item.url())) {
            return false;
        }

        // Assume the root folder of a protocol is always a folder.
        // This avoids spinning up e.g. trash KIO slave just to check whether trash:/ is a folder.
        if (url.path() == QLatin1String("/")) {
            m_isDirCache.insert(item.url(), true);
            return true;
        }

        if (KProtocolInfo::protocolClass(url.scheme()) != QLatin1String(":local")) {
            return false;
        }

        KIO::StatJob *job = KIO::stat(url, KIO::HideProgressInfo);
        job->setProperty("org.kde.plasma.folder_url", item.url());
        job->setSide(KIO::StatJob::SourceSide);
        job->setDetails(0);
        connect(job, &KJob::result, this, &FolderModel::statResult);
        m_isDirJobs.insert(item.url(), job);
    }

    return false;
}

void FolderModel::statResult(KJob *job)
{
    KIO::StatJob *statJob = static_cast<KIO::StatJob *>(job);

    const QUrl &url = statJob->property("org.kde.plasma.folder_url").toUrl();
    const QModelIndex &idx = index(indexForUrl(url), 0);

    if (idx.isValid() && statJob->error() == KJob::NoError) {
        m_isDirCache[url] = statJob->statResult().isDir();

        Q_EMIT dataChanged(idx, idx, QVector<int>() << IsDirRole);
    }

    m_isDirJobs.remove(url);
}

void FolderModel::evictFromIsDirCache(const KFileItemList &items)
{
    foreach (const KFileItem &item, items) {
        m_screenMapper->removeFromMap(item.url());
        m_isDirCache.remove(item.url());
    }
}

bool FolderModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    const KDirModel *dirModel = static_cast<KDirModel *>(sourceModel());

    if (m_sortDirsFirst || left.column() == KDirModel::Size) {
        bool leftIsDir = isDir(left, dirModel);
        bool rightIsDir = isDir(right, dirModel);

        if (leftIsDir && !rightIsDir) {
            return (sortOrder() == Qt::AscendingOrder);
        }

        if (!leftIsDir && rightIsDir) {
            return (sortOrder() == Qt::DescendingOrder);
        }
    }

    const KFileItem leftItem = dirModel->data(left, KDirModel::FileItemRole).value<KFileItem>();
    const KFileItem rightItem = dirModel->data(right, KDirModel::FileItemRole).value<KFileItem>();
    const int column = left.column();
    int result = 0;

    switch (column) {
    case KDirModel::Size: {
        if (isDir(left, dirModel) && isDir(right, dirModel)) {
            const int leftChildCount = dirModel->data(left, KDirModel::ChildCountRole).toInt();
            const int rightChildCount = dirModel->data(right, KDirModel::ChildCountRole).toInt();
            if (leftChildCount < rightChildCount)
                result = -1;
            else if (leftChildCount > rightChildCount)
                result = +1;
        } else {
            const KIO::filesize_t leftSize = leftItem.size();
            const KIO::filesize_t rightSize = rightItem.size();
            if (leftSize < rightSize)
                result = -1;
            else if (leftSize > rightSize)
                result = +1;
        }

        break;
    }
    case KDirModel::ModifiedTime: {
        const long long leftTime = leftItem.entry().numberValue(KIO::UDSEntry::UDS_MODIFICATION_TIME, -1);
        const long long rightTime = rightItem.entry().numberValue(KIO::UDSEntry::UDS_MODIFICATION_TIME, -1);
        if (leftTime < rightTime)
            result = -1;
        else if (leftTime > rightTime)
            result = +1;

        break;
    }
    case KDirModel::Type:
        result = QString::compare(dirModel->data(left, Qt::DisplayRole).toString(), dirModel->data(right, Qt::DisplayRole).toString());
        break;

    default:
        break;
    }

    if (result != 0)
        return result < 0;

    QCollator collator;

    result = collator.compare(leftItem.text(), rightItem.text());

    if (result != 0)
        return result < 0;

    result = collator.compare(leftItem.name(), rightItem.name());

    if (result != 0)
        return result < 0;

    return QString::compare(leftItem.url().url(), rightItem.url().url(), Qt::CaseSensitive);
}

Qt::DropActions FolderModel::supportedDragActions() const
{
    return Qt::CopyAction | Qt::MoveAction | Qt::LinkAction;
}

Qt::DropActions FolderModel::supportedDropActions() const
{
    return Qt::CopyAction | Qt::MoveAction | Qt::LinkAction;
}

inline bool FolderModel::matchMimeType(const KFileItem &item) const
{
    if (m_mimeSet.isEmpty()) {
        return false;
    }

    if (m_mimeSet.contains(QLatin1String("all/all")) || m_mimeSet.contains(QLatin1String("all/allfiles"))) {
        return true;
    }

    const QString mimeType = item.determineMimeType().name();
    return m_mimeSet.contains(mimeType);
}

inline bool FolderModel::matchPattern(const KFileItem &item) const
{
    if (m_filterPatternMatchAll) {
        return true;
    }

    const QString name = item.name();
    QListIterator<QRegExp> i(m_regExps);
    while (i.hasNext()) {
        if (i.next().exactMatch(name)) {
            return true;
        }
    }

    return false;
}

bool FolderModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    const KDirModel *dirModel = static_cast<KDirModel *>(sourceModel());
    const KFileItem item = dirModel->itemForIndex(dirModel->index(sourceRow, KDirModel::Name, sourceParent));

    if (m_usedByContainment && !m_screenMapper->sharedDesktops()) {
        const QUrl url = item.url();
        const int screen = m_screenMapper->screenForItem(url);
        // don't do anything if the folderview is not associated with a screen
        if (m_screenUsed && screen == -1) {
            // The item is not associated with a screen, probably because this is the first
            // time we see it or the folderview was previously used as a regular applet.
            // Associated with this folderview if the view is on the first available screen
            if (m_screen == m_screenMapper->firstAvailableScreen(resolvedUrl())) {
                m_screenMapper->addMapping(url, m_screen, ScreenMapper::DelayedSignal);
            } else {
                return false;
            }
        } else if (m_screen != screen) {
            // the item belongs to a different screen, filter it out
            return false;
        }
    }

    if (m_filterMode == NoFilter) {
        return true;
    }

    if (m_filterMode == FilterShowMatches) {
        return (matchPattern(item) && matchMimeType(item));
    } else {
        return !(matchPattern(item) && matchMimeType(item));
    }
}

void FolderModel::createActions()
{
    KIO::FileUndoManager *manager = KIO::FileUndoManager::self();

    QAction *cut = KStandardAction::cut(this, &FolderModel::cut, this);
    QAction *copy = KStandardAction::copy(this, &FolderModel::copy, this);

    QAction *undo = KStandardAction::undo(manager, &KIO::FileUndoManager::undo, this);
    undo->setEnabled(manager->isUndoAvailable());
    undo->setShortcutContext(Qt::WidgetShortcut);
    connect(manager, SIGNAL(undoAvailable(bool)), undo, SLOT(setEnabled(bool)));
    connect(manager, &KIO::FileUndoManager::undoTextChanged, this, &FolderModel::undoTextChanged);

    QAction *paste = KStandardAction::paste(this, &FolderModel::paste, this);
    QAction *pasteTo = KStandardAction::paste(this, &FolderModel::pasteTo, this);

    QAction *refresh = new QAction(QIcon::fromTheme(QStringLiteral("view-refresh")), i18n("&Refresh View"), this);
    refresh->setShortcut(QKeySequence(QKeySequence::Refresh));
    connect(refresh, &QAction::triggered, this, &FolderModel::refresh);

    QAction *rename = KStandardAction::renameFile(this, &FolderModel::requestRename, this);
    QAction *trash = KStandardAction::moveToTrash(this, &FolderModel::moveSelectedToTrash, this);
    QAction *del = KStandardAction::deleteFile(this, &FolderModel::deleteSelected, this);
    RemoveAction *remove = new RemoveAction(&m_actionCollection, this);

    QAction *emptyTrash = new QAction(QIcon::fromTheme(QStringLiteral("trash-empty")), i18n("&Empty Trash"), this);
    connect(emptyTrash, &QAction::triggered, this, &FolderModel::emptyTrashBin);

    QAction *restoreFromTrash = new QAction(i18nc("Restore from trash", "Restore"), this);
    connect(restoreFromTrash, &QAction::triggered, this, &FolderModel::restoreSelectedFromTrash);

    QAction *actOpen = new QAction(QIcon::fromTheme(QStringLiteral("window-new")), i18n("&Open"), this);
    connect(actOpen, &QAction::triggered, this, &FolderModel::openSelected);

    m_actionCollection.addAction(QStringLiteral("open"), actOpen);
    m_actionCollection.addAction(QStringLiteral("cut"), cut);
    m_actionCollection.addAction(QStringLiteral("undo"), undo);
    m_actionCollection.addAction(QStringLiteral("copy"), copy);
    m_actionCollection.addAction(QStringLiteral("paste"), paste);
    m_actionCollection.addAction(QStringLiteral("pasteto"), pasteTo);
    m_actionCollection.addAction(QStringLiteral("refresh"), refresh);
    m_actionCollection.addAction(QStringLiteral("rename"), rename);
    m_actionCollection.addAction(QStringLiteral("remove"), remove);
    m_actionCollection.addAction(QStringLiteral("trash"), trash);
    m_actionCollection.addAction(QStringLiteral("del"), del);
    m_actionCollection.addAction(QStringLiteral("restoreFromTrash"), restoreFromTrash);
    m_actionCollection.addAction(QStringLiteral("emptyTrash"), emptyTrash);

    // The RemoveAction needs to be updated after adding all actions to the actionCollection
    remove->update();

    m_newMenu = new KNewFileMenu(&m_actionCollection, QStringLiteral("newMenu"), this);
    m_newMenu->setModal(false);
    connect(m_newMenu, &KNewFileMenu::directoryCreated, this, &FolderModel::newFileMenuItemCreated);
    connect(m_newMenu, &KNewFileMenu::fileCreated, this, &FolderModel::newFileMenuItemCreated);

    m_copyToMenu = new KFileCopyToMenu(nullptr);
}

QAction *FolderModel::action(const QString &name) const
{
    return m_actionCollection.action(name);
}

QObject *FolderModel::newMenu() const
{
    return m_newMenu->menu();
}

void FolderModel::updateActions()
{
    const QModelIndexList indexes = m_selectionModel->selectedIndexes();

    KFileItemList items;
    QList<QUrl> urls;
    bool hasRemoteFiles = false;
    bool isTrashLink = false;
    const bool isTrash = (resolvedUrl().scheme() == QLatin1String("trash"));

    if (indexes.isEmpty()) {
        items << rootItem();
    } else {
        items.reserve(indexes.count());
        urls.reserve(indexes.count());
        for (const QModelIndex &index : indexes) {
            KFileItem item = itemForIndex(index);
            if (!item.isNull()) {
                hasRemoteFiles |= item.localPath().isEmpty();
                items.append(item);
                urls.append(item.url());
            }
        }
    }

    KFileItemListProperties itemProperties(items);
    // Check if we're showing the menu for the trash link
    if (items.count() == 1 && items.at(0).isDesktopFile()) {
        KDesktopFile file(items.at(0).localPath());
        if (file.hasLinkType() && file.readUrl() == QLatin1String("trash:/")) {
            isTrashLink = true;
        }
    }

    if (m_newMenu) {
        m_newMenu->checkUpToDate();
        m_newMenu->setPopupFiles(QList<QUrl>() << m_dirModel->dirLister()->url());
        // we need to set here as well, when the menu is shown via AppletInterface::eventFilter
        m_menuPosition = QCursor::pos();

        if (QAction *newMenuAction = m_actionCollection.action(QStringLiteral("newMenu"))) {
            newMenuAction->setEnabled(itemProperties.supportsWriting());
            newMenuAction->setVisible(!isTrash);
        }
    }

    if (QAction *emptyTrash = m_actionCollection.action(QStringLiteral("emptyTrash"))) {
        if (isTrash || isTrashLink) {
            emptyTrash->setVisible(true);
            emptyTrash->setEnabled(!isTrashEmpty());
        } else {
            emptyTrash->setVisible(false);
        }
    }

    if (QAction *restoreFromTrash = m_actionCollection.action(QStringLiteral("restoreFromTrash"))) {
        restoreFromTrash->setVisible(isTrash);
    }

    if (QAction *remove = m_actionCollection.action(QStringLiteral("remove"))) {
        remove->setVisible(!hasRemoteFiles && itemProperties.supportsMoving() && itemProperties.supportsDeleting());
    }

    if (QAction *cut = m_actionCollection.action(QStringLiteral("cut"))) {
        cut->setEnabled(itemProperties.supportsDeleting());
        cut->setVisible(!isTrash);
    }

    if (QAction *paste = m_actionCollection.action(QStringLiteral("paste"))) {
        bool enable = false;

        const QString pasteText = KIO::pasteActionText(QApplication::clipboard()->mimeData(), &enable, rootItem());

        if (enable) {
            paste->setText(pasteText);
            paste->setEnabled(true);
        } else {
            paste->setText(i18n("&Paste"));
            paste->setEnabled(false);
        }

        if (QAction *pasteTo = m_actionCollection.action(QStringLiteral("pasteto"))) {
            pasteTo->setVisible(itemProperties.isDirectory() && itemProperties.supportsWriting());
            pasteTo->setEnabled(paste->isEnabled());
            pasteTo->setText(paste->text());
        }
    }

    if (QAction *rename = m_actionCollection.action(QStringLiteral("rename"))) {
        rename->setEnabled(itemProperties.supportsMoving());
        rename->setVisible(!isTrash);
    }
}

void FolderModel::openContextMenu(QQuickItem *visualParent, Qt::KeyboardModifiers modifiers)
{
    if (m_usedByContainment && !KAuthorized::authorize(QStringLiteral("action/kdesktop_rmb"))) {
        return;
    }

    updateActions();

    const QModelIndexList indexes = m_selectionModel->selectedIndexes();

    QMenu *menu = new QMenu();
    if (!m_fileItemActions) {
        m_fileItemActions = new KFileItemActions(this);
        m_fileItemActions->setParentWidget(QApplication::desktop());
    }

    if (indexes.isEmpty()) {
        menu->addAction(m_actionCollection.action(QStringLiteral("newMenu")));
        menu->addSeparator();
        menu->addAction(m_actionCollection.action(QStringLiteral("paste")));
        menu->addAction(m_actionCollection.action(QStringLiteral("undo")));
        menu->addAction(m_actionCollection.action(QStringLiteral("refresh")));
        menu->addAction(m_actionCollection.action(QStringLiteral("emptyTrash")));
        menu->addSeparator();

        KFileItemListProperties itemProperties(KFileItemList() << rootItem());
        m_fileItemActions->setItemListProperties(itemProperties);

        m_fileItemActions->insertOpenWithActionsTo(nullptr, menu, QStringList());
    } else {
        KFileItemList items;
        QList<QUrl> urls;

        items.reserve(indexes.count());
        urls.reserve(indexes.count());
        for (const QModelIndex &index : indexes) {
            KFileItem item = itemForIndex(index);
            if (!item.isNull()) {
                items.append(item);
                urls.append(item.url());
            }
        }

        KFileItemListProperties itemProperties(items);

        // Start adding the actions:
        // "Open" and "Open with" actions
        m_fileItemActions->setItemListProperties(itemProperties);
        m_fileItemActions->addOpenWithActionsTo(menu);
        menu->addSeparator();
        menu->addAction(m_actionCollection.action(QStringLiteral("cut")));
        menu->addAction(m_actionCollection.action(QStringLiteral("copy")));
        if (urls.length() == 1 && items.first().isDir()) {
            menu->addAction(m_actionCollection.action(QStringLiteral("pasteto")));
        } else {
            menu->addAction(m_actionCollection.action(QStringLiteral("paste")));
        }

        menu->addAction(m_actionCollection.action(QStringLiteral("rename")));
        menu->addSeparator();
        menu->addAction(m_actionCollection.action(QStringLiteral("restoreFromTrash")));

        if (isDeleteCommandShown()) {
            QAction *trashAction = m_actionCollection.action(QStringLiteral("trash"));
            QAction *deleteAction = m_actionCollection.action(QStringLiteral("del"));
            menu->addAction(trashAction);
            menu->addAction(deleteAction);
        } else {
            if (RemoveAction *removeAction = qobject_cast<RemoveAction *>(m_actionCollection.action(QStringLiteral("remove")))) {
                removeAction->update();
                menu->addAction(removeAction);

                // Used to monitor Shift modifier usage while the menu is open, to
                // swap the Trash and Delete actions.
                menu->installEventFilter(removeAction);
                QCoreApplication::instance()->installEventFilter(removeAction);
            }
        }

        menu->addAction(m_actionCollection.action(QStringLiteral("emptyTrash")));

        menu->addSeparator();

        m_fileItemActions->addActionsTo(menu);

        // Copy To, Move To
        KSharedConfig::Ptr dolphin = KSharedConfig::openConfig(QStringLiteral("dolphinrc"));
        if (KConfigGroup(dolphin, "General").readEntry("ShowCopyMoveMenu", false)) {
            m_copyToMenu->setUrls(urls);
            m_copyToMenu->setReadOnly(!itemProperties.supportsMoving());
            m_copyToMenu->addActionsTo(menu);
            menu->addSeparator();
        }

        // Properties
        if (KPropertiesDialog::canDisplay(items)) {
            menu->addSeparator();
            QAction *act = new QAction(QIcon::fromTheme(QStringLiteral("document-properties")), i18n("&Properties"), menu);
            act->setShortcuts({Qt::ALT | Qt::Key_Return, Qt::ALT | Qt::Key_Enter});
            QObject::connect(act, &QAction::triggered, this, &FolderModel::openPropertiesDialog);
            menu->addAction(act);
        }
    }

    menu->setAttribute(Qt::WA_TranslucentBackground);
    menu->winId(); // force surface creation before ensurePolish call in menu::Popup which happens before show

    if (visualParent && menu->windowHandle()) {
        menu->windowHandle()->setTransientParent(visualParent->window());
    }

    menu->popup(m_menuPosition);
    connect(menu, &QMenu::aboutToHide, [menu]() {
        menu->deleteLater();
    });
}

void FolderModel::openPropertiesDialog()
{
    const QModelIndexList indexes = m_selectionModel->selectedIndexes();
    if (indexes.isEmpty()) {
        return;
    }

    KFileItemList items;
    items.reserve(indexes.count());
    for (const QModelIndex &index : indexes) {
        KFileItem item = itemForIndex(index);
        if (!item.isNull()) {
            items.append(item);
        }
    }

    if (!KPropertiesDialog::canDisplay(items)) {
        return;
    }

    KPropertiesDialog::showDialog(items, nullptr, false /*non modal*/);
}

void FolderModel::linkHere(const QUrl &sourceUrl)
{
    KIO::CopyJob *job = KIO::link(sourceUrl, m_dirModel->dirLister()->url(), KIO::HideProgressInfo);
    KIO::FileUndoManager::self()->recordCopyJob(job);
}

QList<QUrl> FolderModel::selectedUrls() const
{
    const auto indexes = m_selectionModel->selectedIndexes();

    QList<QUrl> urls;
    urls.reserve(indexes.count());

    for (const QModelIndex &index : indexes) {
        urls.append(itemForIndex(index).url());
    }

    return urls;
}

void FolderModel::copy()
{
    if (!m_selectionModel->hasSelection()) {
        return;
    }

    if (QAction *action = m_actionCollection.action(QStringLiteral("copy"))) {
        if (!action->isEnabled()) {
            return;
        }
    }

    QMimeData *mimeData = QSortFilterProxyModel::mimeData(m_selectionModel->selectedIndexes());
    QApplication::clipboard()->setMimeData(mimeData);
}

void FolderModel::cut()
{
    if (!m_selectionModel->hasSelection()) {
        return;
    }

    if (QAction *action = m_actionCollection.action(QStringLiteral("cut"))) {
        if (!action->isEnabled()) {
            return;
        }
    }

    QMimeData *mimeData = QSortFilterProxyModel::mimeData(m_selectionModel->selectedIndexes());
    KIO::setClipboardDataCut(mimeData, true);
    QApplication::clipboard()->setMimeData(mimeData);
}

void FolderModel::paste()
{
    if (QAction *action = m_actionCollection.action(QStringLiteral("paste"))) {
        if (!action->isEnabled()) {
            return;
        }
    }

    KIO::paste(QApplication::clipboard()->mimeData(), m_dirModel->dirLister()->url());
}

void FolderModel::pasteTo()
{
    const QList<QUrl> urls = selectedUrls();
    Q_ASSERT(urls.count() == 1);
    KIO::paste(QApplication::clipboard()->mimeData(), urls.first());
}

void FolderModel::refresh()
{
    m_errorString.clear();
    Q_EMIT errorStringChanged();

    m_dirModel->dirLister()->updateDirectory(m_dirModel->dirLister()->url());
}

QObject *FolderModel::appletInterface() const
{
    return m_appletInterface;
}

void FolderModel::setAppletInterface(QObject *appletInterface)
{
    if (m_appletInterface != appletInterface) {
        Q_ASSERT(!m_appletInterface);

        m_appletInterface = appletInterface;

        if (appletInterface) {
            Plasma::Applet *applet = appletInterface->property("_plasma_applet").value<Plasma::Applet *>();

            if (applet) {
                Plasma::Containment *containment = applet->containment();

                if (containment) {
                    Plasma::Corona *corona = containment->corona();

                    if (corona) {
                        m_screenMapper->setCorona(corona);
                    }
                    setScreen(containment->screen());
                    connect(containment, &Plasma::Containment::screenChanged, this, &FolderModel::setScreen);
                }
            }
        }

        Q_EMIT appletInterfaceChanged();
    }
}

void FolderModel::moveSelectedToTrash()
{
    if (!m_selectionModel->hasSelection()) {
        return;
    }

    if (!isDeleteCommandShown()) {
        if (RemoveAction *action = qobject_cast<RemoveAction *>(m_actionCollection.action(QStringLiteral("remove")))) {
            if (action->proxyAction() != m_actionCollection.action(QStringLiteral("trash"))) {
                return;
            }
        }
    }

    if (QAction *action = m_actionCollection.action(QStringLiteral("trash"))) {
        if (!action->isEnabled()) {
            return;
        }
    }

    const QList<QUrl> urls = selectedUrls();
    KIO::JobUiDelegate uiDelegate;

    if (uiDelegate.askDeleteConfirmation(urls, KIO::JobUiDelegate::Trash, KIO::JobUiDelegate::DefaultConfirmation)) {
        KIO::Job *job = KIO::trash(urls);
        job->uiDelegate()->setAutoErrorHandlingEnabled(true);
        KIO::FileUndoManager::self()->recordJob(KIO::FileUndoManager::Trash, urls, QUrl(QStringLiteral("trash:/")), job);
    }
}

void FolderModel::deleteSelected()
{
    if (!m_selectionModel->hasSelection()) {
        return;
    }

    if (QAction *action = m_actionCollection.action(QStringLiteral("del"))) {
        if (!action->isEnabled()) {
            return;
        }
    }

    const QList<QUrl> urls = selectedUrls();
    KIO::JobUiDelegate uiDelegate;

    if (uiDelegate.askDeleteConfirmation(urls, KIO::JobUiDelegate::Delete, KIO::JobUiDelegate::DefaultConfirmation)) {
        KIO::Job *job = KIO::del(urls);
        job->uiDelegate()->setAutoErrorHandlingEnabled(true);
    }
}

void FolderModel::openSelected()
{
    if (!m_selectionModel->hasSelection()) {
        return;
    }

    const QList<QUrl> urls = selectedUrls();
    for (const QUrl &url : urls) {
        (void)new KRun(url, nullptr);
    }
}

void FolderModel::undo()
{
    if (QAction *action = m_actionCollection.action(QStringLiteral("undo"))) {
        // trigger() doesn't check enabled and would crash if invoked nonetheless.
        if (action->isEnabled()) {
            action->trigger();
        }
    }
}

void FolderModel::emptyTrashBin()
{
    KIO::JobUiDelegate uiDelegate;
    uiDelegate.setWindow(QApplication::desktop());

    if (uiDelegate.askDeleteConfirmation(QList<QUrl>(), KIO::JobUiDelegate::EmptyTrash, KIO::JobUiDelegate::DefaultConfirmation)) {
        KIO::Job *job = KIO::emptyTrash();
        job->uiDelegate()->setAutoErrorHandlingEnabled(true);
    }
}

void FolderModel::restoreSelectedFromTrash()
{
    if (!m_selectionModel->hasSelection()) {
        return;
    }

    const auto &urls = selectedUrls();

    KIO::RestoreJob *job = KIO::restoreFromTrash(urls);
    job->uiDelegate()->setAutoErrorHandlingEnabled(true);
}

bool FolderModel::isTrashEmpty()
{
    KConfig trashConfig(QStringLiteral("trashrc"), KConfig::SimpleConfig);
    return trashConfig.group("Status").readEntry("Empty", true);
}

void FolderModel::undoTextChanged(const QString &text)
{
    if (QAction *action = m_actionCollection.action(QStringLiteral("undo"))) {
        action->setText(text);
    }
}

void FolderModel::createFolder()
{
    m_newMenu->setPopupFiles(QList<QUrl>() << m_dirModel->dirLister()->url());
    m_newMenu->createDirectory();
}

bool FolderModel::isDeleteCommandShown()
{
    KConfigGroup cg(KSharedConfig::openConfig(), "KDE");
    return cg.readEntry("ShowDeleteCommand", false);
}
