#include "backend.h"

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusReply>

#include <KMessageBox>
#include <KLocalizedString>

#include "timedated_interface.h"

//#include <kauthaction.h>
//#include <kauthexecutejob.h>


Backend* Backend::create()
{
    auto reply = QDBusConnection::systemBus().call(QDBusMessage::createMethodCall(QStringLiteral("org.freedesktop.DBus"),
                                                                                QStringLiteral("/org/freedesktop/DBus"),
                                                                                QStringLiteral("org.freedesktop.DBus"),
                                                                                QStringLiteral("ListActivatableNames")));

    if (!reply.arguments().isEmpty() &&  reply.arguments().first().value<QStringList>().contains(QStringLiteral("org.freedesktop.timedate1"))) {
        return new TimedatedBackend();
    } else {
        return nullptr; //new LegacyBackend();
    }
}

Backend::Backend() {}

Backend::~Backend() {}

TimedatedBackend::TimedatedBackend() :
    m_iface(new OrgFreedesktopTimedate1Interface(QStringLiteral("org.freedesktop.timedate1"), QStringLiteral("/org/freedesktop/timedate1"), QDBusConnection::systemBus()))
{
    m_canNtp = m_iface->canNTP();
    m_ntpEnabled = m_iface->nTP();
    m_timeZone = m_iface->timezone();
}

bool TimedatedBackend::canNtp() const
{
    return m_canNtp;
}

bool TimedatedBackend::ntpEnabled() const
{
    return m_ntpEnabled;
}

QString TimedatedBackend::timeZone() const
{
    return m_timeZone;
}

bool TimedatedBackend::save(bool newUseNtp, const QDateTime &newTime, const QString &newTimezone)
{
    bool rc;
    if (newUseNtp != m_ntpEnabled) {
        auto reply = m_iface->SetNTP(newUseNtp, true);
        reply.waitForFinished();
        if (reply.isError()) {
            KMessageBox::error(nullptr, i18n("Unable to change NTP settings"));
            qWarning() << "Failed to enable NTP" << reply.error().name() << reply.error().message();
            rc = false;
        }
    }

    if (!newUseNtp && !newTime.isNull()) {
        qint64 timeDiff = newTime.toMSecsSinceEpoch() - QDateTime::currentMSecsSinceEpoch();
        //*1000 for milliseconds -> microseconds
        auto reply = m_iface->SetTime(timeDiff * 1000, true, true);
        reply.waitForFinished();
        if (reply.isError()) {
            KMessageBox::error(nullptr, i18n("Unable to set current time"));
            qWarning() << "Failed to set current time" << reply.error().name() << reply.error().message();
            rc = false;
        }
    }
    if (newTimezone != m_timeZone && !newTimezone.isEmpty()) {
        auto reply = m_iface->SetTimezone(newTimezone, true);
        reply.waitForFinished();
        if (reply.isError()) {
            KMessageBox::error(nullptr, i18n("Unable to set timezone"));
            qWarning() << "Failed to set timezone" << reply.error().name() << reply.error().message();
            rc = false;
        }
    }
    return rc;
}

//legacy backend crap

/*
 * bool KclockModule::kauthSave()
{
    bool rc;
//   QVariantMap helperargs;
//   helperargs[QStringLiteral("ntp")] = true;
// //   helperargs[QStringLiteral("ntpServers")] = dtime->ntpServers();
// //   helperargs[QStringLiteral("ntpEnabled")] = dtime->ntpEnabled();
//
//   if (!dtime->ntpEnabled()) {
//       QDateTime newTime = dtime->userTime();
// //       qDebug() << "Set date to " << dtime->userTime();
//       helperargs[QStringLiteral("date")] = true;
//       helperargs[QStringLiteral("newdate")] = QString::number(newTime.toTime_t());
//       helperargs[QStringLiteral("olddate")] = QString::number(::time(0));
//   }
//
// //   QString selectedTimeZone = dtime->selectedTimeZone();
//   if (!selectedTimeZone.isEmpty()) {
//     helperargs[QStringLiteral("tz")] = true;
//     helperargs[QStringLiteral("tzone")] = selectedTimeZone;
//   } else {
//     helperargs[QStringLiteral("tzreset")] = true; // make the helper reset the timezone
//   }
//
//   Action action = authAction();
//   action.setArguments(helperargs);
//
//   ExecuteJob *job = action.execute();
//   bool rc = job->exec();
//   if (!rc) {
//       KMessageBox::error(this, i18n("Unable to authenticate/execute the action: %1, %2", job->error(), job->errorString()));
//   }
  return rc;
}
*/

//loading
/*
      // The config is actually written to the system config, but the user does not have any local config,
        // since there is nothing writing it.
        KConfig _config( QStringLiteral("kcmclockrc"), KConfig::NoGlobals );
        KConfigGroup config(&_config, "NTP");
        timeServerList->clear();
        timeServerList->addItems(config.readEntry("servers",
            i18n("Public Time Server (pool.ntp.org),\
        asia.pool.ntp.org,\
        europe.pool.ntp.org,\
        north-america.pool.ntp.org,\
        oceania.pool.ntp.org")).split(',', QString::SkipEmptyParts));
        setDateTimeAuto->setChecked(config.readEntry("enabled", false));

        if (ntpUtility.isEmpty()) {
            timeServerList->setEnabled(false);
        }
        currentTimeZone  = KSystemTimeZones::local().name();

        */
