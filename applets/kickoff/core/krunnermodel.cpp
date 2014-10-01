/*
    Copyright 2009 Ivan Cukic <ivan.cukic+kde@gmail.com>

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

// Own
#include "krunnermodel.h"

// Qt
#include <QBasicTimer>
#include <QDebug>
#include <QList>
#include <QMimeData>
#include <QString>
#include <QTimerEvent>

// KDE
#include <KService>
#include <KStandardDirs>
#include <KLocalizedString>
#include <KUrl>
//#include <KRunner/AbstractRunner>
//#include <KRunner/RunnerManager>
#include <krunner/abstractrunner.h>
#include <krunner/runnermanager.h>


// Local
#include "recentapplications.h"

#define DELAY_TIME 50

using namespace Kickoff;

Plasma::RunnerManager * _runnerManager = NULL;
Plasma::RunnerManager * runnerManager() {
    if (_runnerManager == NULL) {
        KConfigGroup conf =
            componentData().config()->group("KRunner");

        conf.writeEntry("loadAll", false);
        _runnerManager = new Plasma::RunnerManager(conf);

        // initializing allowed runners
        QStringList allowed;
        allowed
            << "places"
            << "shell"
            << "services"
            << "bookmarks"
            << "recentdocuments"
            << "locations"
            << "baloosearch";

        _runnerManager->setAllowedRunners(allowed);

        conf.sync();

        // runner:  "konquerorsessions"   "Konqueror Sessions"
        // runner:  "desktopsessions"   "Desktop Sessions"
        // runner:  "places"   "Places"
        // runner:  "windows"   "Windows"
        // runner:  "plasma-desktop"   "Plasma Desktop Shell"
        // runner:  "services"   "Applications"
        // runner:  "PowerDevil"   "PowerDevil"
        // runner:  "browserhistory"   "Web Browser History"
        // runner:  "shell"   "Command Line"
        // runner:  "katesessions"   "Kate Sessions"
        // runner:  "locations"   "Locations"
        // runner:  "webshortcuts"   "Web Shortcuts"
        // runner:  "kabccontacts"   "Contacts"
        // runner:  "konsolesessions"   "Konsole Sessions"
        // runner:  "recentdocuments"   "Recent Documents"
        // runner:  "calculator"   "Calculator"
        // runner:  "amarok"   "Amarok"
        // runner:  "wikipedia"   "Wikipedia"
        // runner:  "kopete"   "Kopete Contact Runner"
        // runner:  "bookmarks"   "Bookmarks"
        // runner:  "unitconverter"   "Unit Converter"
    }
    return _runnerManager;
}

KService::Ptr Kickoff::serviceForUrl(const KUrl & url)
{
    QString runner = url.host();
    QString id = url.path();

    if (id.startsWith(QLatin1Char('/'))) {
        id = id.remove(0, 1);
    }

    if (runner != QLatin1String("services")) {
        return KService::Ptr(NULL);
    }

    // URL path example: services_kde4-kate.desktop
    // or: services_firefox.desktop
    id.remove("services_");

    return KService::serviceByStorageId(id);
}


class ResultItem {
public:
    ResultItem()
    {
    }

    ResultItem(QIcon _icon, QString _name, QString _description, QVariant _data)
        : icon(_icon),
          name(_name),
          description(_description),
          data(_data)
    {
    }

    QIcon icon;
    QString name, description;
    QVariant data;
};

bool KRunnerItemHandler::openUrl(const KUrl& url)
{
    QString runner = url.host();
    QString id = url.path();
    if (id.startsWith(QLatin1Char('/'))) {
        id = id.remove(0, 1);
    }

    // Since krunner:// urls can't be added to recent applications,
    // we find the local .desktop entry.

    KService::Ptr service = serviceForUrl(url);
    if (service) {
        RecentApplications::self()->add(service);
    } else {
        qWarning() << "Failed to find service for" << url;
    }

    runnerManager()->run(id);
    return true;
}

class KRunnerModel::Private {
public:
    Private()
    {
    }

    ~Private()
    {
    }

    QBasicTimer searchDelay;
    QString searchQuery;
    DisplayOrder displayOrder;
};

KRunnerModel::KRunnerModel(QObject *parent)
        : KickoffModel(parent)
        , d(new Private())
{
    connect(runnerManager(),
            SIGNAL(matchesChanged(QList<Plasma::QueryMatch>)),
            this,
            SLOT(matchesChanged(QList<Plasma::QueryMatch>)));

    UrlItemLauncher::addGlobalHandler(UrlItemLauncher::ProtocolHandler, "krunner", new KRunnerItemHandler);
}

KRunnerModel::~KRunnerModel()
{
    delete d;
}

void KRunnerModel::setQuery(const QString& query)
{
    runnerManager()->reset();
    clear();

    d->searchQuery = query.trimmed();

    if (d->searchQuery.isEmpty()) {
        return;
    }

    d->searchDelay.start(DELAY_TIME, this);
}

void KRunnerModel::timerEvent(QTimerEvent * event)
{
    KickoffModel::timerEvent(event);

    if (event->timerId() == d->searchDelay.timerId()) {
        d->searchDelay.stop();
        runnerManager()->launchQuery(d->searchQuery);
    };
}

void KRunnerModel::setNameDisplayOrder(DisplayOrder displayOrder)
{
    d->displayOrder = displayOrder;
}

DisplayOrder KRunnerModel::nameDisplayOrder() const
{
   return d->displayOrder;
}

void KRunnerModel::matchesChanged(const QList< Plasma::QueryMatch > & m)
{
    QList< Plasma::QueryMatch > matches = m;

    qSort(matches.begin(), matches.end());

    clear();

    while (matches.size()) {
        Plasma::QueryMatch match = matches.takeLast();
        qDebug() << "matches " << QString("krunner://") + match.runner()->id() + "/" + match.id();
        QString mimeUrl;
        if (runnerManager()->mimeDataForMatch(match)) {
            mimeUrl = runnerManager()->mimeDataForMatch(match)->urls().first().toString();
        }
        appendRow(
            StandardItemFactory::createItem(
                match.icon(),
                match.text(),
                match.subtext(),
                QString("krunner://") + match.runner()->id() + "/" + match.id(),
                mimeUrl
                )
            );
    }
}

Qt::ItemFlags KRunnerModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags flags = KickoffModel::flags(index);

    if (index.isValid()) {
        KUrl url = data(index, UrlRole).toString();
        QString host = url.host();
        if (host != "services") {
            flags &= ~ ( Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled );
        }
    } else {
        flags = 0;
    }

    return flags;
}

QMimeData * KRunnerModel::mimeData(const QModelIndexList &indexes) const
{
    KUrl::List urls;

    foreach (const QModelIndex & index, indexes) {
        KUrl url = data(index, UrlRole).toString();

        KService::Ptr service = serviceForUrl(url);

        if (service) {
            urls << KUrl(service->entryPath());
        }
    }

    QMimeData *mimeData = new QMimeData();

    if (!urls.isEmpty()) {
        urls.populateMimeData(mimeData);
    }

    return mimeData;

}

#include "krunnermodel.moc"
