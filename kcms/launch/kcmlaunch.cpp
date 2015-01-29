/*
 *  Copyright (C) 2001 Rik Hemsley (rikkus) <rik@kde.org>
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
 */

#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QGroupBox>

#include <QVBoxLayout>
#include <QGridLayout>
#include <QSpinBox>

#include <KConfigGroup>
#include <KLocalizedString>
#include <kconfig.h>

#include "kcmlaunch.h"
#include "kwin_interface.h"
#include <KPluginFactory>

K_PLUGIN_FACTORY(LaunchFactory,
        registerPlugin<LaunchConfig>();
        )
static const int s_startupDefaultTimeout = 5;


LaunchConfig::LaunchConfig(QWidget * parent, const QVariantList &)
  : KCModule(parent)
{
    QVBoxLayout* Form1Layout = new QVBoxLayout( this );
    Form1Layout->setMargin( 0 );

    setQuickHelp( i18n ( "<h1>Launch Feedback</h1>"
     " You can configure the application-launch feedback here." ) );

    QGroupBox* GroupBox1 = new QGroupBox(i18n( "Bus&y Cursor" ));
    GroupBox1->setWhatsThis( i18n(
     "<h1>Busy Cursor</h1>\n"
     "KDE offers a busy cursor for application startup notification.\n"
     "To enable the busy cursor, select one kind of visual feedback\n"
     "from the combobox.\n"
     "It may occur, that some applications are not aware of this startup\n"
     "notification. In this case, the cursor stops blinking after the time\n"
     "given in the section 'Startup indication timeout'"));

    QGridLayout* GroupBox1Layout = new QGridLayout();
    GroupBox1->setLayout( GroupBox1Layout );
    Form1Layout->addWidget(GroupBox1);

    GroupBox1Layout->setColumnStretch( 1, 1 );

    cb_busyCursor = new QComboBox( GroupBox1);
    cb_busyCursor->setObjectName( "cb_busyCursor" );
    cb_busyCursor->insertItem( 0, i18n( "No Busy Cursor" ) );
    cb_busyCursor->insertItem( 1, i18n( "Passive Busy Cursor" ) );
    cb_busyCursor->insertItem( 2, i18n( "Blinking Cursor" ) );
    cb_busyCursor->insertItem( 3, i18n( "Bouncing Cursor" ) );
    GroupBox1Layout->addWidget( cb_busyCursor, 0, 0 );
    connect(cb_busyCursor, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), this, &LaunchConfig::slotBusyCursor);
    connect(cb_busyCursor, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), this, &LaunchConfig::checkChanged);

    lbl_cursorTimeout = new QLabel( GroupBox1 );
    lbl_cursorTimeout->setObjectName( "TextLabel1" );
    lbl_cursorTimeout->setText( i18n( "&Startup indication timeout:" ) );
    GroupBox1Layout->addWidget( lbl_cursorTimeout, 2, 0 );
    sb_cursorTimeout = new QSpinBox( GroupBox1);
    sb_cursorTimeout->setRange( 0, 99 );
    sb_cursorTimeout->setSuffix( i18n(" sec") );
    GroupBox1Layout->addWidget( sb_cursorTimeout, 2, 1 );
    lbl_cursorTimeout->setBuddy( sb_cursorTimeout );
    connect(sb_cursorTimeout, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &LaunchConfig::checkChanged);

    QGroupBox* GroupBox2 = new QGroupBox( i18n( "Taskbar &Notification" ) );
    GroupBox2->setWhatsThis( i18n("<H1>Taskbar Notification</H1>\n"
    "You can enable a second method of startup notification which is\n"
    "used by the taskbar where a button with a rotating hourglass appears,\n"
    "symbolizing that your started application is loading.\n"
    "It may occur, that some applications are not aware of this startup\n"
     "notification. In this case, the button disappears after the time\n"
     "given in the section 'Startup indication timeout'"));

    QGridLayout* GroupBox2Layout = new QGridLayout();
    GroupBox2->setLayout( GroupBox2Layout );
    Form1Layout->addWidget( GroupBox2 );
    GroupBox2Layout->setColumnStretch( 1, 1 );

    cb_taskbarButton = new QCheckBox( GroupBox2 );
    cb_taskbarButton->setObjectName( "cb_taskbarButton" );
    cb_taskbarButton->setText( i18n( "Enable &taskbar notification" ) );
    GroupBox2Layout->addWidget( cb_taskbarButton, 0, 0, 1, 2 );
    connect(cb_taskbarButton, &QCheckBox::toggled, this, &LaunchConfig::slotTaskbarButton);
    connect(cb_taskbarButton, &QCheckBox::toggled, this, &LaunchConfig::checkChanged);

    lbl_taskbarTimeout = new QLabel( GroupBox2 );
    lbl_taskbarTimeout->setObjectName( "TextLabel2" );
    lbl_taskbarTimeout->setText( i18n( "Start&up indication timeout:" ) );
    GroupBox2Layout->addWidget( lbl_taskbarTimeout, 1, 0 );
    sb_taskbarTimeout = new QSpinBox( GroupBox2);
    sb_taskbarTimeout->setRange( 0, 99 );
    sb_taskbarTimeout->setSuffix( i18n(" sec") );
    GroupBox2Layout->addWidget( sb_taskbarTimeout, 1, 1 );
    lbl_taskbarTimeout->setBuddy( sb_taskbarTimeout );
    connect(sb_taskbarTimeout, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &LaunchConfig::checkChanged);

    Form1Layout->addStretch();
}

