/***************************************************************************
 *   Copyright (C) 2006 David Faure <faure@kde.org>                        *
 *   Copyright (C) 2008 Fredrik Höglund <fredrik@kde.org>                  *
 *   Copyright (C) 2008 Rafael Fernández López <ereslibre@kde.org>           *
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

#include "foldermodel.h"
#include "itemviewadapter.h"
#include "positioner.h"

#include <QApplication>
#include <QClipboard>
#include <QCollator>
#include <QDesktopWidget>
#include <QDrag>
#include <QImage>
#include <QItemSelectionModel>
#include <QMimeData>
#include <QPainter>
#include <QPixmap>
#include <QQuickItem>
#include <QQuickWindow>
#include <qplatformdefs.h>
#include <QDebug>

#include "internallibkonq/konq_popupmenu.h"
#include <KIO/DropJob>
#include <KAuthorized>
#include <KBookmarkManager>
#include <KConfigGroup>
#include <KFileItemActions>
#include <KFileItemListProperties>
#include <KNewFileMenu>
#include <KIO/DeleteJob>
#include <KIO/EmptyTrashJob>
#include <KIO/FileUndoManager>
#include <KIO/JobUiDelegate>
#include <KIO/Paste>
#include <KIO/PasteJob>
#include <KLocalizedString>
#include <KSharedConfig>
#include <KShell>

#include <KDesktopFile>
#include <KDirModel>
#include <KIO/CopyJob>
#include <KIO/Job>
#include <KProtocolInfo>
#include <KRun>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

DirLister::DirLister(QObject *parent) : KDirLister(parent)
{
}

DirLister:: ~DirLister()
{
}

void DirLister::handleError(KIO::Job *job)
{
    if (!autoErrorHandlingEnabled()) {
        emit error(job->errorString());
        return;
    }

    KDirLister::handleError(job);
}

FolderModel::FolderModel(QObject *parent) : QSortFilterProxyModel(parent),
    m_dragInProgress(false),
    m_previewGenerator(0),
    m_viewAdapter(0),
    m_actionCollection(this),
    m_newMenu(0),
    m_fileItemActions(0),
    m_usedByContainment(false),
    m_locked(true),
    m_sortMode(0),
    m_sortDesc(false),
    m_sortDirsFirst(true),
    m_parseDesktopFiles(false),
    m_previews(false),
    m_filterMode(NoFilter),
    m_filterPatternMatchAll(true)
{
    DirLister *dirLister = new DirLister(this);
    dirLister->setDelayedMimeTypes(true);
    dirLister->setAutoErrorHandlingEnabled(false, 0);
    connect(dirLister, SIGNAL(error(QString)), this, SLOT(dirListFailed(QString)));
    connect(dirLister, SIGNAL(itemsDeleted(KFileItemList)), this, SLOT(evictFromIsDirCache(KFileItemList)));

    m_dirModel = new KDirModel(this);
    m_dirModel->setDirLister(dirLister);
    m_dirModel->setDropsAllowed(KDirModel::DropOnDirectory | KDirModel::DropOnLocalExecutable);

    m_selectionModel = new QItemSelectionModel(this, this);
    connect(m_selectionModel, SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            this, SLOT(selectionChanged(QItemSelection,QItemSelection)));

    setSourceModel(m_dirModel);

    setSortLocaleAware(true);
    setFilterCaseSensitivity(Qt::CaseInsensitive);
    setDynamicSortFilter(true);

    sort(m_sortMode, m_sortDesc ? Qt::DescendingOrder : Qt::AscendingOrder);

    setSupportedDragActions(Qt::CopyAction | Qt::MoveAction | Qt::LinkAction);

    createActions();
}

FolderModel::~FolderModel()
{
}

QHash< int, QByteArray > FolderModel::roleNames() const
{
    return staticRoleNames();
}

QHash< int, QByteArray > FolderModel::staticRoleNames()
{
    QHash<int, QByteArray> roleNames;
    roleNames[Qt::DisplayRole] = "display";
    roleNames[Qt::DecorationRole] = "decoration";
    roleNames[BlankRole] = "blank";
    roleNames[SelectedRole] = "selected";
    roleNames[IsDirRole] = "isDir";
    roleNames[UrlRole] = "url";
    roleNames[LinkDestinationUrl] = "linkDestinationUrl";
    roleNames[SizeRole] = "size";
    roleNames[TypeRole] = "type";

    return roleNames;
}

QString FolderModel::url() const
{
    return m_url;
}

void FolderModel::setUrl(const QString& url)
{
    QUrl resolvedUrl;

    if (url.startsWith('~')) {
        resolvedUrl = QUrl::fromLocalFile(KShell::tildeExpand(url));
    } else {
        resolvedUrl = QUrl::fromUserInput(url);
    }

    if (url == m_url) {
        m_dirModel->dirLister()->updateDirectory(resolvedUrl);

        return;
    }

    beginResetModel();
    m_url = url;
    m_isDirCache.clear();
    m_dirModel->dirLister()->openUrl(resolvedUrl);
    clearDragImages();
    endResetModel();

    emit urlChanged();
    emit resolvedUrlChanged();

    m_errorString.clear();
    emit errorStringChanged();
}

QUrl FolderModel::resolvedUrl() const
{
    return m_dirModel->dirLister()->url();
}

QString FolderModel::errorString() const
{
    return m_errorString;
}

bool FolderModel::usedByContainment() const
{
    return m_usedByContainment;
}

void FolderModel::setUsedByContainment(bool used)
{
    if (m_usedByContainment != used) {
        m_usedByContainment = used;

        QAction *action = m_actionCollection.action("refresh");

        if (action) {
            action->setText(m_usedByContainment ? i18n("&Refresh Desktop") : i18n("&Refresh View"));
            action->setIcon(m_usedByContainment ? QIcon::fromTheme("user-desktop") : QIcon::fromTheme("view-refresh"));
        }

        emit usedByContainmentChanged();
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

        emit lockedChanged();
    }
}

void FolderModel::dirListFailed(const QString& error)
{
    m_errorString = error;
    emit errorStringChanged();
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
            invalidate();
            sort(m_sortMode, m_sortDesc ? Qt::DescendingOrder : Qt::AscendingOrder);
            setDynamicSortFilter(true);
        }

        emit sortModeChanged();
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
            invalidate();
            sort(m_sortMode, m_sortDesc ? Qt::DescendingOrder : Qt::AscendingOrder);
        }

        emit sortDescChanged();
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
            invalidate();
            sort(m_sortMode, m_sortDesc ? Qt::DescendingOrder : Qt::AscendingOrder);
        }

        emit sortDirsFirstChanged();
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
        emit parseDesktopFilesChanged();
    }
}

QObject* FolderModel::viewAdapter() const
{
    return m_viewAdapter;
}

void FolderModel::setViewAdapter(QObject* adapter)
{
    if (m_viewAdapter != adapter) {
        KAbstractViewAdapter *abstractViewAdapter = dynamic_cast<KAbstractViewAdapter *>(adapter);

        m_viewAdapter = abstractViewAdapter;

        if (m_viewAdapter && !m_previewGenerator) {
            m_previewGenerator = new KFilePreviewGenerator(abstractViewAdapter, this);
            m_previewGenerator->setPreviewShown(m_previews);
            m_previewGenerator->setEnabledPlugins(m_previewPlugins);
        }

        emit viewAdapterChanged();
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

        emit previewsChanged();
    }
}

QStringList FolderModel::previewPlugins() const
{
    return m_previewPlugins;
}

void FolderModel::setPreviewPlugins(const QStringList& previewPlugins)
{
    if (m_previewPlugins != previewPlugins) {
        m_previewPlugins = previewPlugins;

        if (m_previewGenerator) {
            m_previewGenerator->setPreviewShown(false);
            m_previewGenerator->setEnabledPlugins(m_previewPlugins);
            m_previewGenerator->setPreviewShown(true);
        }

        emit previewPluginsChanged();
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

        invalidateFilter();

        emit filterModeChanged();
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
    m_filterPatternMatchAll = (pattern == "*");

    const QStringList patterns = pattern.split(' ');
    m_regExps.clear();

    foreach (const QString &pattern, patterns) {
        QRegExp rx(pattern);
        rx.setPatternSyntax(QRegExp::Wildcard);
        rx.setCaseSensitivity(Qt::CaseInsensitive);
        m_regExps.append(rx);
    }

    emit filterPatternChanged();
}

QStringList FolderModel::filterMimeTypes() const
{
    return m_mimeSet.toList();
}

void FolderModel::setFilterMimeTypes(const QStringList &mimeList)
{
    const QSet<QString> &set = QSet<QString>::fromList(mimeList);

    if (m_mimeSet != set) {

        m_mimeSet = set;

        invalidateFilter();

        emit filterMimeTypesChanged();
    }
}

void FolderModel::run(int row)
{
    if (row < 0) {
        return;
    }

    KFileItem item = itemForIndex(index(row, 0));

    QUrl url(item.targetUrl());
    if (url.scheme().isEmpty()) {
        url.setScheme("file");
    }

    new KRun(url, 0);
}

void FolderModel::rename(int row, const QString& name)
{
    if (row < 0) {
        return;
    }

    QModelIndex idx = index(row, 0);
    m_dirModel->setData(mapToSource(idx), name, Qt::EditRole);
}

bool FolderModel::hasSelection()
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

    DragImage *dragImage = 0;

    if (m_dragImages.contains(row)) {
        dragImage = m_dragImages.value(row);
        delete dragImage;
        m_dragImages.remove(row);
    }

    dragImage = new DragImage();
    dragImage->row = row;
    dragImage->rect = QRect(x, y, width, height);
    dragImage->image = image.value<QImage>();
    dragImage->blank = false;

    m_dragImages.insert(row, dragImage);
}

void FolderModel::clearDragImages()
{
    if (!m_dragImages.isEmpty()) {
        foreach (DragImage *image, m_dragImages) {
            delete image;
        }

        m_dragImages.clear();
    }
}

void FolderModel::setDragHotSpotScrollOffset(int x, int y)
{
    m_dragHotSpotScrollOffset.setX(x);
    m_dragHotSpotScrollOffset.setY(y);
}

QPoint FolderModel::dragCursorOffset(int row)
{
    if (!m_dragImages.contains(row)) {
        return QPoint(-1, -1);
    }

    return m_dragImages.value(row)->cursorOffset;
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
    if (!m_viewAdapter || !m_selectionModel->hasSelection()) {
        return;
    }

    ItemViewAdapter *adapter = qobject_cast<ItemViewAdapter *>(m_viewAdapter);
    QQuickItem *item = qobject_cast<QQuickItem *>(adapter->adapterView());

    QDrag *drag = new QDrag(item);

    addDragImage(drag, x, y);

    m_dragIndexes = m_selectionModel->selectedIndexes();

    qSort(m_dragIndexes.begin(), m_dragIndexes.end());

    // TODO: Optimize to emit contiguous groups.
    emit dataChanged(m_dragIndexes.first(), m_dragIndexes.last(), QVector<int>() << BlankRole);

    QModelIndexList sourceDragIndexes;

    foreach (const QModelIndex &index, m_dragIndexes) {
        sourceDragIndexes.append(mapToSource(index));
    }

    drag->setMimeData(m_dirModel->mimeData(sourceDragIndexes));

    item->grabMouse();
    m_dragInProgress = true;
    drag->exec(supportedDragActions());
    m_dragInProgress = false;
    item->ungrabMouse();

    const QModelIndex first(m_dragIndexes.first());
    const QModelIndex last(m_dragIndexes.last());
    m_dragIndexes.clear();
    // TODO: Optimize to emit contiguous groups.
    emit dataChanged(first, last, QVector<int>() << BlankRole);
}

void FolderModel::drop(QQuickItem *target, QObject* dropEvent, int row)
{
    QMimeData *mimeData = qobject_cast<QMimeData *>(dropEvent->property("mimeData").value<QObject *>());

    if (!mimeData) {
        return;
    }

    if (m_dragInProgress && row == -1) {
        if (m_locked || mimeData->urls().isEmpty()) {
            return;
        }

        setSortMode(-1);

        emit move(dropEvent->property("x").toInt(), dropEvent->property("y").toInt(),
            mimeData->urls());

        return;
    }

    QModelIndex idx;
    KFileItem item;

    if (row > -1 && row < rowCount()) {
         idx = index(row, 0);
         item = itemForIndex(idx);
    }

    if (item.isNull() &&
        mimeData->hasFormat(QLatin1String("application/x-kde-ark-dndextract-service")) &&
        mimeData->hasFormat(QLatin1String("application/x-kde-ark-dndextract-path"))) {
        const QString remoteDBusClient = mimeData->data(QLatin1String("application/x-kde-ark-dndextract-service"));
        const QString remoteDBusPath = mimeData->data(QLatin1String("application/x-kde-ark-dndextract-path"));

        QDBusMessage message =
            QDBusMessage::createMethodCall(remoteDBusClient, remoteDBusPath,
                                            QLatin1String("org.kde.ark.DndExtract"),
                                            QLatin1String("extractSelectedFilesTo"));
        message.setArguments(QVariantList() << m_dirModel->dirLister()->url().adjusted(QUrl::PreferLocalFile));

        QDBusConnection::sessionBus().call(message);

        return;
    }

    if (idx.isValid() && !(flags(idx) & Qt::ItemIsDropEnabled)) {
        return;
    }

    QPoint pos;
    pos.setX(dropEvent->property("x").toInt());
    pos.setY(dropEvent->property("y").toInt());

    pos = target->mapToScene(pos).toPoint();
    pos = target->window()->mapToGlobal(pos);

    Qt::DropAction proposedAction((Qt::DropAction)dropEvent->property("proposedAction").toInt());
    Qt::DropActions possibleActions(dropEvent->property("possibleActions").toInt());
    Qt::MouseButtons buttons(dropEvent->property("buttons").toInt());
    Qt::KeyboardModifiers modifiers(dropEvent->property("modifiers").toInt());

    QDropEvent ev(pos, possibleActions, mimeData, buttons, modifiers);
    ev.setDropAction(proposedAction);

    KIO::DropJob *dropJob = KIO::drop(&ev, item.isNull() ? m_dirModel->dirLister()->url() : item.url());
    dropJob->ui()->setAutoErrorHandlingEnabled(true);
}

void FolderModel::selectionChanged(QItemSelection selected, QItemSelection deselected)
{
    QModelIndexList indices = selected.indexes();
    indices.append(deselected.indexes());

    QVector<int> roles;
    roles.append(SelectedRole);

    foreach(const QModelIndex index, indices) {
        emit dataChanged(index, index, roles);
    }

    if (!m_selectionModel->hasSelection()) {
        clearDragImages();
    } else {
        foreach (const QModelIndex &idx, deselected.indexes()) {
            if (m_dragImages.contains(idx.row())) {
                DragImage *image = m_dragImages.value(idx.row());
                delete image;
                m_dragImages.remove(idx.row());
            }
        }
    }
}

bool FolderModel::isBlank(int row) const
{
    if (row < 0) {
        return true;
    }

    return data(index(row, 0), BlankRole).toBool();
}

QVariant FolderModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    if (role == BlankRole) {
        return m_dragIndexes.contains(index);
    } else if (role == SelectedRole) {
        return m_selectionModel->isSelected(index);
    } else if (role == IsDirRole) {
        const QUrl &url = data(index, UrlRole).value<QUrl>();

        if (m_isDirCache.contains(url)) {
            return m_isDirCache[url];
        } else {
            return isDir(mapToSource(index), m_dirModel);
        }
    } else if (role == UrlRole) {
        return itemForIndex(index).url();
    } else if (role == LinkDestinationUrl) {
        const KFileItem item = itemForIndex(index);

        if (m_parseDesktopFiles && item.isDesktopFile()) {
            const KDesktopFile file(item.targetUrl().path());

            if (file.readType() == "Link") {
                return file.readUrl();
            }
        }

        return item.url();
    } else if (role == SizeRole) {
        return m_dirModel->data(mapToSource(QSortFilterProxyModel::index(index.row(), 1)), Qt::DisplayRole);
    } else if (role == TypeRole) {
        return m_dirModel->data(mapToSource(QSortFilterProxyModel::index(index.row(), 6)), Qt::DisplayRole);
    } else if (role == FileNameRole) {
        return itemForIndex(index).url().fileName();
    }

    return QSortFilterProxyModel::data(index, role);
}

int FolderModel::indexForUrl(const QUrl& url) const
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

    if (m_parseDesktopFiles && item.isDesktopFile()) {
        // Check if the desktop file is a link to a directory
        KDesktopFile file(item.targetUrl().path());

        if (file.readType() == "Link") {
            const QUrl url(file.readUrl());

            if (url.isLocalFile()) {
                QT_STATBUF buf;
                const QString path = url.adjusted(QUrl::StripTrailingSlash).toLocalFile();
                if (QT_STAT(QFile::encodeName(path).constData(), &buf) == 0) {
                    return S_ISDIR(buf.st_mode);
                }
            } else if (!m_isDirCache.contains(item.url()) && KProtocolInfo::protocolClass(url.scheme()) == QString(":local")) {
                KIO::StatJob *job = KIO::stat(url, KIO::HideProgressInfo);
                job->setProperty("org.kde.plasma.folder_url", item.url());
                job->setSide(KIO::StatJob::SourceSide);
                job->setDetails(0);
                connect(job, SIGNAL(result(KJob*)), this, SLOT(statResult(KJob*)));
            }
        }
    }

    return false;
}

void FolderModel::statResult(KJob *job)
{
    KIO::StatJob *statJob = static_cast<KIO::StatJob*>(job);

    const QUrl &url = statJob->property("org.kde.plasma.folder_url").value<QUrl>();
    const QModelIndex &idx = index(indexForUrl(url), 0);

    if (idx.isValid()) {
        m_isDirCache[url] = statJob->statResult().isDir();

        emit dataChanged(idx, idx, QVector<int>() << IsDirRole);
    }
}

void FolderModel::evictFromIsDirCache(const KFileItemList& items)
{
    foreach (const KFileItem &item, items) {
        m_isDirCache.remove(item.url());
    }
}

bool FolderModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    const KDirModel *dirModel = static_cast<KDirModel*>(sourceModel());

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
                const QDateTime leftTime = leftItem.time(KFileItem::ModificationTime);
                const QDateTime rightTime = rightItem.time(KFileItem::ModificationTime);
                if (leftTime < rightTime)
                    result = -1;
                else if (leftTime > rightTime)
                    result = +1;

                break;
            }
        case KDirModel::Type:
            result = QString::compare(dirModel->data(left, Qt::DisplayRole).toString(),
                                      dirModel->data(right, Qt::DisplayRole).toString());
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

inline bool FolderModel::matchMimeType(const KFileItem &item) const
{
    if (m_mimeSet.isEmpty()) {
        return false;
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
    if (m_filterMode == NoFilter) {
        return true;
    }

    const KDirModel *dirModel = static_cast<KDirModel*>(sourceModel());
    const KFileItem item = dirModel->itemForIndex(dirModel->index(sourceRow, KDirModel::Name, sourceParent));

    if (m_filterMode == FilterShowMatches) {
        return (matchPattern(item) && matchMimeType(item));
    } else {
        return !(matchPattern(item) && matchMimeType(item));
    }
}

void FolderModel::createActions()
{
    KIO::FileUndoManager *manager = KIO::FileUndoManager::self();

    QAction *cut = KStandardAction::cut(this, SLOT(cut()), this);
    QAction *copy = KStandardAction::copy(this, SLOT(copy()), this);

    QAction *undo = KStandardAction::undo(manager, SLOT(undo()), this);
    undo->setEnabled(manager->undoAvailable());
    undo->setShortcutContext(Qt::WidgetShortcut);
    connect(manager, SIGNAL(undoAvailable(bool)), undo, SLOT(setEnabled(bool)));
    connect(manager, SIGNAL(undoTextChanged(QString)), SLOT(undoTextChanged(QString)));

    QAction *paste = KStandardAction::paste(this, SLOT(paste()), this);
    QAction *pasteTo = KStandardAction::paste(this, SLOT(pasteTo()), this);

    QAction *reload = new QAction(i18n("&Reload"), this);
    connect(reload, SIGNAL(triggered()), SLOT(refresh()));

    QAction *refresh = new QAction(QIcon::fromTheme("view-refresh"), i18n("&Refresh View"), this);
    connect(refresh, SIGNAL(triggered()), SLOT(refresh()));

    QAction *rename = new QAction(QIcon::fromTheme("edit-rename"), i18n("&Rename"), this);
    connect(rename, SIGNAL(triggered()), SIGNAL(requestRename()));

    QAction *trash = new QAction(QIcon::fromTheme("user-trash"), i18n("&Move to Trash"), this);
    connect(trash, SIGNAL(triggered()), SLOT(moveSelectedToTrash()));

    QAction *emptyTrash = new QAction(QIcon::fromTheme("trash-empty"), i18n("&Empty Trash Bin"), this);
    connect(emptyTrash, SIGNAL(triggered()), SLOT(emptyTrashBin()));

    QAction *del = new QAction(QIcon::fromTheme("edit-delete"), i18n("&Delete"), this);
    connect(del, SIGNAL(triggered()), SLOT(deleteSelected()));

    m_actionCollection.addAction("cut", cut);
    m_actionCollection.addAction("undo", undo);
    m_actionCollection.addAction("copy", copy);
    m_actionCollection.addAction("paste", paste);
    m_actionCollection.addAction("pasteto", pasteTo);
    m_actionCollection.addAction("reload", reload);
    m_actionCollection.addAction("refresh", refresh);
    m_actionCollection.addAction("rename", rename);
    m_actionCollection.addAction("trash", trash);
    m_actionCollection.addAction("del", del);
    m_actionCollection.addAction("emptyTrash", emptyTrash);

    m_newMenu = new KNewFileMenu(&m_actionCollection, "newMenu", QApplication::desktop());
    m_newMenu->setModal(false);
}

QAction* FolderModel::action(const QString &name) const
{
    return m_actionCollection.action(name);
}

QObject* FolderModel::newMenu() const
{
    return m_newMenu->menu();
}

void FolderModel::updateActions()
{
    if (m_newMenu) {
        m_newMenu->checkUpToDate();
        m_newMenu->setPopupFiles(m_dirModel->dirLister()->url());
    }

    QAction *emptyTrash = m_actionCollection.action("emptyTrash");

    if (emptyTrash) {
        if (resolvedUrl() == QUrl("trash:/")) {
            emptyTrash->setVisible(true);
            KConfig trashConfig("trashrc", KConfig::SimpleConfig);
            emptyTrash->setEnabled(!trashConfig.group("Status").readEntry("Empty", true));
        } else {
            emptyTrash->setVisible(false);
        }
    }

    QAction *paste = m_actionCollection.action("paste");

    if (paste) {
        bool enable = false;

        const QString pasteText = KIO::pasteActionText(QApplication::clipboard()->mimeData(),
            &enable, m_dirModel->dirLister()->rootItem());

        if (enable) {
            paste->setText(pasteText);
            paste->setEnabled(true);
        } else {
            paste->setText(i18n("&Paste"));
            paste->setEnabled(false);
        }

        QAction* pasteTo = m_actionCollection.action("pasteto");

        if (pasteTo) {
            pasteTo->setEnabled(paste->isEnabled());
            pasteTo->setText(paste->text());
        }
    }
}

void FolderModel::openContextMenu()
{
    QModelIndexList indexes = m_selectionModel->selectedIndexes();

    if (m_usedByContainment && !KAuthorized::authorize("action/kdesktop_rmb")) {
        return;
    }

    updateActions();

    QMenu *menu = 0;

    if (indexes.isEmpty()) {
        menu = new QMenu();
        menu->addAction(m_actionCollection.action("newMenu"));
        menu->addSeparator();
        menu->addAction(m_actionCollection.action("paste"));
        menu->addAction(m_actionCollection.action("undo"));
        menu->addAction(m_actionCollection.action("refresh"));
        menu->addAction(m_actionCollection.action("emptyTrash"));
        menu->addSeparator();

        if (!m_fileItemActions) {
            m_fileItemActions = new KFileItemActions(this);
        }

        KFileItemListProperties itemList(KFileItemList() << m_dirModel->dirLister()->rootItem());
        m_fileItemActions->setItemListProperties(itemList);

        menu->addAction(m_fileItemActions->preferredOpenWithAction(QString()));
    } else {
        KFileItemList items;
        bool hasRemoteFiles = false;
        bool isTrashLink = false;

        foreach (const QModelIndex &index, indexes) {
            KFileItem item = itemForIndex(index);
            if (!item.isNull()) {
                hasRemoteFiles |= item.localPath().isEmpty();
                items.append(item);
            }
        }

        // Check if we're showing the menu for the trash link
        if (items.count() == 1 && items.at(0).isDesktopFile()) {
            KDesktopFile file(items.at(0).localPath());
            if (file.readType() == "Link" && file.readUrl() == "trash:/") {
                isTrashLink = true;
            }
        }

        QList<QAction*> editActions;
        editActions.append(m_actionCollection.action("rename"));

        KSharedConfig::Ptr globalConfig = KSharedConfig::openConfig("kdeglobals", KConfig::NoGlobals);
        KConfigGroup cg(globalConfig, "KDE");
        bool showDeleteCommand = cg.readEntry("ShowDeleteCommand", false);

        // Don't add the "Move to Trash" action if we're showing the menu for the trash link
        if (!isTrashLink) {
            if (!hasRemoteFiles) {
                editActions.append(m_actionCollection.action("trash"));
            } else {
                showDeleteCommand = true;
            }
        }
        if (showDeleteCommand) {
            editActions.append(m_actionCollection.action("del"));
        }

        KParts::BrowserExtension::ActionGroupMap actionGroups;
        actionGroups.insert("editactions", editActions);

        KParts::BrowserExtension::PopupFlags flags = KParts::BrowserExtension::ShowProperties;
        flags |= KParts::BrowserExtension::ShowUrlOperations;

        menu = new KonqPopupMenu(items, m_dirModel->dirLister()->url(), m_actionCollection, m_newMenu,
                                 KonqPopupMenu::ShowNewWindow, flags, 0,
                                 KBookmarkManager::userBookmarksManager(),
                                 actionGroups);
    }

    menu->exec(QCursor::pos());
    delete menu;
}

void FolderModel::linkHere(const QUrl &sourceUrl)
{
    KIO::CopyJob *job = KIO::link(sourceUrl, m_dirModel->dirLister()->url());
    KIO::FileUndoManager::self()->recordCopyJob(job);
}

QList<QUrl> FolderModel::selectedUrls(bool forTrash) const
{
    QList<QUrl> urls;

    foreach (const QModelIndex &index, m_selectionModel->selectedIndexes())
    {
        KFileItem item = itemForIndex(index);

        if (forTrash) {
            // Prefer the local URL if there is one, since we can't trash remote URL's
            const QString path = item.mostLocalUrl().toString();
            if (!path.isEmpty()) {
                urls.append(path);
            } else {
                urls.append(item.url());
            }
        } else {
            urls.append(item.url());
        }
    }
    return urls;
}

void FolderModel::copy()
{
    if (!m_selectionModel->hasSelection()) {
        return;
    }

    QMimeData *mimeData = QSortFilterProxyModel::mimeData(m_selectionModel->selectedIndexes());
    QApplication::clipboard()->setMimeData(mimeData);
}

void FolderModel::cut()
{
    if (!m_selectionModel->hasSelection()) {
        return;
    }

    QMimeData *mimeData = QSortFilterProxyModel::mimeData(m_selectionModel->selectedIndexes());
    KIO::setClipboardDataCut(mimeData, true);
    QApplication::clipboard()->setMimeData(mimeData);
}

void FolderModel::paste()
{
    KIO::paste(QApplication::clipboard()->mimeData(), m_dirModel->dirLister()->url());
}

void FolderModel::pasteTo()
{
    const QList<QUrl> urls = selectedUrls(false);
    Q_ASSERT(urls.count() == 1);
    KIO::paste(QApplication::clipboard()->mimeData(), urls.first());
}

void FolderModel::refresh()
{
    m_dirModel->dirLister()->updateDirectory(m_dirModel->dirLister()->url());
}

void FolderModel::moveSelectedToTrash()
{
    if (!m_selectionModel->hasSelection()) {
        return;
    }

    const QList<QUrl> urls = selectedUrls(true);
    KIO::JobUiDelegate uiDelegate;
    if (uiDelegate.askDeleteConfirmation(urls, KIO::JobUiDelegate::Trash, KIO::JobUiDelegate::DefaultConfirmation)) {
        KIO::Job* job = KIO::trash(urls);
        job->ui()->setAutoErrorHandlingEnabled(true);
    }
}

void FolderModel::deleteSelected()
{
    if (!m_selectionModel->hasSelection()) {
        return;
    }

    const QList<QUrl> urls = selectedUrls(false);
    KIO::JobUiDelegate uiDelegate;
    if (uiDelegate.askDeleteConfirmation(urls, KIO::JobUiDelegate::Delete, KIO::JobUiDelegate::DefaultConfirmation)) {
        KIO::Job* job = KIO::del(urls);
        job->ui()->setAutoErrorHandlingEnabled(true);
    }
}

void FolderModel::emptyTrashBin()
{
    KIO::JobUiDelegate uiDelegate;
    uiDelegate.setWindow(QApplication::desktop());
    if (uiDelegate.askDeleteConfirmation(QList<QUrl>(), KIO::JobUiDelegate::EmptyTrash, KIO::JobUiDelegate::DefaultConfirmation)) {
        KIO::Job* job = KIO::emptyTrash();
        job->ui()->setAutoErrorHandlingEnabled(true);
    }
}

void FolderModel::undoTextChanged(const QString &text)
{
    if (QAction *action = m_actionCollection.action("undo")) {
        action->setText(text);
    }
}
