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
#include "konqmimedata.h"

#include <ktoolinvocation.h>
#include <kautomount.h>
#include <kmountpoint.h>
#include <kinputdialog.h>
#include <klocale.h>
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
#include <kio/paste.h>
#include <kio/renamedialog.h>
#include <KJobWidgets>
#include <kdirnotify.h>
#include <kuiserverjobtracker.h>
#include <kstandarddirs.h>
// For doDrop
#include <kicon.h>
#include <kauthorized.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <kdebug.h>
#include <kfileitem.h>
#include <kdesktopfile.h>

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

#include <assert.h>
#include <unistd.h>
#include <kconfiggroup.h>

static const KCatalogLoader loader("libkonq");

KonqOperations::KonqOperations( QWidget *parent )
    : QObject( parent ),
      m_method( UNKNOWN ), m_info(0), m_pasteInfo(0)
{
    setObjectName( QLatin1String( "KonqOperations" ) );
}

KonqOperations::~KonqOperations()
{
    delete m_info;
    delete m_pasteInfo;
}

void KonqOperations::editMimeType( const QString & mimeType, QWidget* parent )
{
    QString keditfiletype = QLatin1String("keditfiletype");
    KRun::runCommand( keditfiletype
                      + " --parent " + QString::number( (qptrdiff)parent->winId())
                      + ' ' + KShell::quoteArg(mimeType),
                      keditfiletype, keditfiletype /*unused*/, parent );
}

void KonqOperations::del( QWidget * parent, Operation method, const KUrl::List & selectedUrls )
{
    kDebug(1203) << parent->metaObject()->className();
    if ( selectedUrls.isEmpty() )
    {
        kWarning(1203) << "Empty URL list !" ;
        return;
    }

    KonqOperations * op = new KonqOperations( parent );
    ConfirmationType confirmation = DEFAULT_CONFIRMATION;
    op->_del( method, selectedUrls, confirmation );
}

void KonqOperations::emptyTrash( QWidget* parent )
{
    KonqOperations *op = new KonqOperations( parent );
    op->_del( EMPTYTRASH, KUrl("trash:/"), DEFAULT_CONFIRMATION );
}

void KonqOperations::restoreTrashedItems( const KUrl::List& urls, QWidget* parent )
{
    KonqOperations *op = new KonqOperations( parent );
    op->_restoreTrashedItems( urls );
}

KIO::SimpleJob* KonqOperations::mkdir( QWidget *parent, const KUrl & url )
{
    KIO::SimpleJob * job = KIO::mkdir(url);
    KJobWidgets::setWindow(job, parent);
    job->ui()->setAutoErrorHandlingEnabled(true);
    KIO::FileUndoManager::self()->recordJob( KIO::FileUndoManager::Mkdir, QList<QUrl>(), url, job );
    return job;
}

void KonqOperations::doPaste( QWidget * parent, const KUrl & destUrl, const QPoint &pos )
{
    (void) KonqOperations::doPasteV2( parent, destUrl, pos );
}

KonqOperations *KonqOperations::doPasteV2(QWidget *parent, const KUrl &destUrl, const QPoint &pos)
{
    QClipboard *clipboard = QApplication::clipboard();
    const QMimeData *data = clipboard->mimeData();
    const bool move = KonqMimeData::decodeIsCutSelection(data);

    KIO::Job *job = KIO::pasteClipboard(destUrl, parent, move);
    if (job) {
        KonqOperations *op = new KonqOperations(parent);
        KIOPasteInfo *pi = new KIOPasteInfo;
        pi->mousePos = pos;
        op->setPasteInfo(pi);
        KIO::CopyJob *copyJob = qobject_cast<KIO::CopyJob*>(job);
        if (copyJob) {
            op->setOperation(job, move ? MOVE : COPY, copyJob->destUrl());
            KIO::FileUndoManager::self()->recordJob(move ? KIO::FileUndoManager::Move : KIO::FileUndoManager::Copy, KUrl::List(), destUrl, job);
            connect(copyJob, SIGNAL(copyingDone(KIO::Job*,KUrl,KUrl,time_t,bool,bool)),
                    op, SLOT(slotCopyingDone(KIO::Job*,KUrl,KUrl)));
            connect(copyJob, SIGNAL(copyingLinkDone(KIO::Job*,KUrl,QString,KUrl)),
                    op, SLOT(slotCopyingLinkDone(KIO::Job*,KUrl,QString,KUrl)));
        } else if (KIO::SimpleJob *simpleJob = qobject_cast<KIO::SimpleJob*>(job)) {
            op->setOperation(job, PUT, simpleJob->url());
            KIO::FileUndoManager::self()->recordJob(KIO::FileUndoManager::Put, KUrl::List(), simpleJob->url(), job);
        }
        return op;
    }

    return 0;
}