LaunchConfig::~LaunchConfig()
{
}

  void
LaunchConfig::slotBusyCursor(int i)
{
    lbl_cursorTimeout->setEnabled( i != 0 );
    sb_cursorTimeout->setEnabled( i != 0 );
}

  void
LaunchConfig::slotTaskbarButton(bool b)
{
    lbl_taskbarTimeout->setEnabled( b );
    sb_taskbarTimeout->setEnabled( b );
}

  void
LaunchConfig::load()
{
  KConfig conf("klaunchrc", KConfig::NoGlobals);
  KConfigGroup c = conf.group("FeedbackStyle");

  bool busyCursor =
    c.readEntry("BusyCursor", (bool)(Default & BusyCursor));

  bool taskbarButton =
    c.readEntry("TaskbarButton", (bool)(Default & TaskbarButton));

  cb_taskbarButton->setChecked(taskbarButton);

  c= conf.group( "BusyCursorSettings" );
  sb_cursorTimeout->setValue( c.readEntry( "Timeout", s_startupDefaultTimeout ));
  bool busyBlinking =c.readEntry("Blinking", false);
  bool busyBouncing =c.readEntry("Bouncing", true);
  if ( !busyCursor )
     cb_busyCursor->setCurrentIndex(0);
  else if ( busyBlinking )
     cb_busyCursor->setCurrentIndex(2);
  else if ( busyBouncing )
     cb_busyCursor->setCurrentIndex(3);
  else
     cb_busyCursor->setCurrentIndex(1);

  c= conf.group( "TaskbarButtonSettings" );
  sb_taskbarTimeout->setValue( c.readEntry( "Timeout", s_startupDefaultTimeout ));

  slotBusyCursor( cb_busyCursor->currentIndex() );
  slotTaskbarButton( taskbarButton );

  emit changed( false );
}

  void
LaunchConfig::save()
{
  KConfig conf("klaunchrc", KConfig::NoGlobals);
  KConfigGroup  c = conf.group("FeedbackStyle");

  c.writeEntry("BusyCursor",   cb_busyCursor->currentIndex() != 0);
  c.writeEntry("TaskbarButton", cb_taskbarButton->isChecked());

  c = conf.group("BusyCursorSettings");
  c.writeEntry( "Timeout", sb_cursorTimeout->value());
  c.writeEntry("Blinking", cb_busyCursor->currentIndex() == 2);
  c.writeEntry("Bouncing", cb_busyCursor->currentIndex() == 3);

  c = conf.group("TaskbarButtonSettings");
  c.writeEntry( "Timeout", sb_taskbarTimeout->value());

  c.sync();

  emit changed( false );

  org::kde::kwin::Effects kwin("org.kde.KWin", "/Effects", QDBusConnection::sessionBus());
  kwin.reconfigureEffect("startupfeedback");
}

  void
LaunchConfig::defaults()
{
  cb_busyCursor->setCurrentIndex(3); // bouncing cursor
  cb_taskbarButton->setChecked( (bool)(Default & TaskbarButton) );

  sb_cursorTimeout->setValue( s_startupDefaultTimeout );
  sb_taskbarTimeout->setValue( s_startupDefaultTimeout );

  slotBusyCursor(3);
  slotTaskbarButton( (bool)(Default & TaskbarButton) );

  checkChanged();
}

  void
LaunchConfig::checkChanged()
{
  KConfig conf("klaunchrc", KConfig::NoGlobals);
  KConfigGroup  c = conf.group("FeedbackStyle");

  bool savedBusyCursor =
    c.readEntry("BusyCursor", (bool)(Default & BusyCursor));

  bool savedTaskbarButton =
    c.readEntry("TaskbarButton", (bool)(Default & TaskbarButton));

  c = conf.group("BusyCursorSettings");
  unsigned int savedCursorTimeout = c.readEntry( "Timeout", s_startupDefaultTimeout );
  bool savedBusyBlinking =c.readEntry("Blinking", false);
  bool savedBusyBouncing =c.readEntry("Bouncing", true);

  c = conf.group("TaskbarButtonSettings");
  unsigned int savedTaskbarTimeout = c.readEntry( "Timeout", s_startupDefaultTimeout );

  bool newBusyCursor =cb_busyCursor->currentIndex()!=0;

  bool newTaskbarButton =cb_taskbarButton->isChecked();

  bool newBusyBlinking= cb_busyCursor->currentIndex()==2;
  bool newBusyBouncing= cb_busyCursor->currentIndex()==3;

  unsigned int newCursorTimeout = sb_cursorTimeout->value();

  unsigned int newTaskbarTimeout = sb_taskbarTimeout->value();

  emit changed(
      savedBusyCursor     != newBusyCursor
      ||
      savedTaskbarButton  != newTaskbarButton
      ||
      savedCursorTimeout  != newCursorTimeout
      ||
      savedTaskbarTimeout != newTaskbarTimeout
      ||
      savedBusyBlinking != newBusyBlinking
      ||
      savedBusyBouncing != newBusyBouncing
    );
}

#include "kcmlaunch.moc"
