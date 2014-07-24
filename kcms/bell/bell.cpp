/*
  Copyright (c) 1997 Christian Czezatke (e9025461@student.tuwien.ac.at)
                1998 Bernd Wuebben <wuebben@kde.org>
                2000 Matthias Elter <elter@kde.org>
                2001 Carsten PFeiffer <pfeiffer@kde.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include <QCheckBox>
#include <QPushButton>


//Added by qt3to4:
#include <QVBoxLayout>
#include <QFormLayout>
#include <QBoxLayout>
#include <QGroupBox>

#include <kcmodule.h>
#include <kaboutdata.h>
#include <kconfig.h>
#include <kdialog.h>
#include <knotification.h>
#include <KLocalizedString>
#include <knuminput.h>

#include "bell.h"

#include <X11/Xlib.h>
#include <fixx11h.h>
#include <QX11Info>
#include <kpluginfactory.h>
#include <kpluginloader.h>
#include <kdemacros.h>



K_PLUGIN_FACTORY(KBellConfigFactory, registerPlugin<KBellConfig>();)
K_EXPORT_PLUGIN(KBellConfigFactory("kcmbell"))



extern "C"
{
  Q_DECL_EXPORT void kcminit_bell()
  {
    XKeyboardState kbd;
    XKeyboardControl kbdc;

    XGetKeyboardControl(QX11Info::display(), &kbd);

    KConfig _config( "kcmbellrc", KConfig::NoGlobals  );
    KConfigGroup config(&_config, "General");

    kbdc.bell_percent = config.readEntry("Volume", kbd.bell_percent);
    kbdc.bell_pitch = config.readEntry("Pitch", kbd.bell_pitch);
    kbdc.bell_duration = config.readEntry("Duration", kbd.bell_duration);
    XChangeKeyboardControl(QX11Info::display(),
                           KBBellPercent | KBBellPitch | KBBellDuration,
                           &kbdc);
  }
}

KBellConfig::KBellConfig(QWidget *parent, const QVariantList &args):
    KCModule(parent, args)
{
  QBoxLayout *layout = new QVBoxLayout(this);
  layout->setMargin(0);

  QGroupBox *box = new QGroupBox(i18n("Bell Settings"), this );
  QFormLayout *form = new QFormLayout();
  form->setVerticalSpacing(0);
  box->setLayout(form);
  layout->addWidget(box);

  m_useBell = new QCheckBox( i18n("&Use system bell instead of system notification" ), box );
  m_useBell->setWhatsThis( i18n("You can use the standard system bell (PC speaker) or a "
				  "more sophisticated system notification, see the "
				  "\"System Notifications\" control module for the "
				  "\"Something Special Happened in the Program\" event."));
  connect(m_useBell, SIGNAL(toggled(bool)), SLOT(useBell(bool)));
  form->addRow(m_useBell);

  setQuickHelp( i18n("<h1>System Bell</h1> Here you can customize the sound of the standard system bell,"
    " i.e. the \"beep\" you always hear when there is something wrong. Note that you can further"
    " customize this sound using the \"Accessibility\" control module; for example, you can choose"
    " a sound file to be played instead of the standard bell."));

  m_volume = new KIntNumInput(50, box);
  m_volume->setRange(0, 100, 5);
  m_volume->setSuffix("%");
  m_volume->setSteps(5,25);
  form->addRow(i18n("&Volume:"), m_volume);
  m_volume->setWhatsThis( i18n("Here you can customize the volume of the system bell. For further"
    " customization of the bell, see the \"Accessibility\" control module.") );

  m_pitch = new KIntNumInput(800, box);
  m_pitch->setRange(20, 2000, 20);
  m_pitch->setSuffix(i18n(" Hz"));
  m_pitch->setSteps(40,200);
  form->addRow(i18n("&Pitch:"), m_pitch);
  m_pitch->setWhatsThis( i18n("Here you can customize the pitch of the system bell. For further"
    " customization of the bell, see the \"Accessibility\" control module.") );

  m_duration = new KIntNumInput(100, box);
  m_duration->setRange(1, 1000, 50);
  m_duration->setSuffix(i18n(" msec"));
  m_duration->setSteps(20,100);
  form->addRow(i18n("&Duration:"), m_duration);
  m_duration->setWhatsThis( i18n("Here you can customize the duration of the system bell. For further"
    " customization of the bell, see the \"Accessibility\" control module.") );

  m_testButton = new QPushButton(i18n("&Test"), box);
  m_testButton->setObjectName("test");
  form->addRow(QString(), m_testButton);
  connect( m_testButton, SIGNAL(clicked()), SLOT(ringBell()));
  m_testButton->setWhatsThis( i18n("Click \"Test\" to hear how the system bell will sound using your changed settings.") );

  // watch for changes
  connect(m_volume, SIGNAL(valueChanged(int)), SLOT(changed()));
  connect(m_pitch, SIGNAL(valueChanged(int)), SLOT(changed()));
  connect(m_duration, SIGNAL(valueChanged(int)), SLOT(changed()));

  KAboutData *about =
    new KAboutData(QStringLiteral("kcmbell"), i18n("KDE Bell Control Module"), QStringLiteral("1.0"),
                  QString(), KAboutLicense::GPL,
                  i18n("(c) 1997 - 2001 Christian Czezatke, Matthias Elter"));

  about->addAuthor(i18n("Christian Czezatke"), i18n("Original author"), QStringLiteral("e9025461@student.tuwien.ac.at"));
  about->addAuthor(i18n("Bernd Wuebben"), QString(), QStringLiteral("wuebben@kde.org"));
  about->addAuthor(i18n("Matthias Elter"), i18n("Current maintainer"), QStringLiteral("elter@kde.org"));
  about->addAuthor(i18n("Carsten Pfeiffer"), QString(), QStringLiteral("pfeiffer@kde.org"));
  setAboutData(about);
}

void KBellConfig::load()
{
  XKeyboardState kbd;
  XGetKeyboardControl(QX11Info::display(), &kbd);

  m_volume->setValue(kbd.bell_percent);
  m_pitch->setValue(kbd.bell_pitch);
  m_duration->setValue(kbd.bell_duration);

  KConfig _cfg("kdeglobals", KConfig::NoGlobals);
  KConfigGroup cfg(&_cfg, "General");
  m_useBell->setChecked(cfg.readEntry("UseSystemBell", false));
  useBell(m_useBell->isChecked());
  emit changed(false);
}

void KBellConfig::save()
{
  XKeyboardControl kbd;

  int bellVolume = m_volume->value();
  int bellPitch = m_pitch->value();
  int bellDuration = m_duration->value();

  kbd.bell_percent = bellVolume;
  kbd.bell_pitch = bellPitch;
  kbd.bell_duration = bellDuration;
  XChangeKeyboardControl(QX11Info::display(),
                         KBBellPercent | KBBellPitch | KBBellDuration,
                         &kbd);

  KConfig _config("kcmbellrc", KConfig::NoGlobals);
  KConfigGroup config(&_config, "General");
  config.writeEntry("Volume",bellVolume);
  config.writeEntry("Pitch",bellPitch);
  config.writeEntry("Duration",bellDuration);

  config.sync();

  KConfig _cfg("kdeglobals", KConfig::NoGlobals);
  KConfigGroup cfg(&_cfg, "General");
  cfg.writeEntry("UseSystemBell", m_useBell->isChecked());
  cfg.sync();

  if (!m_useBell->isChecked())
  {
    KConfig config("kaccessrc");

    KConfigGroup group = config.group("Bell");
    group.writeEntry("SystemBell", false);
    group.writeEntry("ArtsBell", false);
    group.writeEntry("VisibleBell", false);
  }
}

void KBellConfig::ringBell()
{
  if (!m_useBell->isChecked()) {
    KNotification::beep(QString(), this);
    return;
  }

  // store the old state
  XKeyboardState old_state;
  XGetKeyboardControl(QX11Info::display(), &old_state);

  // switch to the test state
  XKeyboardControl kbd;
  kbd.bell_percent = m_volume->value();
  kbd.bell_pitch = m_pitch->value();
  if (m_volume->value() > 0)
    kbd.bell_duration = m_duration->value();
  else
    kbd.bell_duration = 0;
  XChangeKeyboardControl(QX11Info::display(),
                         KBBellPercent | KBBellPitch | KBBellDuration,
                         &kbd);
  // ring bell
  XBell(QX11Info::display(),0);

  // restore old state
  kbd.bell_percent = old_state.bell_percent;
  kbd.bell_pitch = old_state.bell_pitch;
  kbd.bell_duration = old_state.bell_duration;
  XChangeKeyboardControl(QX11Info::display(),
                         KBBellPercent | KBBellPitch | KBBellDuration,
                         &kbd);
}

void KBellConfig::defaults()
{
  m_volume->setValue(100);
  m_pitch->setValue(800);
  m_duration->setValue(100);
  m_useBell->setChecked( false );
  useBell( false );
}

void KBellConfig::useBell( bool on )
{
  m_volume->setEnabled( on );
  m_pitch->setEnabled( on );
  m_duration->setEnabled( on );
  m_testButton->setEnabled( on );
  changed();
}

#include "bell.moc"