void KonqOperations::copy( QWidget * parent, Operation method, const KUrl::List & selectedUrls, const KUrl& destUrl )
{
    kDebug(1203) << parent->metaObject()->className() << selectedUrls << destUrl;
    if ((method!=COPY) && (method!=MOVE) && (method!=LINK))
    {
        kWarning(1203) << "Illegal copy method !" ;
        return;
    }
    if ( selectedUrls.isEmpty() )
    {
        kWarning(1203) << "Empty URL list !" ;
        return;
    }

    KonqOperations * op = new KonqOperations( parent );
    KIO::CopyJob* job;
    if (method == LINK)
        job = KIO::link( selectedUrls, destUrl );
    else if (method == MOVE)
        job = KIO::move( selectedUrls, destUrl );
    else
        job = KIO::copy( selectedUrls, destUrl );

    connect(job, SIGNAL(copyingDone(KIO::Job*,KUrl,KUrl,time_t,bool,bool)),
            op, SLOT(slotCopyingDone(KIO::Job*,KUrl,KUrl)));
    connect(job, SIGNAL(copyingLinkDone(KIO::Job*,KUrl,QString,KUrl)),
            op, SLOT(slotCopyingLinkDone(KIO::Job*,KUrl,QString,KUrl)));

    op->setOperation( job, method, destUrl );

    KIO::FileUndoManager::self()->recordCopyJob(job);
}

void KonqOperations::_del( Operation method, const KUrl::List & _selectedUrls, ConfirmationType confirmation )
{
    KUrl::List selectedUrls;
    for (KUrl::List::ConstIterator it = _selectedUrls.begin(); it != _selectedUrls.end(); ++it)
        if (KProtocolManager::supportsDeleting(*it))
            selectedUrls.append(*it);
    if (selectedUrls.isEmpty()) {
        delete this; // this one is ok, _del is always called directly
        return;
    }

    if ( confirmation == SKIP_CONFIRMATION || askDeleteConfirmation( selectedUrls, method, confirmation, parentWidget() ) )
    {
        //m_srcUrls = selectedUrls;
        KIO::Job *job;
        m_method = method;
        switch( method )
        {
        case TRASH:
        {
            job = KIO::trash( selectedUrls );
            KIO::FileUndoManager::self()->recordJob( KIO::FileUndoManager::Trash, selectedUrls, KUrl("trash:/"), job );
            break;
        }
        case EMPTYTRASH:
        {
            // Same as in ktrash --empty
            QByteArray packedArgs;
            QDataStream stream( &packedArgs, QIODevice::WriteOnly );
            stream << (int)1;
            job = KIO::special( KUrl("trash:/"), packedArgs );
            KNotification::event("Trash: emptied", QString() , QPixmap() , 0l, KNotification::DefaultEvent );
            break;
        }
        case DEL:
            job = KIO::del( selectedUrls );
            break;
        default:
            kWarning() << "Unknown operation: " << method ;
            delete this; // this one is ok, _del is always called directly
            return;
        }
        KJobWidgets::setWindow(job, parentWidget());
        connect( job, SIGNAL(result(KJob*)),
                 SLOT(slotResult(KJob*)) );
    } else {
        delete this; // this one is ok, _del is always called directly
    }
}

