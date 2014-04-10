/* This file is part of the KDE project
   Copyright 2000,2001 Carsten Pfeiffer <pfeiffer@kde.org>
   Copyright 2007-2009 David Faure <faure@kde.org>

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

#ifndef KONQ_HISTORYPROVIDER_H
#define KONQ_HISTORYPROVIDER_H

#include <kparts/historyprovider.h>
#include <kurl.h>
#include "libkonq_export.h"
#include "konq_historyentry.h"

class KonqHistoryEntry;
class KonqHistoryList;
class KonqHistoryProviderPrivate;

/**
 * This class maintains and manages a history of all URLs visited by Konqueror.
 * It synchronizes the history with other KonqHistoryProvider instances in
 * other processes (konqueror, history list, krunner etc.) via D-Bus to keep
 * one global and persistent history.
 */
class LIBKONQ_EXPORT KonqHistoryProvider : public KParts::HistoryProvider
{
    Q_OBJECT

public:
    /**
     * Returns the KonqHistoryProvider instance. This relies on a KonqHistoryProvider
     * instance being created first!
     * This is a bit like "qApp": you can access the instance anywhere, but you need to
     * create it first.
     */
    static KonqHistoryProvider *self() {
        return static_cast<KonqHistoryProvider*>(KParts::HistoryProvider::self());
    }

    explicit KonqHistoryProvider(QObject* parent = 0);
    virtual ~KonqHistoryProvider();

    /**
     * @returns the list of all history entries, sorted by date
     * (oldest entries first)
     */
    const KonqHistoryList& entries() const;

    /**
     * @returns the current maximum number of history entries.
     */
    int maxCount() const;

    /**
     * @returns the current maximum age (in days) of history entries.
     */
    int maxAge() const;

    /**
     * Sets a new maximum size of history and truncates the current history
     * if necessary. Notifies all other Konqueror instances via D-Bus
     * to do the same.
     *
     * The history is saved after receiving the D-Bus call.
     */
    void emitSetMaxCount(int count);

    /**
     * Sets a new maximum age of history entries and removes all entries that
     * are older than @p days. Notifies all other Konqueror instances via D-Bus
     * to do the same.
     *
     * An age of 0 means no expiry based on the age.
     *
     * The history is saved after receiving the D-Bus call.
     */
    void emitSetMaxAge(int days);

    /**
     * Removes the history entry for @p url, if existent. Tells all other
     * Konqueror instances via D-Bus to do the same.
     *
     * The history is saved after receiving the D-Bus call.
     */
    void emitRemoveFromHistory(const KUrl& url);

    /**
     * Removes the history entries for the given list of @p urls. Tells all
     * other Konqueror instances via D-Bus to do the same.
     *
     * The history is saved after receiving the D-Bus call.
     */
    void emitRemoveListFromHistory(const KUrl::List& urls);

    /**
     * Clears the history and tells all other Konqueror instances via D-Bus
     * to do the same.
     * The history is saved afterwards, if necessary.
     */
    void emitClear();

    /**
     * Load the whole history from disk. Call this exactly once.
     */
    bool loadHistory();

Q_SIGNALS:
    /**
     * Emitted after a new entry was added
     */
    void entryAdded( const KonqHistoryEntry& entry );

    /**
     * Emitted after an entry was removed from the history
     * Note, that this entry will be deleted immediately after you got
     * that signal.
     */
    void entryRemoved( const KonqHistoryEntry& entry );

protected: // only to be used by konqueror's KonqHistoryManager

    virtual void finishAddingEntry(const KonqHistoryEntry& entry, bool isSender);
    virtual void removeEntry(KonqHistoryList::iterator it);

    /**
     * a little optimization for KonqHistoryList::findEntry(),
     * checking the dict of KParts::HistoryProvider before traversing the list.
     * Can't be used everywhere, because it always returns end() for "pending"
     * entries, as those are not added to the dict, currently.
     */
    KonqHistoryList::iterator findEntry(const KUrl& url);
    KonqHistoryList::const_iterator constFindEntry(const KUrl& url) const;

    /**
     * Notifies all running instances about a new HistoryEntry via D-Bus.
     */
    void emitAddToHistory(const KonqHistoryEntry& entry);

private:
    KonqHistoryProviderPrivate* const d;
    friend class KonqHistoryProviderPrivate;
};

#endif /* KONQ_HISTORYPROVIDER_H */
