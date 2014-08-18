/*
 *  main.cpp
 *
 *  Copyright (C) 1998 Luca Montecchiani <m.luca@usa.net>
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

//Added by qt3to4:
#include <QVBoxLayout>

#include <QtDBus/QtDBus>
#include <kaboutdata.h>
#include <kdialog.h>
#include <kpluginfactory.h>
#include <kpluginloader.h>
#include <kprocess.h>
#include <kmessagebox.h>

#include "dtime.h"
#include "helper.h"

#include <kauthaction.h>
#include <kauthexecutejob.h>

K_PLUGIN_FACTORY(KlockModuleFactory, registerPlugin<KclockModule>();)
K_EXPORT_PLUGIN(KlockModuleFactory("kcmkclock"))


KclockModule::KclockModule(QWidget *parent, const QVariantList &)
  : KCModule(parent)
{
  KAboutData *about =
  new KAboutData(QStringLiteral("kcmclock"), i18n("KDE Clock Control Module"), QStringLiteral("1.0"),
                  QString(), KAboutLicense::GPL,
                  i18n("(c) 1996 - 2001 Luca Montecchiani"));

  about->addAuthor(i18n("Luca Montecchiani"), i18n("Original author"), QStringLiteral("m.luca@usa.net"));
  about->addAuthor(i18n("Paul Campbell"), i18n("Current Maintainer"), QStringLiteral("paul@taniwha.com"));
  about->addAuthor(i18n("Benjamin Meyer"), i18n("Added NTP support"), QStringLiteral("ben+kcmclock@meyerhome.net"));
  setAboutData( about );
  setQuickHelp( i18n("<h1>Date & Time</h1> This control module can be used to set the system date and"
    " time. As these settings do not only affect you as a user, but rather the whole system, you"
    " can only change these settings when you start the System Settings as root. If you do not have"
    " the root password, but feel the system time should be corrected, please contact your system"
    " administrator."));

  QVBoxLayout *layout = new QVBoxLayout(this);
  layout->setMargin(0);
  layout->setSpacing(KDialog::spacingHint());

  dtime = new Dtime(this);
  layout->addWidget(dtime);
  connect(dtime, SIGNAL(timeChanged(bool)), this, SIGNAL(changed(bool)));

  setButtons(Help|Apply);

  setNeedsAuthorization(true);

  process = NULL;
}

void KclockModule::save()
{
  setDisabled(true);

  QVariantMap helperargs;
  dtime->save( helperargs );

  Action action = authAction();
  action.setArguments(helperargs);

  ExecuteJob *job = action.execute();

  if (!job->exec()) {
      KMessageBox::error(this, i18n("Unable to authenticate/execute the action: %1, %2", job->error(), job->errorString()));
  }
  else {
      QDBusMessage msg = QDBusMessage::createSignal("/org/kde/kcmshell_clock", "org.kde.kcmshell_clock", "clockUpdated");
      QDBusConnection::sessionBus().send(msg);
  }

  // NOTE: super nasty hack #1
  // Try to work around time mismatch between KSystemTimeZones' update of local
  // timezone and reloading of data, so that the new timezone is taken into account.
  // The Ultimate solution to this would be if KSTZ emitted a signal when a new
  // local timezone was found.
  QTimer::singleShot(5000, this, SLOT(load()));

  // setDisabled(false) happens in load(), since QTimer::singleShot is non-blocking
}

void KclockModule::load()
{
  dtime->load();
  setDisabled(false);
}

#include "main.moc"
