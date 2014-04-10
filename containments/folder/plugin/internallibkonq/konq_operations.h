/*  This file is part of the KDE project
    Copyright 2000-2007  David Faure <faure@kde.org>
    Copyright 2003       Waldo Bastian <bastian@kde.org>
    Copyright 2001-2002  Alexander Neundorf <neundorf@kde.org>
    Copyright 2002       Michael Brade <brade@kde.org>

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

#ifndef __konq_operations_h__
#define __konq_operations_h__

#include <kurl.h>
#include "libkonq_export.h"

#include <QObject>
#include <QDropEvent>

class KJob;
namespace KIO { class Job; class SimpleJob; struct CopyInfo; }
class QWidget;
class KFileItem;
class KFileItemListProperties;

/**
 * Implements file operations (move,del,trash,paste,copy,move,link...)
 * for file managers
 */
class LIBKONQ_EXPORT KonqOperations : public QObject
{
    Q_OBJECT
protected:
    KonqOperations( QWidget * parent );
    virtual ~KonqOperations();

public:
    /**
     * Pop up properties dialog for mimetype @p mimeType.
     * @param parent parent widget (for dialogs)
     */
    static void editMimeType( const QString & mimeType, QWidget* parent );

    enum Operation { TRASH, DEL, COPY, MOVE, LINK, EMPTYTRASH, STAT, MKDIR, RESTORE, UNKNOWN, PUT, RENAME };
    /**
     * Delete the @p selectedUrls if possible.
     *
     * @param parent parent widget (for error dialog box if any)
     * @param method should be TRASH or DEL
     * @param selectedUrls the URLs to be deleted
     */
    static void del( QWidget * parent, Operation method, const KUrl::List & selectedUrls );

    /**
     * Copy the @p selectedUrls to the destination @p destUrl.
     *
     * @param parent parent widget (for error dialog box if any)
     * @param method should be COPY, MOVE or LINK
     * @param selectedUrls the URLs to copy
     * @param destUrl destination of the copy
     *
     * @todo document restrictions on the kind of destination
     */
    static void copy( QWidget * parent, Operation method, const KUrl::List & selectedUrls, const KUrl& destUrl );
    /**
     * Drop
     * @param destItem destination KFileItem for the drop (background or item)
     * @param destUrl destination URL for the drop.
     * @param ev the drop event
     * @param parent parent widget (for error dialog box if any)
     *
     * If destItem is 0L, doDrop will stat the URL to determine it.
     */
    static void doDrop( const KFileItem & destItem, const KUrl & destUrl, QDropEvent * ev, QWidget * parent );

    /**
     * Drop
     * @param destItem destination KFileItem for the drop (background or item)
     * @param destUrl destination URL for the drop.
     * @param ev the drop event
     * @param parent parent widget (for error dialog box if any)
     * @param userActions additional actions to include in the drop menu
     *
     * This is an overloaded member function that lets you add your own actions
     * to the drop menu shown by KonqOperations.
     *
     * The drop menu will be shown when the application re-enters the event loop.
     *
     * If destItem is 0L, doDrop will stat the URL to determine it.
     *
     * Note that the returned KonqOperations object will be deleted automatically
     * when the drop is completed.
     *
     * It is still valid when a slot connected to a triggered() signal in one
     * of the user actions is invoked, but should not be assumed to be valid
     * after the slot returns.
     *
     * @return The KonqOperations object
     * @since 4.3
     */
    static KonqOperations *doDrop( const KFileItem & destItem, const KUrl & destUrl, QDropEvent * ev, QWidget * parent,
                                   const QList<QAction*> &userActions );

    /**
     * Paste the clipboard contents
     */
    static void doPaste( QWidget * parent, const KUrl & destUrl, const QPoint &pos = QPoint() );

