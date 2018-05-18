/*
 *  main.cpp
 *
 *  Copyright (C) 1998 Luca Montecchiani <m.luca@usa.net>
 *  Copyright (C) 2015 David Edmundson <davidedmundson@kde.org>
 *  Copyright (C) 2018 David Edmundson <davidedmundson@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */
#include "main.h"

#include <unistd.h>
#include <time.h>

#include <kaboutdata.h>
#include <kdialog.h>
#include <kpluginfactory.h>
#include <kpluginloader.h>
#include <KLocalizedString>

#include "helper.h"
#include "backend.h"

#include <QDBusConnection>
#include <QDBusMessage>

K_PLUGIN_FACTORY(KlockModuleFactory, registerPlugin<KclockModule>();)
K_EXPORT_PLUGIN(KlockModuleFactory("kcmkclock"))


KclockModule::KclockModule(QObject *parent, const QVariantList &)
  : KQuickAddons::ConfigModule(parent)
{
  KAboutData *about =
  new KAboutData(QStringLiteral("kcm_clock"), i18n("KDE Clock Control Module"), QStringLiteral("1.0"),
                  QString(), KAboutLicense::GPL,
                  i18n("(c) 1996 - 2001 Luca Montecchiani"));

  about->addAuthor(i18n("Luca Montecchiani"), i18n("Original author"), QStringLiteral("m.luca@usa.net"));
  about->addAuthor(i18n("Paul Campbell"), i18n("Former maintainer"), QStringLiteral("paul@taniwha.com"));
  about->addAuthor(i18n("Benjamin Meyer"), i18n("Added NTP support"), QStringLiteral("ben+kcmclock@meyerhome.net"));
  about->addAuthor(i18n("David Edmundson"), i18n("Current maintainer"), QStringLiteral("davidedmundson@kde.org"));

  setAboutData( about );

  setButtons(Help|Apply);

  m_ntpEnabled = m_backend->ntpEnabled();
  m_timeZone = m_backend->timeZone();

  auto timer = new QTimer(this);
  timer->setInterval(1000);
  timer->setSingleShot(false);
  connect(timer, &QTimer::timeout, this, &KclockModule::timeChanged);

/*
    if (m_haveTimedated) {
//         setAuthAction(KAuth::Action(QStringLiteral("org.freedesktop.timedate1.set-time")));
    } else {
        //auth action name will be automatically guessed from the KCM name
        qWarning() << "Timedated not found, using legacy saving mode";
        setNeedsAuthorization(true);
    }*/
}



void KclockModule::save()
{
//   setDisabled(true);

  bool success = m_backend->save(m_ntpEnabled, m_dateTime, m_timeZone);

  //used by the Plasma timeengine for non-linux systems
  if (success) {
      QDBusMessage msg = QDBusMessage::createSignal(QStringLiteral("/org/kde/kcmshell_clock"), QStringLiteral("org.kde.kcmshell_clock"), QStringLiteral("clockUpdated"));
      QDBusConnection::sessionBus().send(msg);
  }


  // NOTE: super nasty hack #1
  // Try to work around time mismatch between KSystemTimeZones' update of local
  // timezone and reloading of data, so that the new timezone is taken into account.
  // The Ultimate solution to this would be if KSTZ emitted a signal when a new
  // local timezone was found.

  // setDisabled(false) happens in load(), since QTimer::singleShot is non-blocking
    if (false) { // was !mTimeDate
    QTimer::singleShot(5000, this, SLOT(load()));
  } else {
    load();
  }

}

void KclockModule::load()
{
    m_backend.reset(Backend::create());
    KQuickAddons::ConfigModule::load();
}

bool KclockModule::canNtp() const
{
    return m_backend->canNtp();
}

QDateTime KclockModule::dateTime() const
{
    if (m_dateTime.isNull()) {
        return QDateTime::currentDateTime();
    }
    return m_dateTime;
}

void KclockModule::setDateTime(const QDateTime &dateTime)
{
    m_dateTime = dateTime;
}

#include "main.moc"
