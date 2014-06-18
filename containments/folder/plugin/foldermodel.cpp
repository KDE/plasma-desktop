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

#include "foldermodel.h"

#include <QApplication>
#include <QClipboard>
#include <QDesktopWidget>
#include <QCollator>
#include <QItemSelectionModel>
#include <qplatformdefs.h>
#include <QDebug>

#include "internallibkonq/konq_popupmenu.h"
#include "internallibkonq/konq_operations.h"
#include "internallibkonq/konqmimedata.h"
#include <KAuthorized>
#include <KBookmarkManager>
#include <KConfigGroup>
#include <KNewFileMenu>
#include <KIO/FileUndoManager>
#include <KIO/Paste>
#include <KLocalizedString>
#include <KSharedConfig>
#include <KShell>
#include <KUrl>

#include <KDesktopFile>
#include <KDirModel>
#include <KIO/CopyJob>
#include <KIO/Job>
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
    m_previewGenerator(0),
    m_viewAdapter(0),
    m_actionCollection(this),
    m_newMenu(0),
    m_usedByContainment(false),
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

    m_dirModel = new KDirModel(this);
    m_dirModel->setDirLister(dirLister);

    m_selectionModel = new QItemSelectionModel(this, this);
    connect(m_selectionModel, SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            this, SLOT(selectionChanged(QItemSelection,QItemSelection)));

    setSourceModel(m_dirModel);

    setSortLocaleAware(true);
    setFilterCaseSensitivity(Qt::CaseInsensitive);
    setDynamicSortFilter(true);

    sort(m_sortMode, m_sortDesc ? Qt::DescendingOrder : Qt::AscendingOrder);

    createActions();
}

FolderModel::~FolderModel()
{
}

QHash< int, QByteArray > FolderModel::roleNames() const
{
    QHash<int, QByteArray> roleNames = m_dirModel->roleNames();
    roleNames[Qt::DisplayRole] = "display";
    roleNames[Qt::DecorationRole] = "decoration";
    roleNames[Qt::UserRole + 1] = "selected";
    roleNames[Qt::UserRole + 2] = "isDir";
    roleNames[Qt::UserRole + 3] = "url";
    roleNames[Qt::UserRole + 4] = "size";
    roleNames[Qt::UserRole + 5] = "type";

    return roleNames;
}

QString FolderModel::url() const
{
    return m_url;
}

void FolderModel::setUrl(const QString& _url)
{
    QUrl url;

    if (_url.startsWith('~')) {
        url = QUrl::fromLocalFile(KShell::tildeExpand(_url));
    } else {
        url = QUrl::fromUserInput(_url);
    }

    if (_url == m_url) {
        m_dirModel->dirLister()->updateDirectory(url);

        return;
    }

    beginResetModel();
    m_url = _url;
    m_dirModel->dirLister()->openUrl(url);
    endResetModel();

    emit urlChanged();

    m_errorString.clear();
    emit errorStringChanged();
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

        emit usedByContainmentChanged();
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

        invalidate();
        sort(m_sortMode, m_sortDesc ? Qt::DescendingOrder : Qt::AscendingOrder);

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

        invalidate();
        sort(m_sortMode, m_sortDesc ? Qt::DescendingOrder : Qt::AscendingOrder);

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

        invalidate();
        sort(m_sortMode, m_sortDesc ? Qt::DescendingOrder : Qt::AscendingOrder);

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
    KFileItem item = itemForIndex(index(row, 0));

    QUrl url(item.targetUrl());
    if (url.scheme().isEmpty()) {
        url.setScheme("file");
    }

    new KRun(url, 0);
}

void FolderModel::rename(int row, const QString& name)
{
    QModelIndex idx = index(row, 0);
    m_dirModel->setData(mapToSource(idx), name, Qt::EditRole);
}

void FolderModel::setSelected(int row)
{
    m_selectionModel->select(index(row, 0), QItemSelectionModel::Select);
}

void FolderModel::setRangeSelected(int startRow, int endRow)
{
    QItemSelection selection(index(startRow, 0), index(endRow, 0));
    m_selectionModel->select(selection, QItemSelectionModel::ClearAndSelect);
}

void FolderModel::toggleSelected(int row)
{
    m_selectionModel->select(index(row, 0), QItemSelectionModel::Toggle);
}

void FolderModel::clearSelection()
{
    m_selectionModel->clear();
}