    /**
     * Paste the clipboard contents
     *
     * @return The KonqOperations object
     * @since 4.10
     *
     * @todo TODO KDE 5,0 - Merge doPaste and doPasteV2
     */
    static KonqOperations *doPasteV2(QWidget * parent, const KUrl & destUrl, const QPoint &pos = QPoint());

    /**
     * Returns the state of the paste action:
     * first is whether the action should be enabled
     * second is the text for the action
     * @since 4.3
     */
    static QPair<bool, QString> pasteInfo(const KUrl& targetUrl);

    /**
     * Empty the trash
     */
    static void emptyTrash( QWidget* parent );
    /**
     * Restore trashed items
     */
    static void restoreTrashedItems( const KUrl::List& urls, QWidget* parent );

    /**
     * Create a directory. Same as KIO::mkdir but records job into KonqFileUndoManager for undo/redo purposes.
     */
    static KIO::SimpleJob* mkdir( QWidget *parent, const KUrl & url );

    /**
     * Ask for the name of a new directory and create it.
     * Calls KonqOperations::mkdir.
     *
     * @param parent the parent widget
     * @param baseUrl the directory to create the new directory in
     * @return the job used to create the directory or 0 if the creation was cancelled by the user
     */
    static KIO::SimpleJob* newDir( QWidget * parent, const KUrl & baseUrl );
    enum NewDirFlag { ViewShowsHiddenFile = 1 };
    Q_DECLARE_FLAGS(NewDirFlags, NewDirFlag)
    /**
     * Ask for the name of a new directory and create it.
     * Calls KonqOperations::mkdir.
     *
     * @param parent the parent widget
     * @param baseUrl the directory to create the new directory in
     * @param flags see NewDirFlags
     * @return the job used to create the directory or 0 if the creation was cancelled by the user
     */
    static KIO::SimpleJob* newDir( QWidget * parent, const KUrl & baseUrl, NewDirFlags flags );

    /**
     * Get info about a given URL, and when that's done (it's asynchronous!),
     * call a given slot with a const KFileItem& as argument.
     * The KFileItem will be deleted by statUrl after calling the slot. Make a copy
     * if you need one !
     */
    static void statUrl( const KUrl & url, const QObject *receiver, const char *member, QWidget* parent );

    /**
     * Do a renaming.
     * @param parent the parent widget, passed to KonqOperations ctor
     * @param oldurl the current url of the file to be renamed
     * @param name the new name for the file. Shouldn't include '/'.
     */
    static void rename( QWidget * parent, const KUrl & oldurl, const QString & name );

    /**
     * Do a renaming.
     * @param parent the parent widget, passed to KonqOperations ctor
     * @param oldurl the current url of the file to be renamed
     * @param name the new name for the file. Shouldn't include '/'.
     *
     * @return The KonqOperations object
     * @since 4.11
     *
     * @todo TODO KDE 5.0 - Merge rename and renameV2
     */
    static KonqOperations *renameV2( QWidget * parent, const KUrl & oldurl, const QString & name );

    /**
     * Do a renaming.
     * @param parent the parent widget, passed to KonqOperations ctor
     * @param oldurl the current url of the file to be renamed
     * @param newurl the new url for the file
     * Use this version if the other one wouldn't work :)  (e.g. because name could
     * be a relative path, including a '/').
     */
    static void rename( QWidget * parent, const KUrl & oldurl, const KUrl & newurl );

    /**
     * Do a renaming.
     * @param parent the parent widget, passed to KonqOperations ctor
     * @param oldurl the current url of the file to be renamed
     * @param newurl the new url for the file
     * Use this version if the other one wouldn't work :)  (e.g. because name could
     * be a relative path, including a '/').
     *
     * @return The KonqOperations object
     * @since 4.11
     *
     * @todo TODO KDE 5.0 - Merge rename and renameV2
     */
    static KonqOperations *renameV2( QWidget * parent, const KUrl & oldurl, const KUrl & newurl );

