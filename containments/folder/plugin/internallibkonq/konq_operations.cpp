/*  This file is part of the KDE project
    Copyright 2000-2007  David Faure <faure@kde.org>
    Copyright 2003       Waldo Bastian <bastian@kde.org>
    Copyright 2002       Michael Brade <brade@kde.org>
    Copyright 2001-2002  Alexander Neundorf <neundorf@kde.org>
    Copyright 2000-2001  Simon Hausmann <hausmann@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) version 3.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "konq_operations.h"
#include "konq_dndpopupmenuplugin.h"

#include <ktoolinvocation.h>
#include <kautomount.h>
#include <kmountpoint.h>
#include <QInputDialog>
#include <kmessagebox.h>
#include <knotification.h>
#include <krun.h>
#include <kshell.h>
#include <kprocess.h>
#include <kshortcut.h>
#include <kprotocolmanager.h>
#include <kio/deletejob.h>
#include <kio/fileundomanager.h>
#include <kio/job.h>
#include <kio/jobuidelegate.h>
#include <kio/jobclasses.h>
#include <kio/copyjob.h>
#include <kio/mkpathjob.h>
#include <kio/paste.h>
#include <kio/renamedialog.h>
#include <KJobWidgets>
#include <kdirnotify.h>
#include <kuiserverjobtracker.h>
#include <kstandarddirs.h>
// For doDrop
#include <QIcon>
#include <kauthorized.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <kdebug.h>
#include <kfileitem.h>
#include <kdesktopfile.h>
#include <KUrlMimeData>

//for _addPluginActions
#include <kfileitemlistproperties.h>
#include <kservice.h>
#include <kmimetypetrader.h>

//#include <konq_iconviewwidget.h>
#include <QMenu>
#include <QApplication>
#include <QClipboard>
#include <QDropEvent>
#include <QList>
#include <QDir>
#include <QTimer>
#include <QStandardPaths>

#include <assert.h>
#include <unistd.h>
#include <kconfiggroup.h>

KonqOperations::KonqOperations( QWidget *parent )
    : QObject( parent ),
      m_method( UNKNOWN ), m_info(0)
{
    setObjectName( QLatin1String( "KonqOperations" ) );
}

KonqOperations::~KonqOperations()
{
    delete m_info;
}

KonqOperations *KonqOperations::doDrop( const KFileItem & destItem, const QUrl & dest, QDropEvent * ev, QWidget * parent,
                                        const QList<QAction*> & userActions )
{
    kDebug(1203) << "dest:" << dest;
    QMap<QString, QString> metaData;
    // Prefer local urls if possible, to avoid problems with desktop:/ urls from other users (#184403)
    const QList<QUrl> lst = KUrlMimeData::urlsFromMimeData(ev->mimeData(), KUrlMimeData::PreferLocalUrls, &metaData);
    if (!lst.isEmpty()) { // Are they urls ?
        //kDebug(1203) << "metaData:" << metaData.count() << "entries.";
        //QMap<QString,QString>::ConstIterator mit;
        //for( mit = metaData.begin(); mit != metaData.end(); ++mit ) {
        //    kDebug(1203) << "metaData: key=" << mit.key() << "value=" << mit.value();
        //}
        // Check if we dropped something on itself
        QList<QUrl>::ConstIterator it = lst.constBegin();
        for (; it != lst.constEnd() ; it++) {
            kDebug(1203) << "URL:" << (*it);
            if (dest.matches(*it, QUrl::StripTrailingSlash)) {
                // The event source may be the view or an item (icon)
                // Note: ev->source() can be 0L! (in case of kdesktop) (Simon)
                if ( !ev->source() || ( ev->source() != parent && ev->source()->parent() != parent ) )
                    KMessageBox::sorry( parent, i18n("You cannot drop a folder on to itself") );
                kDebug(1203) << "Dropped on itself";
                ev->setAccepted( false );
                return 0; // do nothing instead of displaying kfm's annoying error box
            }
        }

        // Check the state of the modifiers key at the time of the drop
        Qt::KeyboardModifiers modifiers = QApplication::keyboardModifiers();

        Qt::DropAction action = ev->dropAction();
        // Check for the drop of a bookmark -> we want a Link action
        if ( ev->mimeData()->formats().contains("application/x-xbel") )
        {
            modifiers |= Qt::ControlModifier | Qt::ShiftModifier;
            action = Qt::LinkAction;
            kDebug(1203) << "Bookmark -> emulating Link";
        }

        KonqOperations * op = new KonqOperations(parent);
        op->setDropInfo( new DropInfo( modifiers, lst, metaData, QCursor::pos(), action, userActions ) );

        // Ok, now we need destItem.
        if ( !destItem.isNull() )
        {
            // We have it already, we could just call asyncDrop.
            // But popping up a menu in the middle of a DND operation confuses and crashes Qt (#157630)
            // So let's delay it.
            qRegisterMetaType<KFileItem>("KFileItem");
            QMetaObject::invokeMethod(op, "asyncDrop", Qt::QueuedConnection, Q_ARG(KFileItem, destItem));
        }
        else
        {
            // we need to stat to get it.
            KIO::StatJob *job = KIO::stat(dest);
            KJobWidgets::setWindow(job, parent);
            connect(job, &KIO::StatJob::result, op, &KonqOperations::slotStatResult);

        }
        // In both cases asyncDrop will delete op when done

        ev->acceptProposedAction();
        return op;
    }
    else
    {
        //kDebug(1203) << "Pasting to " << dest.url();
        KonqOperations * op = new KonqOperations(parent);
        KIO::Job* job = KIO::pasteMimeData(ev->mimeData(), dest,
                                           i18n( "File name for dropped contents:" ),
                                           parent);
        if (KIO::SimpleJob* simpleJob = qobject_cast<KIO::SimpleJob *>(job)) {
            op->setOperation(job, PUT, simpleJob->url());
            KIO::FileUndoManager::self()->recordJob(KIO::FileUndoManager::Put, QList<QUrl>(), simpleJob->url(), simpleJob);
        }
        ev->acceptProposedAction();
        return op;
    }
}

void KonqOperations::asyncDrop( const KFileItem & destItem )
{
    assert(m_info); // setDropInfo should have been called before asyncDrop
    bool m_destIsLocal = false;
    m_destUrl = destItem.mostLocalUrl(m_destIsLocal); // #168154

    //kDebug(1203) << "destItem->mode=" << destItem->mode() << " url=" << m_destUrl;
    // Check what the destination is
    if ( destItem.isDir() )
    {
        doDropFileCopy();
        return;
    }
    if ( !m_destIsLocal )
    {
        // We dropped onto a remote URL that is not a directory!
        // (e.g. an HTTP link in the sidebar).
        // Can't do that, but we can't prevent it before stating the dest....
        kWarning(1203) << "Cannot drop onto " << m_destUrl ;
        deleteLater();
        return;
    }
    if ( destItem.isDesktopFile() )
    {
        // Local .desktop file. What type ?
        KDesktopFile desktopFile( m_destUrl.path() );
        KConfigGroup desktopGroup = desktopFile.desktopGroup();
        if ( desktopFile.hasApplicationType() )
        {
            QString error;
            QStringList urlStrList;
            foreach (const QUrl & url, m_info->urls) {
                urlStrList << url.url();
            }

            if ( KToolInvocation::startServiceByDesktopPath( m_destUrl.path(), urlStrList, &error ) > 0 )
                KMessageBox::error( parentWidget(), error );
        }
        else
        {
            // Device or Link -> adjust dest
            if ( desktopFile.hasDeviceType() && desktopGroup.hasKey("MountPoint") ) {
                QString point = desktopGroup.readEntry( "MountPoint" );
                m_destUrl.setPath( point );
                QString dev = desktopFile.readDevice();
                KMountPoint::Ptr mp = KMountPoint::currentMountPoints().findByDevice( dev );
                // Is the device already mounted ?
                if ( mp ) {
                    doDropFileCopy();
                }
#ifndef Q_OS_WIN
                else
                {
                    const bool ro = desktopGroup.readEntry( "ReadOnly", false );
                    const QByteArray fstype = desktopGroup.readEntry( "FSType" ).toLatin1();
                    KAutoMount* am = new KAutoMount( ro, fstype, dev, point, m_destUrl.path(), false );
                    connect( am, &KAutoMount::finished, this, &KonqOperations::doDropFileCopy );
                }
#endif
                return;
            }
            else if ( desktopFile.hasLinkType() && desktopGroup.hasKey("URL") ) {
                m_destUrl = QUrl::fromUserInput(desktopGroup.readPathEntry("URL", QString()));
                doDropFileCopy();
                return;
            }
            // else, well: mimetype, service, servicetype or .directory. Can't really drop anything on those.
        }
    }
    else
    {
        // Should be a local executable
        // (If this fails, there is a bug in KFileItem::acceptsDrops / KDirModel::flags)
        kDebug(1203) << m_destUrl.path() << "should be an executable";
        Q_ASSERT ( access( QFile::encodeName(m_destUrl.path()), X_OK ) == 0 );
        // Launch executable for each of the files
        QStringList args;
        const QList<QUrl> lst = m_info->urls;
        QList<QUrl>::ConstIterator it = lst.constBegin();
        for ( ; it != lst.constEnd() ; it++ )
            args << (*it).path(); // assume local files
        kDebug(1203) << "starting " << m_destUrl.path() << " with " << lst.count() << " arguments";
        KProcess::startDetached( m_destUrl.path(), args );
    }
    deleteLater();
}

void KonqOperations::doDropFileCopy()
{
    assert(m_info); // setDropInfo - and asyncDrop - should have been called before asyncDrop
    const QList<QUrl> lst = m_info->urls;
    Qt::DropAction action = m_info->action;
    bool isDesktopFile = false;
    bool itemIsOnDesktop = false;
    bool allItemsAreFromTrash = true;
    QList<QUrl> mlst; // list of items that can be moved
    for (QList<QUrl>::ConstIterator it = lst.constBegin(); it != lst.constEnd(); ++it)
    {
        const QUrl url = (*it);
        const bool local = url.isLocalFile();
        if ( KProtocolManager::supportsDeleting(url) &&
             (!local || QFileInfo(QFileInfo(url.adjusted(QUrl::RemoveFilename).path()).absolutePath()).isWritable() ))
            mlst.append(url);
        if ( local && KDesktopFile::isDesktopFile(url.toLocalFile()))
            isDesktopFile = true;
        if ( local && url.path().startsWith(QStandardPaths::writableLocation(QStandardPaths::DesktopLocation)))
            itemIsOnDesktop = true;
        if ( local || url.scheme() != "trash" )
            allItemsAreFromTrash = false;
    }

    bool linkOnly = false; // if true, we'll show a popup menu, but with only "link" in it (for confirmation)
    if (isDesktopFile && !KAuthorized::authorizeKAction("run_desktop_files") &&
            (m_destUrl.toString(QUrl::PreferLocalFile) == QStandardPaths::writableLocation(QStandardPaths::DesktopLocation)) ) {
        linkOnly = true;
    } else if ( allItemsAreFromTrash && lst.first().path() == "/" ) {
        // Dropping a link to the trash: don't move the full contents, just make a link (#319660)
        linkOnly = true;
    }

    if ( !mlst.isEmpty() && m_destUrl.scheme() == "trash" )
    {
        if ( itemIsOnDesktop && !KAuthorized::authorizeKAction("editable_desktop_icons") )
        {
            deleteLater();
            return;
        }

        m_method = TRASH;
        KIO::JobUiDelegate uiDelegate;
        uiDelegate.setWindow(parentWidget());
        if (uiDelegate.askDeleteConfirmation(mlst, KIO::JobUiDelegate::Trash, KIO::JobUiDelegate::DefaultConfirmation)) {
            action = Qt::MoveAction;
        } else {
            deleteLater();
            return;
        }
    } else if (!linkOnly && (allItemsAreFromTrash || m_destUrl.scheme() == "trash")) {
        // No point in asking copy/move/link when using dnd from or to the trash.
        action = Qt::MoveAction;
    }
    else if ( (
        ((m_info->keyboardModifiers & Qt::ControlModifier) == 0) &&
        ((m_info->keyboardModifiers & Qt::ShiftModifier) == 0) &&
        ((m_info->keyboardModifiers & Qt::AltModifier) == 0) ) || linkOnly )
    {
        // Neither control, shift or alt are pressed => show popup menu

        // Check what the source can do
        // we'll assume it's the same for all URLs (hack)
        // TODO: if we had a KFileItemList instead of a KUrl::List,
        // we could use KFileItemsCapabilities
        const QUrl url = lst.first();
        bool sReading = KProtocolManager::supportsReading( url );
        bool sDeleting = KProtocolManager::supportsDeleting( url );
        bool sMoving = KProtocolManager::supportsMoving( url );
        // Check what the destination can do
        bool dWriting = KProtocolManager::supportsWriting( m_destUrl );
        if ( !dWriting )
        {
            deleteLater();
            return;
        }

        bool enableLinking = true;			// for now, but see below

        // We don't want to offer "move" for temp files. They might come from
        // kmail using a tempfile for attachments, or ark using a tempdir for
        // extracting an archive -- in all cases, we can't implement a real move,
        // it's just a copy of the tempfile [and the source app will delete it later].
        // https://www.intevation.de/roundup/kolab/issue2026
        //
        // Similarly, linking to a temp file is pointless.
        if (url.isLocalFile() && url.toLocalFile().startsWith(QStandardPaths::standardLocations(QStandardPaths::TempLocation).at(0))) {
            sMoving = false;
            sDeleting = false;
            enableLinking = false;
        }

        QMenu popup;
        QString seq = QKeySequence( Qt::ShiftModifier ).toString();
        seq.chop(1); // chop superfluous '+'
        QAction* popupMoveAction = new QAction(i18n( "&Move Here" ) + '\t' + seq, this);
        popupMoveAction->setIcon(QIcon::fromTheme("go-jump"));
        seq = QKeySequence( Qt::ControlModifier ).toString();
        seq.chop(1);
        QAction* popupCopyAction = new QAction(i18n( "&Copy Here" ) + '\t' + seq, this);
        popupCopyAction->setIcon(QIcon::fromTheme("edit-copy"));
        seq = QKeySequence( Qt::ControlModifier + Qt::ShiftModifier ).toString();
        seq.chop(1);
        QAction* popupLinkAction = new QAction(i18n( "&Link Here" ) + '\t' + seq, this);
        popupLinkAction->setIcon(QIcon::fromTheme("edit-link"));
        QAction* popupCancelAction = new QAction(i18n( "C&ancel" ) + '\t' + QKeySequence( Qt::Key_Escape ).toString(), this);
        popupCancelAction->setIcon(QIcon::fromTheme("process-stop"));

        if (!mlst.isEmpty() && (sMoving || (sReading && sDeleting)) && !linkOnly )
        {
            bool equalDestination = true;
            foreach ( const QUrl & src, lst )
            {
                const bool equalProtocol = ( m_destUrl.scheme() == src.scheme() );
                if ( !equalProtocol || !m_destUrl.matches(src, QUrl::StripTrailingSlash) )
                {
                    equalDestination = false;
                    break;
                }
            }

            if ( !equalDestination )
                popup.addAction(popupMoveAction);
        }

        if ( sReading && !linkOnly)
            popup.addAction(popupCopyAction);

        if ( enableLinking )
            popup.addAction(popupLinkAction);

        //now initialize the drop plugins
        KFileItemList fileItems;
        foreach(const QUrl& url, lst) {
            fileItems.append(KFileItem(
                        KFileItem::Unknown,
                        KFileItem::Unknown,
                        url));

        }

        QList<QAction*> pluginActions;
        KFileItemListProperties info(fileItems);
        _addPluginActions(pluginActions, m_destUrl, info);

        if (!m_info->userActions.isEmpty() || !pluginActions.isEmpty()) {
            popup.addSeparator();
            popup.addActions(m_info->userActions);
            popup.addActions(pluginActions);
        }

        popup.addSeparator();
        popup.addAction(popupCancelAction);

        QAction* result = popup.exec( m_info->mousePos );

        if(result == popupCopyAction)
            action = Qt::CopyAction;
        else if(result == popupMoveAction)
            action = Qt::MoveAction;
        else if(result == popupLinkAction)
            action = Qt::LinkAction;
        else {
            deleteLater();
            return;
        }
    }

    KIO::CopyJob * job = 0;
    switch ( action ) {
    case Qt::MoveAction :
        job = KIO::move( lst, m_destUrl );
        job->setMetaData( m_info->metaData );
        setOperation( job, m_method == TRASH ? TRASH : MOVE, m_destUrl );
        KIO::FileUndoManager::self()->recordJob(
            m_method == TRASH ? KIO::FileUndoManager::Trash : KIO::FileUndoManager::Move,
            lst, m_destUrl, job );
        break;
    case Qt::CopyAction :
        job = KIO::copy( lst, m_destUrl );
        job->setMetaData( m_info->metaData );
        setOperation( job, COPY, m_destUrl );
        KIO::FileUndoManager::self()->recordCopyJob(job);
        break;
    case Qt::LinkAction :
        kDebug(1203) << "lst.count=" << lst.count();
        job = KIO::link( lst, m_destUrl );
        job->setMetaData( m_info->metaData );
        setOperation( job, LINK, m_destUrl );
        KIO::FileUndoManager::self()->recordCopyJob(job);
        break;
    default : kError(1203) << "Unknown action " << (int)action << endl;
    }
    if (job) {
        connect(job, &KIO::CopyJob::copyingDone, this, &KonqOperations::slotCopyingDone);
        connect(job, &KIO::CopyJob::copyingLinkDone, this, &KonqOperations::slotCopyingLinkDone);
        return; // we still have stuff to do -> don't delete ourselves
    }
    deleteLater();
}

void KonqOperations::slotCopyingDone(KIO::Job*, const QUrl &, const QUrl &to)
{
    m_createdUrls << to;
}

void KonqOperations::slotCopyingLinkDone(KIO::Job*, const QUrl&, const QString&, const QUrl &to)
{
    m_createdUrls << to;
}

void KonqOperations::_addPluginActions(QList<QAction*>& pluginActions, const QUrl &destination, const KFileItemListProperties& info)
{
    kDebug(1203);
    const QString commonMimeType = info.mimeType();
    kDebug() << commonMimeType;
    const KService::List plugin_offers = KMimeTypeTrader::self()->query(commonMimeType.isEmpty() ? QLatin1String("application/octet-stream") : commonMimeType, "KonqDndPopupMenu/Plugin", "exist Library");

    KService::List::ConstIterator iterator = plugin_offers.begin();
    const KService::List::ConstIterator end = plugin_offers.end();
    for(; iterator != end; ++iterator) {
        //kDebug() << (*iterator)->name() << (*iterator)->library();
        KonqDndPopupMenuPlugin *plugin = (*iterator)->createInstance<KonqDndPopupMenuPlugin>(this);
        if (!plugin)
            continue;
        plugin->setup(info, destination, pluginActions);
    }
}

void KonqOperations::setOperation( KIO::Job * job, Operation method, const QUrl & dest )
{
    m_method = method;
    m_destUrl = dest;
    if ( job )
    {
        KJobWidgets::setWindow(job, parentWidget());
        connect( job, &KIO::Job::result, this, &KonqOperations::slotResult );
    }
    else // for link
        slotResult( 0L );
}

void KonqOperations::slotStatResult(KJob * job)
{
    KIO::StatJob * statJob = static_cast<KIO::StatJob*>(job);
    if (statJob->error()) {
        statJob->ui()->showErrorMessage();
    } else {
        KFileItem item(statJob->statResult(), statJob->url());
        asyncDrop(item);
    }
}

void KonqOperations::slotResult(KJob *job)
{
    bool jobFailed = false;
    if (job && job->error()) {
        static_cast<KIO::Job*>(job)->ui()->showErrorMessage();
        jobFailed = true;
    }

    switch (m_method) {
    case PUT: {
            KIO::SimpleJob *simpleJob = qobject_cast<KIO::SimpleJob*>(job);
            if (simpleJob && !jobFailed) {
                m_createdUrls << simpleJob->url();
            }
        }
        break;
    default:
        break;
    }

    if (!m_createdUrls.isEmpty()) {
        // Inform the application about all created urls
        emit aboutToCreate(m_createdUrls);
        m_createdUrls.clear();
    }

    deleteLater();
}

QWidget* KonqOperations::parentWidget() const
{
    return static_cast<QWidget *>( parent() );
}

