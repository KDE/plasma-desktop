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

#include "konq_historyprovider.h"
#include <ksavefile.h>
#include <kstandarddirs.h>
#include <kdebug.h>
#include <kconfiggroup.h>
#include <ksharedconfig.h>
#include "konq_historyloader.h"
#include <zlib.h> // for crc32
#include <QtDBus>

class KonqHistoryProviderPrivate : public QObject, QDBusContext
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.Konqueror.HistoryManager")
public:
    KonqHistoryProviderPrivate(KonqHistoryProvider* qq);

    /**
     * Resizes the history list to contain less or equal than m_maxCount
     * entries. The first (oldest) entries are removed.
     */
    void adjustSize();

    /**
     * Saves the entire history.
     */
    bool saveHistory();

Q_SIGNALS: // DBUS methods/signals,  they have to match org.kde.Konqueror.HistoryManager.xml
    friend class KonqHistoryProvider;
    /**
     * Every konqueror instance broadcasts new history entries to the other
     * konqueror instances. Those add the entry to their list, but don't
     * save the list, because the sender saves the list.
     *
     * @param e the new history entry
     * @param saveId is the dbus service of the sender so that
     * only the sender saves the new history.
     */
    void notifyHistoryEntry(const QByteArray & historyEntry);

    /**
     * Called when the configuration of the maximum count changed.
     * Called via DBUS by some config-module
     */
    void notifyMaxCount(int count);

    /**
     * Called when the configuration of the maximum age of history-entries
     * changed. Called via DBUS by some config-module
     */
    void notifyMaxAge(int days);

    /**
     * Clears the history completely. Called via DBUS by some config-module
     */
    void notifyClear();

    /**
     * Notifes about a url that has to be removed from the history.
     * The sender instance has to save the history.
     */
    void notifyRemove(const QString& url);

    /**
     * Notifes about a list of urls that has to be removed from the history.
     * The sender instance has to save the history.
     */
    void notifyRemoveList(const QStringList& urls);

private Q_SLOTS: // connected to DBUS signals
    void slotNotifyHistoryEntry(const QByteArray& historyEntry);
    void slotNotifyMaxCount(int count);
    void slotNotifyMaxAge(int days);
    void slotNotifyClear();
    void slotNotifyRemove(const QString& url);
    void slotNotifyRemoveList(const QStringList& urls);

public:
    KSharedConfig::Ptr konqConfig() {
        // We want to use konquerorrc even when this class isn't used in konqueror,
        // this is why this doesn't say KGlobal::config().
        return KSharedConfig::openConfig("konquerorrc");
    }

    KonqHistoryList m_history;
    int m_maxCount;   // maximum of history entries
    int m_maxAgeDays; // maximum age of a history entry
    KonqHistoryProvider* q;
};

KonqHistoryProviderPrivate::KonqHistoryProviderPrivate(KonqHistoryProvider* qq)
    : QObject(), QDBusContext(), q(qq)
{
    // defaults
    KConfigGroup cs(konqConfig(), "HistorySettings");
    m_maxCount = cs.readEntry("Maximum of History entries", 500);
    m_maxCount = qMax(1, m_maxCount);
    m_maxAgeDays = cs.readEntry("Maximum age of History entries", 90);

    const QString dbusPath = "/KonqHistoryManager";
    const QString dbusInterface = "org.kde.Konqueror.HistoryManager";

    QDBusConnection dbus = QDBusConnection::sessionBus();
    dbus.registerObject(dbusPath, this, QDBusConnection::ExportAllSignals);
    dbus.connect(QString(), dbusPath, dbusInterface, "notifyClear", this, SLOT(slotNotifyClear()));
    dbus.connect(QString(), dbusPath, dbusInterface, "notifyHistoryEntry", this, SLOT(slotNotifyHistoryEntry(QByteArray)));
    dbus.connect(QString(), dbusPath, dbusInterface, "notifyMaxAge", this, SLOT(slotNotifyMaxAge(int)));
    dbus.connect(QString(), dbusPath, dbusInterface, "notifyMaxCount", this, SLOT(slotNotifyMaxCount(int)));
    dbus.connect(QString(), dbusPath, dbusInterface, "notifyRemove", this, SLOT(slotNotifyRemove(QString)));
    dbus.connect(QString(), dbusPath, dbusInterface, "notifyRemoveList", this, SLOT(slotNotifyRemoveList(QStringList)));
}

////