void FolderModel::selectionChanged(QItemSelection selected, QItemSelection deselected)
{
    QModelIndexList indices = selected.indexes();
    indices.append(deselected.indexes());

    QVector<int> roles;
    roles.append(Qt::UserRole + 1);

    foreach(const QModelIndex index, indices) {
        emit dataChanged(index, index, roles);
    }
}

QVariant FolderModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    if (role == Qt::UserRole + 1) {
        return m_selectionModel->isSelected(index);
    } else if (role == Qt::UserRole + 2) {
        bool moo = isDir(mapToSource(index), m_dirModel);

        return moo;
    } else if (role == Qt::UserRole + 3) {
        KFileItem item = itemForIndex(index);

        return item.url();
    } else if (role == Qt::UserRole + 4) {
        return m_dirModel->data(mapToSource(QSortFilterProxyModel::index(index.row(), 1)), Qt::DisplayRole);
    }
    else if (role == Qt::UserRole + 5) {
        return m_dirModel->data(mapToSource(QSortFilterProxyModel::index(index.row(), 6)), Qt::DisplayRole);
    }

    return QSortFilterProxyModel::data(index, role);
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
            }
        }
    }

    return false;
}

bool FolderModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    const KDirModel *dirModel = static_cast<KDirModel*>(sourceModel());

    // When sorting by size, folders are compared using the number of items in them,
    // so they need to be given precedence over regular files as the comparison criteria is different
    if (m_sortDirsFirst || left.column() == KDirModel::Size) {
        bool leftIsDir = isDir(left, dirModel);
        bool rightIsDir = isDir(right, dirModel);
        if (leftIsDir && !rightIsDir) {
            return (sortOrder() == Qt::AscendingOrder); // folders > files independent of the sorting order
        }
        if (!leftIsDir && rightIsDir) {
            return (sortOrder() == Qt::DescendingOrder); // same here
        }
    }

    const KFileItem leftItem = dirModel->data(left, KDirModel::FileItemRole).value<KFileItem>();
    const KFileItem rightItem = dirModel->data(right, KDirModel::FileItemRole).value<KFileItem>();
    const int column = left.column();
    int result = 0;

    switch (column) {
      case KDirModel::Name:
            // fall through to the naturalCompare call
            break;
        case KDirModel::ModifiedTime: {
            const QDateTime leftTime = leftItem.time(KFileItem::ModificationTime);
            const QDateTime rightTime = rightItem.time(KFileItem::ModificationTime);
            if (leftTime < rightTime)
                result = -1;
            else if (leftTime > rightTime)
                result = +1;
            break;
            }
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
        case KDirModel::Type:
            // add other sorting modes here
            // KDirModel::data(index, Qt::DisplayRole) returns the data in index.column()
            result = QString::compare(dirModel->data(left, Qt::DisplayRole).toString(),
                                      dirModel->data(right, Qt::DisplayRole).toString());
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
    bool m_usedByContainment = true;

    KIO::FileUndoManager *manager = KIO::FileUndoManager::self();

    QAction *cut = KStandardAction::cut(this, SLOT(cut()), this);
    QAction *copy = KStandardAction::copy(this, SLOT(copy()), this);
    QAction *undo = KStandardAction::undo(manager, SLOT(undo()), this);
    undo->setEnabled(manager->undoAvailable());
    undo->setShortcutContext(Qt::WidgetShortcut);
    connect(manager, SIGNAL(undoAvailable(bool)), undo, SLOT(setEnabled(bool)));
    connect(manager, SIGNAL(undoTextChanged(QString)), SLOT(undoTextChanged(QString)));

    QAction *paste = KStandardAction::paste(this, SLOT(paste()), this);

    QString actionText = KIO::pasteActionText();
    if (!actionText.isEmpty()) {
       paste->setText(actionText);
    } else {
       paste->setEnabled(false);
    }

    QAction *pasteTo = KStandardAction::paste(this, SLOT(pasteTo()), this);

    QAction *reload = new QAction(i18n("&Reload"), this);
    connect(reload, SIGNAL(triggered()), SLOT(refresh()));

    QAction *refresh = new QAction(m_usedByContainment ? i18n("&Refresh Desktop") : i18n("&Refresh View"), this);
    if (m_usedByContainment) {
        refresh->setIcon(QIcon("user-desktop"));
    }
    connect(refresh, SIGNAL(triggered()), SLOT(refresh()));

    QAction *rename = new QAction(QIcon("edit-rename"), i18n("&Rename"), this);
    connect(rename, SIGNAL(triggered()), SIGNAL(requestRename()));

    QAction *trash = new QAction(QIcon("user-trash"), i18n("&Move to Trash"), this);
    connect(trash, SIGNAL(triggered()), SLOT(moveSelectedToTrash()));

    QAction *emptyTrash = new QAction(QIcon("trash-empty"), i18n("&Empty Trash Bin"), this);
    KConfig trashConfig("trashrc", KConfig::SimpleConfig);
    emptyTrash->setEnabled(!trashConfig.group("Status").readEntry("Empty", true));
    connect(emptyTrash, SIGNAL(triggered()), SLOT(emptyTrashBin()));

    QAction *del = new QAction(i18n("&Delete"), this);
    del->setIcon(QIcon("edit-delete"));
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
    m_actionCollection.addAction("empty_trash", emptyTrash);

    m_newMenu = new KNewFileMenu(&m_actionCollection, "new_menu", QApplication::desktop());
    m_newMenu->setModal(false);

    connect(m_newMenu->menu(), SIGNAL(aboutToShow()), this, SLOT(aboutToShowCreateNew()));
}

