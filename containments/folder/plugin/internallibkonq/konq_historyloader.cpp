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

#include "konq_historyloader.h"
#include <kdebug.h>
#include <QFile>
#include <kstandarddirs.h>
#include "konq_historyentry.h"
#include <zlib.h> // for crc32

class KonqHistoryLoaderPrivate
{
public:
    KonqHistoryList m_history;
};

KonqHistoryLoader::KonqHistoryLoader(QObject* parent)
    : QObject(parent), d(new KonqHistoryLoaderPrivate)
{
    loadHistory();
}

KonqHistoryLoader::~KonqHistoryLoader()
{
    delete d;
}

/**
 * Ensures that the items are sorted by the lastVisited date
 * (oldest goes first)
 */
static bool lastVisitedOrder( const KonqHistoryEntry& lhs, const KonqHistoryEntry& rhs ) {
    return lhs.lastVisited < rhs.lastVisited;
}

bool KonqHistoryLoader::loadHistory()
{
    d->m_history.clear();

    const QString filename = KStandardDirs::locateLocal("data", QLatin1String("konqueror/konq_history"));
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
	if (file.exists())
	    kWarning() << "Can't open" << filename;
	return false;
    }

    QDataStream fileStream(&file);
    QByteArray data; // only used for version == 2
    // we construct the stream object now but fill in the data later.
    QDataStream crcStream(&data, QIODevice::ReadOnly);
    KonqHistoryEntry::Flags flags = KonqHistoryEntry::NoFlags;

    if (!fileStream.atEnd()) {
	quint32 version;
        fileStream >> version;

        QDataStream *stream = &fileStream;

        bool crcChecked = false;
        bool crcOk = false;

        if (version >= 2 && version <= 4) {
            quint32 crc;
            crcChecked = true;
            fileStream >> crc >> data;
            crcOk = crc32(0, reinterpret_cast<unsigned char *>(data.data()), data.size()) == crc;
            stream = &crcStream; // pick up the right stream
        }

        // We can't read v3 history anymore, because operator<<(KURL) disappeared.

        if (version == 4)
        {
            // Use QUrl marshalling for V4 format.
	    flags = KonqHistoryEntry::NoFlags;
	}

#if 0 // who cares for versions 1 and 2 nowadays...
	if (version != 0 && version < 3) //Versions 1,2 (but not 0) are also valid
	{
	    //Turn on backwards compatibility mode..
	    marshalURLAsStrings = true;
	    // it doesn't make sense to save to save maxAge and maxCount  in the
	    // binary file, this would make backups impossible (they would clear
	    // themselves on startup, because all entries expire).
	    // [But V1 and V2 formats did it, so we do a dummy read]
	    quint32 dummy;
	    *stream >> dummy;
	    *stream >> dummy;

	    //OK.
	    version = 3;
	}
#endif

        if (historyVersion() != (int)version || (crcChecked && !crcOk)) {
	    kWarning() << "The history version doesn't match, aborting loading" ;
	    file.close();
	    return false;
	}

        while (!stream->atEnd()) {
	    KonqHistoryEntry entry;
            entry.load(*stream, flags);
	    // kDebug(1202) << "loaded entry:" << entry.url << ", Title:" << entry.title;
	    d->m_history.append(entry);
	}

	//kDebug(1202) << "loaded:" << m_history.count() << "entries.";

	qSort(d->m_history.begin(), d->m_history.end(), lastVisitedOrder);
    }

    // Theoretically, we should emit update() here, but as we only ever
    // load items on startup up to now, this doesn't make much sense.
    // emit KParts::HistoryProvider::update(some list);
    return true;
}

const KonqHistoryList& KonqHistoryLoader::entries() const
{
    return d->m_history;
}

int KonqHistoryLoader::historyVersion()
{
    return 4;
}