    enum ConfirmationType { DEFAULT_CONFIRMATION, SKIP_CONFIRMATION, FORCE_CONFIRMATION };
    /**
     * Ask for confirmation before deleting/trashing @p selectedUrls.
     * @param selectedUrls the urls about to be deleted
     * @param method the type of deletion (DEL for real deletion, anything else for trash)
     * @param confirmation default (based on config file), skip (no confirmation) or force (always confirm)
     * @param widget parent widget for message boxes
     * @return true if confirmed
     */
    static bool askDeleteConfirmation( const KUrl::List & selectedUrls, int method, ConfirmationType confirmation, QWidget* widget );

    /**
     * Returns the list of dropped URL's.
     *
     * You can call this method on the object returned by KonqOperations::doDrop(),
     * to obtain the list of URL's this object handles.
     *
     * @since 4.3
     */
    KUrl::List droppedUrls() const;

    /**
     * Returns the position where the drop occurred.
     * @since 4.3
     */
    QPoint dropPosition() const;

Q_SIGNALS:
    void statFinished( const KFileItem & item );
    void aboutToCreate(const QPoint &pos, const QList<KIO::CopyInfo> &files);
    void aboutToCreate(const KUrl::List &urls);
    void renamingFailed(const KUrl &oldUrl, const KUrl &newUrl);

private:
    QWidget* parentWidget() const;
    void _del( Operation method, const KUrl::List & selectedUrls, ConfirmationType confirmation );
    void _restoreTrashedItems( const KUrl::List& urls );
    void _statUrl( const KUrl & url, const QObject *receiver, const char *member );
    void _addPluginActions(QList<QAction*>& pluginActions, const KUrl& destination, const KFileItemListProperties& info);

    // internal, for COPY/MOVE/LINK/MKDIR
    void setOperation( KIO::Job * job, Operation method, const KUrl & dest );

    struct DropInfo
    {
        DropInfo( Qt::KeyboardModifiers k, const KUrl::List & u, const QMap<QString,QString> &m,
                  const QPoint& pos, Qt::DropAction a, const QList<QAction *> &actions) :
            keyboardModifiers(k), urls(u), metaData(m), mousePos(pos), action(a), userActions(actions)
        {}
        Qt::KeyboardModifiers keyboardModifiers;
        KUrl::List urls;
        QMap<QString,QString> metaData;
        QPoint mousePos;
        Qt::DropAction action;
        QList<QAction*> userActions;
    };
    // internal, for doDrop
    void setDropInfo( DropInfo * info ) { m_info = info; }

    struct KIOPasteInfo // KDE4: remove and use DropInfo instead or a QPoint member
    {
        // Used to position the files at the position where RMB/Paste was used [mostly on the desktop]
        QPoint mousePos;
    };
    void setPasteInfo( KIOPasteInfo * info ) { m_pasteInfo = info; }

protected Q_SLOTS:

    void slotAboutToCreate(KIO::Job *job, const QList<KIO::CopyInfo> &files);
    void slotResult( KJob * job );
    void slotStatResult( KJob * job );
    void asyncDrop( const KFileItem & item );
    void doDropFileCopy();
    void slotCopyingDone(KIO::Job *job, const KUrl &from, const KUrl &to);
    void slotCopyingLinkDone(KIO::Job *job, const KUrl &from, const QString &target, const KUrl &to);

private:
    Operation m_method;
    //KUrl::List m_srcUrls;
    KUrl::List m_createdUrls;
    KUrl m_destUrl;
    // for doDrop
    DropInfo * m_info;
    KIOPasteInfo * m_pasteInfo;
};

#include <kio/job.h>

/// Restore multiple trashed files
class KonqMultiRestoreJob : public KIO::Job
{
    Q_OBJECT

public:
    KonqMultiRestoreJob( const KUrl::List& urls );

protected Q_SLOTS:
    virtual void slotStart();
    virtual void slotResult( KJob *job );

private:
    const KUrl::List m_urls;
    KUrl::List::const_iterator m_urlsIterator;
    int m_progress;
};

#endif