KonqHistoryProvider::KonqHistoryProvider(QObject* parent)
    : KParts::HistoryProvider(parent), d(new KonqHistoryProviderPrivate(this))
{
}

KonqHistoryProvider::~KonqHistoryProvider()
{
    delete d;
}

const KonqHistoryList& KonqHistoryProvider::entries() const
{
    return d->m_history;
}

bool KonqHistoryProvider::loadHistory()
{
    KonqHistoryLoader loader;
    if (!loader.loadHistory()) {
        return false;
    }

    d->m_history = loader.entries();

    d->adjustSize();

    QListIterator<KonqHistoryEntry> it(d->m_history);
    while (it.hasNext()) {
        const KonqHistoryEntry& entry = it.next();

        // Fill the entries into KParts::HistoryProvider.
        const QString urlString = entry.url.url();
        KParts::HistoryProvider::insert(urlString);
        // DF: also insert the "pretty" version if different
        // This helps getting 'visited' links on websites which don't use fully-escaped urls.
        const QString prettyUrlString = entry.url.prettyUrl();
        if (urlString != prettyUrlString)
            KParts::HistoryProvider::insert(prettyUrlString);
    }

    return true;
}

void KonqHistoryProviderPrivate::adjustSize()
{
    if (m_history.isEmpty())
        return;

    KonqHistoryEntry entry = m_history.first();
    const QDateTime expirationDate(QDate::currentDate().addDays(-m_maxAgeDays));

    while (m_history.count() > (qint32)m_maxCount ||
           (m_maxAgeDays > 0 && entry.lastVisited.isValid() && entry.lastVisited < expirationDate)) // i.e. entry is expired
    {
        q->removeEntry(m_history.begin());

        if (m_history.isEmpty())
            break;
	entry = m_history.first();
    }
}

static QString dbusService()
{
    return QDBusConnection::sessionBus().baseService();
}

void KonqHistoryProvider::emitAddToHistory(const KonqHistoryEntry& entry)
{
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    entry.save(stream, KonqHistoryEntry::MarshalUrlAsStrings);
    stream << dbusService();
    // Protection against very long urls (like data:)
    if (data.size() > 4096)
        return;
    emit d->notifyHistoryEntry(data);
}

void KonqHistoryProvider::emitRemoveFromHistory(const KUrl& url)
{
    emit d->notifyRemove(url.url());
}

void KonqHistoryProvider::emitRemoveListFromHistory(const KUrl::List& urls)
{
    emit d->notifyRemoveList(urls.toStringList());
}

void KonqHistoryProvider::emitClear()
{
    emit d->notifyClear();
}

void KonqHistoryProvider::emitSetMaxCount(int count)
{
    emit d->notifyMaxCount(count);
}

void KonqHistoryProvider::emitSetMaxAge(int days)
{
    emit d->notifyMaxAge(days);
}

/**
 * Returns whether the D-Bus call we are handling was a call from us self
 */
static bool isSenderOfSignal(const QDBusMessage& msg)
{
    return dbusService() == msg.service();
}

void KonqHistoryProviderPrivate::slotNotifyHistoryEntry(const QByteArray& data)
{
    KonqHistoryEntry e;
    QDataStream stream(const_cast<QByteArray *>(&data), QIODevice::ReadOnly);

    //This is important - we need to switch to a consistent marshalling format for
    //communicating between different konqueror instances. Since during an upgrade
    //some "old" copies may still running, we use the old format for the DBUS transfers.
    //This doesn't make that much difference performance-wise for single entries anyway.
    e.load(stream, KonqHistoryEntry::MarshalUrlAsStrings);
    //kDebug(1202) << "Got new entry from Broadcast:" << e.url;

    KonqHistoryList::iterator existingEntry = q->findEntry(e.url);
    QString urlString = e.url.url();
    const bool newEntry = existingEntry == m_history.end();

    KonqHistoryEntry entry;

    if (!newEntry) {
        entry = *existingEntry;
    } else { // create a new history entry
	entry.url = e.url;
	entry.firstVisited = e.firstVisited;
	entry.numberOfTimesVisited = 0; // will get set to 1 below
	q->KParts::HistoryProvider::insert(urlString);
    }

    if (!e.typedUrl.isEmpty())
	entry.typedUrl = e.typedUrl;
    if (!e.title.isEmpty())
	entry.title = e.title;
    entry.numberOfTimesVisited += e.numberOfTimesVisited;
    entry.lastVisited = e.lastVisited;

    if (newEntry)
        m_history.append(entry);
    else {
        *existingEntry = entry;
    }

    adjustSize();

    q->finishAddingEntry(entry, isSenderOfSignal(message()));

    emit q->entryAdded(entry);
}