void KonqOperations::_restoreTrashedItems( const KUrl::List& urls )
{
    m_method = RESTORE;
    KonqMultiRestoreJob* job = new KonqMultiRestoreJob( urls );
    KJobWidgets::setWindow(job, parentWidget());
    KIO::getJobTracker()->registerJob(job);
    connect( job, SIGNAL(result(KJob*)),
             SLOT(slotResult(KJob*)) );
}

bool KonqOperations::askDeleteConfirmation( const KUrl::List & selectedUrls, int method, ConfirmationType confirmation, QWidget* widget )
{
     KIO::JobUiDelegate::DeletionType deletionType;
     switch (method) {
     case EMPTYTRASH:
         deletionType = KIO::JobUiDelegate::EmptyTrash;
         break;
     case DEL:
         deletionType = KIO::JobUiDelegate::Delete;
         break;
     default:
         deletionType = KIO::JobUiDelegate::Trash;
         break;
     }

    KIO::JobUiDelegate::ConfirmationType confirmationType = confirmation == FORCE_CONFIRMATION ? KIO::JobUiDelegate::ForceConfirmation : KIO::JobUiDelegate::DefaultConfirmation;
    KIO::JobUiDelegate uiDelegate;
    uiDelegate.setWindow(widget);
    return uiDelegate.askDeleteConfirmation(selectedUrls, deletionType, confirmationType);
}

void KonqOperations::doDrop( const KFileItem & destItem, const KUrl & dest, QDropEvent * ev, QWidget * parent )
{
    (void) KonqOperations::doDrop( destItem, dest, ev, parent, QList<QAction*>() );
}

