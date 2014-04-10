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

#include "konq_historyentry.h"
#include <kdebug.h>

KonqHistoryEntry::KonqHistoryEntry()
    : numberOfTimesVisited(1), d(0)
{
}

KonqHistoryEntry::~KonqHistoryEntry()
{
}

bool KonqHistoryEntry::operator==(const KonqHistoryEntry& entry) const
{
    return url == entry.url &&
        typedUrl == entry.typedUrl &&
        title == entry.title &&
        numberOfTimesVisited == entry.numberOfTimesVisited &&
        firstVisited == entry.firstVisited &&
        lastVisited == entry.lastVisited;
}

void KonqHistoryEntry::load(QDataStream& s, Flags flags)
{
    if (flags & MarshalUrlAsStrings) {
        QString urlStr;
        s >> urlStr;
        url = urlStr;
    } else {
        s >> url;
    }
    s >> typedUrl;
    s >> title;
    s >> numberOfTimesVisited;
    s >> firstVisited;
    s >> lastVisited;
}

void KonqHistoryEntry::save(QDataStream& s, Flags flags) const
{
    if (flags & MarshalUrlAsStrings) {
        s << url.url();
    } else {
        s << url;
    }
    s << typedUrl;
    s << title;
    s << numberOfTimesVisited;
    s << firstVisited;
    s << lastVisited;
}

KonqHistoryEntry::KonqHistoryEntry(const KonqHistoryEntry& e)
{
    url = e.url;
    typedUrl = e.typedUrl;
    title = e.title;
    numberOfTimesVisited = e.numberOfTimesVisited;
    firstVisited = e.firstVisited;
    lastVisited = e.lastVisited;
    d = 0;
}

////

KonqHistoryList::iterator KonqHistoryList::findEntry( const KUrl& url )
{
    // we search backwards, probably faster to find an entry
    KonqHistoryList::iterator it = end();
    while ( it != begin() ) {
        --it;
	if ( (*it).url == url )
	    return it;
    }
    return end();
}

KonqHistoryList::const_iterator KonqHistoryList::constFindEntry( const KUrl& url ) const
{
    // we search backwards, probably faster to find an entry
    KonqHistoryList::const_iterator it = constEnd();
    while ( it != constBegin() ) {
        --it;
	if ( (*it).url == url )
	    return it;
    }
    return constEnd();
}

void KonqHistoryList::removeEntry( const KUrl& url )
{
    iterator it = findEntry( url );
    if ( it != end() )
        erase( it );
}

