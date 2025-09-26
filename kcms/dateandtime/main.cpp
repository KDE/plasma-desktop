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

#include <KAuth/Action>
#include <KAuth/ExecuteJob>

#include "timedated_interface.h"

K_PLUGIN_CLASS_WITH_JSON(KclockModule, "kcm_clock.json")

KclockModule::KclockModule(QObject *parent, const KPluginMetaData &metaData)
    : KCModule(parent, metaData)
{
    QVBoxLayout *layout = new QVBoxLayout(widget());
    layout->setContentsMargins(0, 0, 0, 0);

    dtime = new Dtime(widget());
    layout->addWidget(dtime);
    connect(dtime, &Dtime::timeChanged, this, &KCModule::setNeedsSave);
    connect(dtime, &Dtime::selectedTimeZoneChanged, this, &KCModule::setNeedsSave);

    setButtons(Help | Apply);

    setAuthActionName(QStringLiteral("org.freedesktop.timedate1.set-time"));
}

bool KclockModule::timedatedSave()
{
    OrgFreedesktopTimedate1Interface timedateIface(QStringLiteral("org.freedesktop.timedate1"),
                                                   QStringLiteral("/org/freedesktop/timedate1"),
                                                   QDBusConnection::systemBus());

    // final arg in each method is "user-interaction" i.e whether it's OK for polkit to ask for auth

    // we cannot send requests up front then block for all replies as we need NTP to be disabled before we can make a call to SetTime
    // timedated processes these in parallel and will return an error otherwise

    auto reply = timedateIface.SetNTP(dtime->ntpEnabled(), true);
    reply.waitForFinished();
    if (reply.isError()) {
        if (reply.error().name() != QDBusError::errorString(QDBusError::AccessDenied)) {
            KMessageBox::error(widget(), i18n("Unable to change NTP settings"));
        }
        qWarning() << "Failed to enable NTP" << reply.error().name() << reply.error().message();
        return false;
    }

    if (!dtime->ntpEnabled()) {
        qint64 timeDiff = dtime->userTime().toMSecsSinceEpoch() - QDateTime::currentMSecsSinceEpoch();
        //*1000 for milliseconds -> microseconds
        auto reply = timedateIface.SetTime(timeDiff * 1000, true, true);
        reply.waitForFinished();
        if (reply.isError()) {
            if (reply.error().name() != QDBusError::errorString(QDBusError::AccessDenied)) {
                KMessageBox::error(widget(), i18n("Unable to set current time"));
            }
            qWarning() << "Failed to set current time" << reply.error().name() << reply.error().message();            
            return false;
        }
    }
    QString selectedTimeZone = dtime->selectedTimeZone();
    if (!selectedTimeZone.isEmpty()) {
        auto reply = timedateIface.SetTimezone(selectedTimeZone, true);
        reply.waitForFinished();
        if (reply.isError()) {
            if (reply.error().name() != QDBusError::errorString(QDBusError::AccessDenied)) {
                KMessageBox::error(widget(), i18n("Unable to set timezone"));
            }
            qWarning() << "Failed to set timezone" << reply.error().name() << reply.error().message();
            return false;
        }
    }

    return true;
}

void KclockModule::save()
{
    widget()->setDisabled(true);

    if (timedatedSave()) {
        QDBusMessage msg = QDBusMessage::createSignal(QStringLiteral("/org/kde/kcmshell_clock"), //
                                                      QStringLiteral("org.kde.kcmshell_clock"),
                                                      QStringLiteral("clockUpdated"));
        QDBusConnection::sessionBus().send(msg);
    }

    load();
}

void KclockModule::load()
{
    dtime->load();

    widget()->setDisabled(false);
}

#include "main.moc"

#include "moc_main.cpp"
