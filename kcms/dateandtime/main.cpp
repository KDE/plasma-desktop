/*
    SPDX-FileCopyrightText: 1998 Luca Montecchiani <m.luca@usa.net>
    SPDX-FileCopyrightText: 2015 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "main.h"

#include <time.h>
#include <unistd.h>

#include <QVBoxLayout>

#include <KAboutData>
#include <KMessageBox>
#include <KPluginFactory>
#include <QDBusConnection>

#include "dtime.h"
#include "helper.h"

#include <KAuthAction>
#include <KAuthExecuteJob>

#include "timedated_interface.h"

K_PLUGIN_FACTORY(KlockModuleFactory, registerPlugin<KclockModule>();)

KclockModule::KclockModule(QWidget *parent, const QVariantList &)
    : KCModule(parent)
{
    auto reply = QDBusConnection::systemBus().call(QDBusMessage::createMethodCall(QStringLiteral("org.freedesktop.DBus"),
                                                                                  QStringLiteral("/org/freedesktop/DBus"),
                                                                                  QStringLiteral("org.freedesktop.DBus"),
                                                                                  QStringLiteral("ListActivatableNames")));

    if (!reply.arguments().isEmpty() && reply.arguments().constFirst().value<QStringList>().contains(QLatin1String("org.freedesktop.timedate1"))) {
        m_haveTimedated = true;
    }

    KAboutData *about = new KAboutData(QStringLiteral("kcmclock"),
                                       i18n("KDE Clock Control Module"),
                                       QStringLiteral("1.0"),
                                       QString(),
                                       KAboutLicense::GPL,
                                       i18n("(c) 1996 - 2001 Luca Montecchiani"));

    about->addAuthor(i18n("Luca Montecchiani"), i18n("Original author"), QStringLiteral("m.luca@usa.net"));
    about->addAuthor(i18n("Paul Campbell"), i18n("Current Maintainer"), QStringLiteral("paul@taniwha.com"));
    about->addAuthor(i18n("Benjamin Meyer"), i18n("Added NTP support"), QStringLiteral("ben+kcmclock@meyerhome.net"));
    setAboutData(about);
    setQuickHelp(
        i18n("<h1>Date & Time</h1> This control module can be used to set the system date and"
             " time. As these settings do not only affect you as a user, but rather the whole system, you"
             " can only change these settings when you start the System Settings as root. If you do not have"
             " the root password, but feel the system time should be corrected, please contact your system"
             " administrator."));

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));

    dtime = new Dtime(this, m_haveTimedated);
    layout->addWidget(dtime);
    connect(dtime, SIGNAL(timeChanged(bool)), this, SIGNAL(changed(bool)));

    setButtons(Help | Apply);

    if (m_haveTimedated) {
        setAuthAction(KAuth::Action(QStringLiteral("org.freedesktop.timedate1.set-time")));
    } else {
        // auth action name will be automatically guessed from the KCM name
        qWarning() << "Timedated not found, using legacy saving mode";
        setNeedsAuthorization(true);
    }
}

bool KclockModule::kauthSave()
{
    QVariantMap helperargs;
    helperargs[QStringLiteral("ntp")] = true;
    helperargs[QStringLiteral("ntpServers")] = dtime->ntpServers();
    helperargs[QStringLiteral("ntpEnabled")] = dtime->ntpEnabled();

    if (!dtime->ntpEnabled()) {
        QDateTime newTime = dtime->userTime();
        qDebug() << "Set date to " << dtime->userTime();
        helperargs[QStringLiteral("date")] = true;
        helperargs[QStringLiteral("newdate")] = QString::number(newTime.toTime_t());
        helperargs[QStringLiteral("olddate")] = QString::number(::time(nullptr));
    }

    QString selectedTimeZone = dtime->selectedTimeZone();
    if (!selectedTimeZone.isEmpty()) {
        helperargs[QStringLiteral("tz")] = true;
        helperargs[QStringLiteral("tzone")] = selectedTimeZone;
    } else {
        helperargs[QStringLiteral("tzreset")] = true; // make the helper reset the timezone
    }

    Action action = authAction();
    action.setArguments(helperargs);

    ExecuteJob *job = action.execute();
    bool rc = job->exec();
    if (!rc) {
        KMessageBox::error(this, i18n("Unable to authenticate/execute the action: %1, %2", job->error(), job->errorString()));
    }
    return rc;
}

bool KclockModule::timedatedSave()
{
    OrgFreedesktopTimedate1Interface timedateIface(QStringLiteral("org.freedesktop.timedate1"),
                                                   QStringLiteral("/org/freedesktop/timedate1"),
                                                   QDBusConnection::systemBus());

    bool rc = true;
    // final arg in each method is "user-interaction" i.e whether it's OK for polkit to ask for auth

    // we cannot send requests up front then block for all replies as we need NTP to be disabled before we can make a call to SetTime
    // timedated processes these in parallel and will return an error otherwise

    auto reply = timedateIface.SetNTP(dtime->ntpEnabled(), true);
    reply.waitForFinished();
    if (reply.isError()) {
        KMessageBox::error(this, i18n("Unable to change NTP settings"));
        qWarning() << "Failed to enable NTP" << reply.error().name() << reply.error().message();
        rc = false;
    }

    if (!dtime->ntpEnabled()) {
        qint64 timeDiff = dtime->userTime().toMSecsSinceEpoch() - QDateTime::currentMSecsSinceEpoch();
        //*1000 for milliseconds -> microseconds
        auto reply = timedateIface.SetTime(timeDiff * 1000, true, true);
        reply.waitForFinished();
        if (reply.isError()) {
            KMessageBox::error(this, i18n("Unable to set current time"));
            qWarning() << "Failed to set current time" << reply.error().name() << reply.error().message();
            rc = false;
        }
    }
    QString selectedTimeZone = dtime->selectedTimeZone();
    if (!selectedTimeZone.isEmpty()) {
        auto reply = timedateIface.SetTimezone(selectedTimeZone, true);
        reply.waitForFinished();
        if (reply.isError()) {
            KMessageBox::error(this, i18n("Unable to set timezone"));
            qWarning() << "Failed to set timezone" << reply.error().name() << reply.error().message();
            rc = false;
        }
    }

    return rc;
}

void KclockModule::save()
{
    setDisabled(true);

    bool success = false;
    if (m_haveTimedated) {
        success = timedatedSave();
    } else {
        success = kauthSave();
    }

    if (success) {
        QDBusMessage msg = QDBusMessage::createSignal(QStringLiteral("/org/kde/kcmshell_clock"), //
                                                      QStringLiteral("org.kde.kcmshell_clock"),
                                                      QStringLiteral("clockUpdated"));
        QDBusConnection::sessionBus().send(msg);
    }

    // NOTE: super nasty hack #1
    // Try to work around time mismatch between KSystemTimeZones' update of local
    // timezone and reloading of data, so that the new timezone is taken into account.
    // The Ultimate solution to this would be if KSTZ emitted a signal when a new
    // local timezone was found.

    // setDisabled(false) happens in load(), since QTimer::singleShot is non-blocking
    if (!m_haveTimedated) {
        QTimer::singleShot(5000, this, SLOT(load()));
    } else {
        load();
    }
}

void KclockModule::load()
{
    dtime->load();
    setDisabled(false);
}

#include "main.moc"
