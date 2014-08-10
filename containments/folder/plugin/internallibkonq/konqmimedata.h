/* This file is part of the KDE projects
   Copyright (C) 2005 David Faure <faure@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KONQMIMEDATA_H
#define KONQMIMEDATA_H

#include <libkonq_export.h>
#include <kurl.h>
class QMimeData;

/**
 * This class provides functions for creating and decoding clipboard/drag-n-drop data
 * of URLs. In particular it ships: (kde) urls, most-local urls, and an "is cut" boolean.
 */
class LIBKONQ_EXPORT KonqMimeData
{
public:
    /**
     * Populate a QMimeData with urls, and whether they were cut or copied.
     *
     * @param mimeData pointer to the mimeData object to be populated.
     *                 Must not be 0.
     * @param kdeURLs list of urls (which can include kde-specific protocols).
     * This list can be empty if only local urls are being used anyway.
     * @param mostLocalURLs "most local urls" (which try to resolve those to file:/ where possible),
     * @param cut if true, the user selected "cut" (saved as application/x-kde-cutselection in the mimedata).
     */
    static void populateMimeData( QMimeData* mimeData,
                                  const KUrl::List& kdeURLs,
                                  const KUrl::List& mostLocalURLs,
                                  bool cut = false );

    /**
     * Add the information whether the files were cut, into the mimedata.
     * @param mimeData pointer to the mimeData object to be populated.
     *                 Must not be 0.
     * @param cut if true, the user selected "cut" (saved as application/x-kde-cutselection in the mimedata).
     * @since 4.2
     */
    static void addIsCutSelection(QMimeData* mimeData,
                                  bool cut);

    /**
     * @return true if the urls in @p mimeData were cut
     */
    static bool decodeIsCutSelection( const QMimeData *mimeData );
};

#endif /* KONQMIMEDATA_H */