void KonqHistoryProviderPrivate::slotNotifyMaxCount(int count)
{
    m_maxCount = count;
    // TODO clearPending();
    adjustSize();

    KConfigGroup cs(konqConfig(), "HistorySettings");
    cs.writeEntry("Maximum of History entries", m_maxCount);

    if (isSenderOfSignal(message())) {
	saveHistory();
	cs.sync();
    }
}

void KonqHistoryProviderPrivate::slotNotifyMaxAge(int days)
{
    m_maxAgeDays = days;
    // TODO clearPending();
    adjustSize();

    KConfigGroup cs(konqConfig(), "HistorySettings");
    cs.writeEntry("Maximum age of History entries", m_maxAgeDays);

    if (isSenderOfSignal(message())) {
	saveHistory();
	cs.sync();
    }
}

void KonqHistoryProviderPrivate::slotNotifyClear()
{
    m_history.clear();

    if (isSenderOfSignal(message()))
	saveHistory();

    q->KParts::HistoryProvider::clear(); // also emits the cleared() signal
}

void KonqHistoryProviderPrivate::slotNotifyRemove(const QString& urlStr)
{
    KUrl url(urlStr);

    KonqHistoryList::iterator existingEntry = q->findEntry(url);
    if (existingEntry != m_history.end()) {
        q->removeEntry(existingEntry);
	if (isSenderOfSignal(message())) {
	    saveHistory();
        }
    }
}

void KonqHistoryProviderPrivate::slotNotifyRemoveList(const QStringList& urls)
{
    bool doSave = false;
    QStringList::const_iterator it = urls.begin();
    for (; it != urls.end(); ++it) {
        KUrl url(*it);
        KonqHistoryList::iterator existingEntry = m_history.findEntry(url);
        if (existingEntry != m_history.end()) {
            q->removeEntry(existingEntry);
            doSave = true;
	}
    }

    if (doSave && isSenderOfSignal(message())) {
        saveHistory();
    }
}

void KonqHistoryProvider::removeEntry(KonqHistoryList::iterator existingEntry)
{
    const KonqHistoryEntry entry = *existingEntry; // make copy, due to erase call below
    const QString urlString = entry.url.url();

    KParts::HistoryProvider::remove(urlString);

    d->m_history.erase(existingEntry);
    emit entryRemoved(entry);
}

int KonqHistoryProvider::maxCount() const
{
     return d->m_maxCount;
}

int KonqHistoryProvider::maxAge() const
{
     return d->m_maxAgeDays;
}

bool KonqHistoryProviderPrivate::saveHistory()
{
    const QString filename = KStandardDirs::locateLocal("data", QLatin1String("konqueror/konq_history"));
    KSaveFile file(filename);
    if (!file.open()) {
        kWarning() << "Can't open " << file.fileName() ;
        return false;
    }

    QDataStream fileStream (&file);
    fileStream << KonqHistoryLoader::historyVersion();

    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);

    QListIterator<KonqHistoryEntry> it(m_history);
    while (it.hasNext()) {
        //We use QUrl for marshalling URLs in entries in the V4
        //file format
        it.next().save(stream, KonqHistoryEntry::NoFlags);
    }

    quint32 crc = crc32(0, reinterpret_cast<unsigned char *>(data.data()), data.size());
    fileStream << crc << data;

    file.finalize(); //check for error here?

    return true;
}

KonqHistoryList::iterator KonqHistoryProvider::findEntry(const KUrl& url)
{
    // small optimization (dict lookup) for items _not_ in our history
    if (!KParts::HistoryProvider::contains(url.url()))
        return d->m_history.end();
    return d->m_history.findEntry(url);
}

KonqHistoryList::const_iterator KonqHistoryProvider::constFindEntry(const KUrl& url) const
{
    // small optimization (dict lookup) for items _not_ in our history
    if (!KParts::HistoryProvider::contains(url.url()))
        return d->m_history.constEnd();
    return d->m_history.constFindEntry(url);
}

void KonqHistoryProvider::finishAddingEntry(const KonqHistoryEntry& entry, bool isSender)
{
    Q_UNUSED(entry); // this arg is used by konq's reimplementation
    if (isSender) {
	// we are the sender of the broadcast, so we save
	d->saveHistory();
    }
}

#include "moc_konq_historyprovider.cpp"
#include "konq_historyprovider.moc"