void FolderModel::openContextMenu()
{
    QModelIndexList indexes = m_selectionModel->selectedIndexes();

    if (!KAuthorized::authorize("action/kdesktop_rmb") || indexes.isEmpty()) {
        return;
    }

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

    QAction* pasteTo = m_actionCollection.action("pasteto");
    if (pasteTo) {
        if (QAction *paste = m_actionCollection.action("paste")) {
            updatePasteAction();
            pasteTo->setEnabled(paste->isEnabled());
            pasteTo->setText(paste->text());
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

    // m_newMenu can be NULL here but KonqPopupMenu does handle this.
    KonqPopupMenu *contextMenu = new KonqPopupMenu(items, m_dirModel->dirLister()->url(), m_actionCollection, m_newMenu,
                                                   KonqPopupMenu::ShowNewWindow, flags, 0,
                                                   KBookmarkManager::userBookmarksManager(),
                                                   actionGroups);

    contextMenu->exec(QCursor::pos());
    delete contextMenu;

    if (pasteTo) {
        pasteTo->setEnabled(false);
    }
}

void FolderModel::linkHere(const QUrl &sourceUrl)
{
    KIO::CopyJob *job = KIO::link(sourceUrl, m_dirModel->dirLister()->url());
    KIO::FileUndoManager::self()->recordCopyJob(job);
}


void FolderModel::updatePasteAction()
{
    if (QAction *paste = m_actionCollection.action("paste")) {
        const QString pasteText = KIO::pasteActionText();
        if (pasteText.isEmpty()) {
            paste->setText(i18n("&Paste"));
            paste->setEnabled(false);
        } else {
            paste->setText(pasteText);
            paste->setEnabled(true);
        }
    }
}

void FolderModel::aboutToShowCreateNew()
{
    if (m_newMenu) {
        m_newMenu->checkUpToDate();
        m_newMenu->setPopupFiles(m_dirModel->dirLister()->url());
    }
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
    QMimeData *mimeData = QSortFilterProxyModel::mimeData(m_selectionModel->selectedIndexes());
    QApplication::clipboard()->setMimeData(mimeData);
}

void FolderModel::cut()
{
    QMimeData *mimeData = QSortFilterProxyModel::mimeData(m_selectionModel->selectedIndexes());
    KonqMimeData::addIsCutSelection(mimeData, true);
    QApplication::clipboard()->setMimeData(mimeData);
}

void FolderModel::paste()
{
    KonqOperations::doPaste(QApplication::desktop(), m_dirModel->dirLister()->url());
}

void FolderModel::pasteTo()
{
    const QList<QUrl> urls = selectedUrls(false);
    Q_ASSERT(urls.count() == 1);
    KonqOperations::doPaste(QApplication::desktop(), urls.first());
}

void FolderModel::refresh()
{
    m_dirModel->dirLister()->updateDirectory(m_dirModel->dirLister()->url());
}

void FolderModel::moveSelectedToTrash()
{
    KonqOperations::del(QApplication::desktop(), KonqOperations::TRASH, selectedUrls(true));
}

void FolderModel::deleteSelected()
{
    KonqOperations::del(QApplication::desktop(), KonqOperations::DEL, selectedUrls(false));
}

void FolderModel::emptyTrashBin()
{
    KonqOperations::emptyTrash(QApplication::desktop());
}

void FolderModel::undoTextChanged(const QString &text)
{
    if (QAction *action = m_actionCollection.action("undo")) {
        action->setText(text);
    }
}
