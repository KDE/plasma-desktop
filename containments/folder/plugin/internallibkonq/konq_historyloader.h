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

#ifndef KONQ_HISTORYLOADER_H
#define KONQ_HISTORYLOADER_H

#include "libkonq_export.h"
#include <QObject>

class KonqHistoryList;
class KonqHistoryLoaderPrivate;

/**
 * @internal
 * This class loads the Konqueror history file.
 * @since 4.3
 */
class KonqHistoryLoader : public QObject
{
    Q_OBJECT

public:
    explicit KonqHistoryLoader(QObject* parent = 0);
    virtual ~KonqHistoryLoader();

    /**
     * Load the history. No need to call this more than once...
     */
    bool loadHistory();

    /**
     * @returns the list of all history entries, sorted by date
     * (oldest entries first)
     */
    const KonqHistoryList& entries() const;

    static int historyVersion();

private:
    KonqHistoryLoaderPrivate* const d;
};

#endif /* KONQ_HISTORYLOADER_H */
