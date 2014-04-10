/* This file is part of the KDE project
   Copyright 2009 David Faure <faure@kde.org>

   This library is free software; you can redistribute it and/or modify
   it under the terms of the GNU Library General Public License as published
   by the Free Software Foundation; either version 2 of the License or
   ( at your option ) version 3 or, at the discretion of KDE e.V.
   ( which shall act as a proxy as in section 14 of the GPLv3 ), any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KONQ_HISTORYENTRY_H
#define KONQ_HISTORYENTRY_H

#include <QDateTime>
#include <QMetaType>
#include <kurl.h>
#include "libkonq_export.h"

class LIBKONQ_EXPORT KonqHistoryEntry
{
public:
    KonqHistoryEntry();
    ~KonqHistoryEntry();

    KUrl url;
    QString typedUrl;
    QString title;
    quint32 numberOfTimesVisited;
    QDateTime firstVisited;
    QDateTime lastVisited;

    KonqHistoryEntry(const KonqHistoryEntry& e);

    // Necessary for QList (on Windows)
    bool operator==(const KonqHistoryEntry& entry) const;

    enum Flags { NoFlags = 0, MarshalUrlAsStrings = 1 };
    void load(QDataStream& s, Flags flags);
    void save(QDataStream& s, Flags flags) const;

private:
    class Private;
    Private* d;
};

#ifdef MAKE_KONQ_LIB
KDE_DUMMY_QHASH_FUNCTION(KonqHistoryEntry)
#endif

Q_DECLARE_METATYPE(KonqHistoryEntry)

class LIBKONQ_EXPORT KonqHistoryList : public QList<KonqHistoryEntry>
{
public:
    /**
     * Finds an entry by URL and return an iterator to it.
     * If no matching entry is found, end() is returned.
     */
    iterator findEntry( const KUrl& url );

    /**
     * Finds an entry by URL and return an iterator to it.
     * If no matching entry is found, end() is returned.
     */
    const_iterator constFindEntry( const KUrl& url ) const;

    /**
     * Finds an entry by URL and removes it
     */
    void removeEntry( const KUrl& url );
};


#endif /* KONQ_HISTORYENTRY_H */

