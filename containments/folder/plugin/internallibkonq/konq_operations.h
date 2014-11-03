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

#include <QObject>
#include <QDropEvent>
#include <QUrl>

class KJob;
namespace KIO { class Job; class SimpleJob; }
class QWidget;
class KFileItem;
class KFileItemListProperties;

/**
 * Implements file operations (move,del,trash,paste,copy,move,link...)
 * for file managers
 */
class KonqOperations : public QObject
{
    Q_OBJECT
protected:
    KonqOperations( QWidget * parent );
    virtual ~KonqOperations();

public:
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
    static KonqOperations *doDrop(const KFileItem & destItem, const QUrl &destUrl, QDropEvent * ev, QWidget * parent,
                                   const QList<QAction*> &userActions = QList<QAction *>() );

Q_SIGNALS:
    void aboutToCreate(const QList<QUrl> &urls);

private:
    QWidget* parentWidget() const;
    void _addPluginActions(QList<QAction*>& pluginActions, const QUrl& destination, const KFileItemListProperties& info);

    // internal, for COPY/MOVE/LINK/MKDIR
    enum Operation { TRASH, COPY, MOVE, LINK, UNKNOWN, PUT };
    void setOperation( KIO::Job * job, Operation method, const QUrl & dest );

    struct DropInfo
    {
        DropInfo( Qt::KeyboardModifiers k, const QList<QUrl> & u, const QMap<QString,QString> &m,
                  const QPoint& pos, Qt::DropAction a, const QList<QAction *> &actions) :
            keyboardModifiers(k), urls(u), metaData(m), mousePos(pos), action(a), userActions(actions)
        {}
        Qt::KeyboardModifiers keyboardModifiers;
        QList<QUrl> urls;
        QMap<QString,QString> metaData;
        QPoint mousePos;
        Qt::DropAction action;
        QList<QAction*> userActions;
    };
    // internal, for doDrop
    void setDropInfo( DropInfo * info ) { m_info = info; }

protected Q_SLOTS:

    void slotResult( KJob * job );
    void slotStatResult( KJob * job );
    void asyncDrop( const KFileItem & item );
    void doDropFileCopy();
    void slotCopyingDone(KIO::Job *job, const QUrl &from, const QUrl &to);
    void slotCopyingLinkDone(KIO::Job *job, const QUrl &from, const QString &target, const QUrl &to);

private:
    Operation m_method;
    QList<QUrl> m_createdUrls;
    QUrl m_destUrl;
    // for doDrop
    DropInfo * m_info;
};

#endif