KonqOperations *KonqOperations::doDrop( const KFileItem & destItem, const KUrl & dest, QDropEvent * ev, QWidget * parent,
                                        const QList<QAction*> & userActions  )
{
    kDebug(1203) << "dest:" << dest;
    QMap<QString, QString> metaData;
    // Prefer local urls if possible, to avoid problems with desktop:/ urls from other users (#184403)
    const KUrl::List lst = KUrl::List::fromMimeData(ev->mimeData(), KUrl::List::PreferLocalUrls, &metaData);
    if (!lst.isEmpty()) { // Are they urls ?
        //kDebug(1203) << "metaData:" << metaData.count() << "entries.";
        //QMap<QString,QString>::ConstIterator mit;
        //for( mit = metaData.begin(); mit != metaData.end(); ++mit ) {
        //    kDebug(1203) << "metaData: key=" << mit.key() << "value=" << mit.value();
        //}
        // Check if we dropped something on itself
        KUrl::List::ConstIterator it = lst.begin();
        for (; it != lst.end() ; it++) {
            kDebug(1203) << "URL:" << (*it);
            if (dest.equals(*it, KUrl::CompareWithoutTrailingSlash)) {
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
            op->_statUrl( dest, op, SLOT(asyncDrop(KFileItem)) );
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
            KIO::FileUndoManager::self()->recordJob(KIO::FileUndoManager::Put, KUrl::List(), simpleJob->url(), simpleJob);
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
            const QStringList urlStrList = m_info->urls.toStringList();
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
#ifndef Q_WS_WIN
                else
                {
                    const bool ro = desktopGroup.readEntry( "ReadOnly", false );
                    const QByteArray fstype = desktopGroup.readEntry( "FSType" ).toLatin1();
                    KAutoMount* am = new KAutoMount( ro, fstype, dev, point, m_destUrl.path(), false );
                    connect( am, SIGNAL(finished()), this, SLOT(doDropFileCopy()) );
                }
#endif
                return;
            }
            else if ( desktopFile.hasLinkType() && desktopGroup.hasKey("URL") ) {
                m_destUrl = desktopGroup.readPathEntry("URL", QString());
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
        const KUrl::List lst = m_info->urls;
        KUrl::List::ConstIterator it = lst.begin();
        for ( ; it != lst.end() ; it++ )
            args << (*it).path(); // assume local files
        kDebug(1203) << "starting " << m_destUrl.path() << " with " << lst.count() << " arguments";
        KProcess::startDetached( m_destUrl.path(), args );
    }
    deleteLater();
}

KUrl::List KonqOperations::droppedUrls() const
{
    return m_info->urls;
}

QPoint KonqOperations::dropPosition() const
{
   return m_info->mousePos;
}

void KonqOperations::doDropFileCopy()
{
    assert(m_info); // setDropInfo - and asyncDrop - should have been called before asyncDrop
    const KUrl::List lst = m_info->urls;
    Qt::DropAction action = m_info->action;
    bool isDesktopFile = false;
    bool itemIsOnDesktop = false;
    bool allItemsAreFromTrash = true;
    KUrl::List mlst; // list of items that can be moved
    for (KUrl::List::ConstIterator it = lst.begin(); it != lst.end(); ++it)
    {
        bool local = (*it).isLocalFile();
        if ( KProtocolManager::supportsDeleting( *it ) && (!local || QFileInfo((*it).directory()).isWritable() ))
            mlst.append(*it);
        if ( local && KDesktopFile::isDesktopFile((*it).toLocalFile()))
            isDesktopFile = true;
        if ( local && (*it).path().startsWith(KGlobalSettings::desktopPath()))
            itemIsOnDesktop = true;
        if ( local || (*it).protocol() != "trash" )
            allItemsAreFromTrash = false;
    }

    bool linkOnly = false; // if true, we'll show a popup menu, but with only "link" in it (for confirmation)
    if (isDesktopFile && !KAuthorized::authorizeKAction("run_desktop_files") &&
        (m_destUrl.path(KUrl::AddTrailingSlash) == KGlobalSettings::desktopPath()) ) {
        linkOnly = true;
    } else if ( allItemsAreFromTrash && lst.first().path() == "/" ) {
        // Dropping a link to the trash: don't move the full contents, just make a link (#319660)
        linkOnly = true;
    }

    if ( !mlst.isEmpty() && m_destUrl.protocol() == "trash" )
    {
        if ( itemIsOnDesktop && !KAuthorized::authorizeKAction("editable_desktop_icons") )
        {
            deleteLater();
            return;
        }

        m_method = TRASH;
        if ( askDeleteConfirmation( mlst, TRASH, DEFAULT_CONFIRMATION, parentWidget() ) )
            action = Qt::MoveAction;
        else
        {
            deleteLater();
            return;
        }
    } else if (!linkOnly && (allItemsAreFromTrash || m_destUrl.protocol() == "trash")) {
        // No point in asking copy/move/link when using dnd from or to the trash.
        action = Qt::MoveAction;
    }
    else if ( (
        ((m_info->keyboardModifiers & Qt::ControlModifier) == 0) &&
        ((m_info->keyboardModifiers & Qt::ShiftModifier) == 0) &&
        ((m_info->keyboardModifiers & Qt::AltModifier) == 0) ) || linkOnly )
    {
        // Neither control, shift or alt are pressed => show popup menu

        // TODO move this code out somehow. Allow user of KonqOperations to add his own actions...
#if 0
        KonqIconViewWidget *iconView = dynamic_cast<KonqIconViewWidget*>(parent());
        bool bSetWallpaper = false;
        if ( iconView && iconView->maySetWallpaper() && lst.count() == 1 )
	{
            KUrl url = lst.first();
            KMimeType::Ptr mime = KMimeType::findByUrl( url );
            if ( mime && ( ( KImageIO::isSupported(mime->name(), KImageIO::Reading) ) ||
                 mime->is( "image/svg+xml" ) ) )
            {
                bSetWallpaper = true;
            }
        }
#endif

        // Check what the source can do
        // we'll assume it's the same for all URLs (hack)
        // TODO: if we had a KFileItemList instead of a KUrl::List,
        // we could use KFileItemsCapabilities
        const KUrl url = lst.first();
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
        if (url.isLocalFile() && url.toLocalFile().startsWith(KStandardDirs::locateLocal("tmp", QString()))) {
            sMoving = false;
            sDeleting = false;
            enableLinking = false;
        }

        QMenu popup;
        QString seq = QKeySequence( Qt::ShiftModifier ).toString();
        seq.chop(1); // chop superfluous '+'
        QAction* popupMoveAction = new QAction(i18n( "&Move Here" ) + '\t' + seq, this);
        popupMoveAction->setIcon(KIcon("go-jump"));
        seq = QKeySequence( Qt::ControlModifier ).toString();
        seq.chop(1);
        QAction* popupCopyAction = new QAction(i18n( "&Copy Here" ) + '\t' + seq, this);
        popupCopyAction->setIcon(KIcon("edit-copy"));
        seq = QKeySequence( Qt::ControlModifier + Qt::ShiftModifier ).toString();
        seq.chop(1);
        QAction* popupLinkAction = new QAction(i18n( "&Link Here" ) + '\t' + seq, this);
        popupLinkAction->setIcon(KIcon("edit-link"));
        QAction* popupWallAction = new QAction( i18n( "Set as &Wallpaper" ), this );
        popupWallAction->setIcon(KIcon("preferences-desktop-wallpaper"));
        QAction* popupCancelAction = new QAction(i18n( "C&ancel" ) + '\t' + QKeySequence( Qt::Key_Escape ).toString(), this);
        popupCancelAction->setIcon(KIcon("process-stop"));

        if (!mlst.isEmpty() && (sMoving || (sReading && sDeleting)) && !linkOnly )
        {
            bool equalDestination = true;
            foreach ( const KUrl & src, lst )
            {
                const bool equalProtocol = ( m_destUrl.protocol() == src.protocol() );
                if ( !equalProtocol || m_destUrl.path(KUrl::RemoveTrailingSlash) != src.directory() )
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

#if 0
        if (bSetWallpaper)
            popup.addAction(popupWallAction);
#endif

        //now initialize the drop plugins
        KFileItemList fileItems;
        foreach(const KUrl& url, lst) {
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
        connect(job, SIGNAL(copyingDone(KIO::Job*,KUrl,KUrl,time_t,bool,bool)),
                this, SLOT(slotCopyingDone(KIO::Job*,KUrl,KUrl)));
        connect(job, SIGNAL(copyingLinkDone(KIO::Job*,KUrl,QString,KUrl)),
                this, SLOT(slotCopyingLinkDone(KIO::Job*,KUrl,QString,KUrl)));
        return; // we still have stuff to do -> don't delete ourselves
    }
    deleteLater();
}

void KonqOperations::slotCopyingDone( KIO::Job*, const KUrl&, const KUrl &to)
{
    m_createdUrls << to;
}

void KonqOperations::slotCopyingLinkDone(KIO::Job*, const KUrl&, const QString&, const KUrl &to)
{
    m_createdUrls << to;
}

void KonqOperations::_addPluginActions(QList<QAction*>& pluginActions,const KUrl& destination, const KFileItemListProperties& info)
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

// these two are from /apps/konqueror/settings/konq/globalpaths.cpp
static bool cleanHomeDirPath( QString &path, const QString &homeDir )
{
#ifdef Q_WS_WIN //safer
    if (!QDir::convertSeparators(path).startsWith(QDir::convertSeparators(homeDir)))
        return false;
#else
    if (!path.startsWith(homeDir))
        return false;
#endif

    int len = homeDir.length();
    // replace by "$HOME" if possible
    if (len && (path.length() == len || path[len] == '/')) {
        path.replace(0, len, QString::fromLatin1("$HOME"));
        return true;
    } else
        return false;
}
static QString translatePath( QString path ) // krazy:exclude=passbyvalue
{
    // keep only one single '/' at the beginning - needed for cleanHomeDirPath()
    while (path[0] == '/' && path[1] == '/')
        path.remove(0,1);

    // we probably should escape any $ ` and \ characters that may occur in the path, but the Qt code that reads back
    // the file doesn't unescape them so not much point in doing so

    // All of the 3 following functions to return the user's home directory
    // can return different paths. We have to test all them.
    const QString homeDir0 = QFile::decodeName(qgetenv("HOME"));
    const QString homeDir1 = QDir::homePath();
    const QString homeDir2 = QDir(homeDir1).canonicalPath();
    if (cleanHomeDirPath(path, homeDir0) ||
        cleanHomeDirPath(path, homeDir1) ||
        cleanHomeDirPath(path, homeDir2) ) {
        // kDebug() << "Path was replaced\n";
    }

    return path;
}

void KonqOperations::rename( QWidget * parent, const KUrl & oldurl, const KUrl& newurl )
{
    renameV2(parent, oldurl, newurl);
}

KonqOperations *KonqOperations::renameV2( QWidget * parent, const KUrl & oldurl, const KUrl& newurl )
{
    kDebug(1203) << "oldurl=" << oldurl << " newurl=" << newurl;
    if ( oldurl == newurl )
        return 0;

    KUrl::List lst;
    lst.append(oldurl);
    KIO::Job * job = KIO::moveAs( oldurl, newurl, oldurl.isLocalFile() ? KIO::HideProgressInfo : KIO::DefaultFlags );
    KonqOperations * op = new KonqOperations( parent );
    op->setOperation( job, RENAME, newurl );
    KIO::FileUndoManager::self()->recordJob( KIO::FileUndoManager::Rename, lst, newurl, job );
    // if moving the desktop then update config file and emit
    if ( oldurl.isLocalFile() && oldurl.toLocalFile( KUrl::AddTrailingSlash ) == KGlobalSettings::desktopPath() )
    {
        kDebug(1203) << "That rename was the Desktop path, updating config files";
        //save in XDG path
        const QString userDirsFile(KGlobal::dirs()->localxdgconfdir() + QLatin1String("user-dirs.dirs"));
        KConfig xdgUserConf( userDirsFile, KConfig::SimpleConfig );
        KConfigGroup g( &xdgUserConf, "" );
        g.writeEntry( "XDG_DESKTOP_DIR", QString("\"" + translatePath( newurl.path() ) + "\"") );
        KGlobalSettings::self()->emitChange(KGlobalSettings::SettingsChanged, KGlobalSettings::SETTINGS_PATHS);
    }

    return op;
}

void KonqOperations::setOperation( KIO::Job * job, Operation method, const KUrl & dest )
{
    m_method = method;
    m_destUrl = dest;
    if ( job )
    {
        KJobWidgets::setWindow(job, parentWidget());
        connect( job, SIGNAL(result(KJob*)),
                 SLOT(slotResult(KJob*)) );
#if 0
        KIO::CopyJob *copyJob = dynamic_cast<KIO::CopyJob*>(job);
        KonqIconViewWidget *iconView = dynamic_cast<KonqIconViewWidget*>(parent());
        if (copyJob && iconView)
        {
            connect(copyJob, SIGNAL(aboutToCreate(KIO::Job*,QList<KIO::CopyInfo>)),
                 this, SLOT(slotAboutToCreate(KIO::Job*,QList<KIO::CopyInfo>)));
            // TODO move this connect into the iconview!
            connect(this, SIGNAL(aboutToCreate(QPoint,QList<KIO::CopyInfo>)),
                 iconView, SLOT(slotAboutToCreate(QPoint,QList<KIO::CopyInfo>)));
        }
#endif
    }
    else // for link
        slotResult( 0L );
}

void KonqOperations::slotAboutToCreate(KIO::Job *, const QList<KIO::CopyInfo> &files)
{
    emit aboutToCreate( m_info ? m_info->mousePos : m_pasteInfo ? m_pasteInfo->mousePos : QPoint(), files);
}

void KonqOperations::statUrl( const KUrl & url, const QObject *receiver, const char *member, QWidget* parent )
{
    KonqOperations * op = new KonqOperations( parent );
    op->m_method = STAT;
    op->_statUrl( url, receiver, member );
}

void KonqOperations::_statUrl( const KUrl & url, const QObject *receiver, const char *member )
{
    connect( this, SIGNAL(statFinished(KFileItem)), receiver, member );
    KIO::StatJob * job = KIO::stat( url /*, KIO::HideProgressInfo?*/ );
    KJobWidgets::setWindow(job, parentWidget());
    connect( job, SIGNAL(result(KJob*)),
             SLOT(slotStatResult(KJob*)) );
}

void KonqOperations::slotStatResult( KJob * job )
{
    if ( job->error())
    {
        static_cast<KIO::Job*>( job )->ui()->showErrorMessage();
    }
    else
    {
        KIO::StatJob * statJob = static_cast<KIO::StatJob*>(job);
        KFileItem item( statJob->statResult(), statJob->url() );
        emit statFinished( item );
    }
    // If we're only here for a stat, we're done. But not if we used _statUrl internally
    if ( m_method == STAT )
        deleteLater();
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
    case EMPTYTRASH:
    case RESTORE:
        // Update konq windows opened on trash:/
        org::kde::KDirNotify::emitFilesAdded(QUrl("trash:/")); // yeah, files were removed, but we don't know which ones...
        break;
    case RENAME: {
            KIO::CopyJob *renameJob = qobject_cast<KIO::CopyJob*>(job);
            if (renameJob && jobFailed) {
                const KUrl oldUrl = renameJob->srcUrls().first();
                const KUrl newUrl = renameJob->destUrl();
                emit renamingFailed(oldUrl, newUrl);
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

void KonqOperations::rename( QWidget * parent, const KUrl & oldurl, const QString & name )
{
    renameV2(parent, oldurl, name);
}

KonqOperations *KonqOperations::renameV2( QWidget * parent, const KUrl & oldurl, const QString & name )
{
    KUrl newurl( oldurl );
    newurl.setPath( oldurl.directory( KUrl::AppendTrailingSlash ) + name );
    kDebug(1203) << "KonqOperations::rename("<<name<<") called. newurl=" << newurl;
    return renameV2( parent, oldurl, newurl );
}

// Duplicated in libkfile's KDirOperator
static bool confirmCreatingHiddenDir(const QString& name, QWidget* parent)
{
    KGuiItem continueGuiItem(KStandardGuiItem::cont());
    continueGuiItem.setText(i18nc("@action:button", "Create directory"));
    KGuiItem cancelGuiItem(KStandardGuiItem::cancel());
    cancelGuiItem.setText(i18nc("@action:button", "Enter a different name"));
    return KMessageBox::warningContinueCancel(
        parent,
        i18n("The name \"%1\" starts with a dot, so the directory will be hidden by default.", name),
        i18nc("@title:window", "Create hidden directory?"),
        continueGuiItem,
        cancelGuiItem,
        "confirm_create_hidden_dir") == KMessageBox::Continue;
}

KIO::SimpleJob* KonqOperations::newDir(QWidget * parent, const KUrl & baseUrl)
{
    return newDir(parent, baseUrl, NewDirFlags());
}

KIO::SimpleJob* KonqOperations::newDir(QWidget * parent, const KUrl & baseUrl, NewDirFlags flags)
{
    // Notice that kfile's KDirOperator::mkdir() is somewhat similar
    bool ok;
    QString name = i18nc( "@label Default name when creating a folder", "New Folder" );
    if ( baseUrl.isLocalFile() && QFileInfo( baseUrl.toLocalFile( KUrl::AddTrailingSlash ) + name ).exists() )
        name = KIO::RenameDialog::suggestName(baseUrl, name);

    bool askAgain;
    do {
        askAgain = false;
        name = KInputDialog::getText ( i18nc( "@title:window", "New Folder" ),
                                       i18nc( "@label:textbox", "Enter folder name:" ), name, &ok, parent );
        if ( ok && !name.isEmpty() ) {
            KUrl url;
            if ((name[0] == '/') || (name[0] == '~')) {
                url.setPath(KShell::tildeExpand(name));
            } else {
                const bool viewShowsHiddenFiles = (flags & ViewShowsHiddenFile);
                if (!viewShowsHiddenFiles && name.startsWith('.')) {
                    if (!confirmCreatingHiddenDir(name, parent)) {
                        askAgain = true;
                        continue;
                    }
                }
                name = KIO::encodeFileName( name );
                url = baseUrl;
                url.addPath( name );
            }
            return KonqOperations::mkdir( parent, url );
        }
    } while (askAgain);
    return 0;
}

////

KonqMultiRestoreJob::KonqMultiRestoreJob( const KUrl::List& urls )
    : KIO::Job(),
      m_urls( urls ), m_urlsIterator( m_urls.begin() ),
      m_progress( 0 )
{
    QTimer::singleShot(0, this, SLOT(slotStart()));
    setUiDelegate(new KIO::JobUiDelegate);
}

void KonqMultiRestoreJob::slotStart()
{
    if ( m_urlsIterator == m_urls.begin() ) // first time: emit total
        setTotalAmount( KJob::Files, m_urls.count() );

    if ( m_urlsIterator != m_urls.end() )
    {
        const KUrl& url = *m_urlsIterator;

        KUrl new_url = url;
        if ( new_url.protocol()=="system"
          && new_url.path().startsWith("/trash") )
        {
            QString path = new_url.path();
	    path.remove(0, 6);
	    new_url.setProtocol("trash");
	    new_url.setPath(path);
        }

        Q_ASSERT( new_url.protocol() == "trash" );
        QByteArray packedArgs;
        QDataStream stream( &packedArgs, QIODevice::WriteOnly );
        stream << (int)3 << new_url;
        KIO::Job* job = KIO::special( new_url, packedArgs, KIO::HideProgressInfo );
        addSubjob( job );
        setProcessedAmount(KJob::Files, processedAmount(KJob::Files) + 1);
    }
    else // done!
    {
        org::kde::KDirNotify::emitFilesRemoved(m_urls );
        emitResult();
    }
}

void KonqMultiRestoreJob::slotResult( KJob *job )
{
    if ( job->error() )
    {
        KIO::Job::slotResult( job ); // will set the error and emit result(this)
        return;
    }
    removeSubjob(job);
    // Move on to next one
    ++m_urlsIterator;
    ++m_progress;
    //emit processedSize( this, m_progress );
    emitPercent( m_progress, m_urls.count() );
    slotStart();
}

QWidget* KonqOperations::parentWidget() const
{
    return static_cast<QWidget *>( parent() );
}

QPair<bool, QString> KonqOperations::pasteInfo(const KUrl& targetUrl)
{
    QPair<bool, QString> ret;
    QClipboard* clipboard = QApplication::clipboard();
    const QMimeData* mimeData = clipboard->mimeData();

    const bool canPasteData = KIO::canPasteMimeData(mimeData);
    KUrl::List urls = KUrl::List::fromMimeData(mimeData);
    if (!urls.isEmpty() || canPasteData) {
        // disable the paste action if no writing is supported
        KFileItem item(KFileItem::Unknown, KFileItem::Unknown, targetUrl);
        ret.first = KFileItemListProperties(KFileItemList() << item).supportsWriting();

        if (urls.count() == 1) {
            const KFileItem item(KFileItem::Unknown, KFileItem::Unknown, urls.first(), true);
            ret.second = item.isDir() ? i18nc("@action:inmenu", "Paste One Folder") :
                                        i18nc("@action:inmenu", "Paste One File");

        } else if (!urls.isEmpty()) {
            ret.second = i18ncp("@action:inmenu", "Paste One Item", "Paste %1 Items", urls.count());
        } else {
            ret.second = i18nc("@action:inmenu", "Paste Clipboard Contents...");
        }
    } else {
        ret.first = false;
        ret.second = i18nc("@action:inmenu", "Paste");
    }

    return ret;
}

#include "konq_operations.moc"
