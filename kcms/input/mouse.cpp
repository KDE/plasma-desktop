/*
 * mouse.cpp
 *
 * Copyright (c) 1997 Patrick Dowler dowler@morgul.fsh.uvic.ca
 *
 * Layout management, enhancements:
 * Copyright (c) 1999 Dirk A. Mueller <dmuell@gmx.net>
 *
 * SC/DC/AutoSelect/ChangeCursor:
 * Copyright (c) 2000 David Faure <faure@kde.org>
 *
 * Double click interval, drag time & dist
 * Copyright (c) 2000 Bernd Gehrmann
 *
 * Large cursor support
 * Visual activation TODO: speed
 * Copyright (c) 2000 Rik Hemsley <rik@kde.org>
 *
 * White cursor support
 * TODO: give user the option to choose a certain cursor font
 * -> Theming
 *
 * General/Advanced tabs
 * Copyright (c) 2000 Brad Hughes <bhughes@trolltech.com>
 *
 * redesign for KDE 2.2
 * Copyright (c) 2001 Ralf Nolden <nolden@kde.org>
 *
 * Logitech mouse support
 * Copyright (C) 2004 Brad Hards <bradh@frogmouth.net>
 *
 * Requires the Qt widget libraries, available at no cost at
 * http://www.troll.no/
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
 */

#include <QCheckBox>
#include <QFormLayout>
#undef Below
#undef Above
#include <QSlider>
#include <QWhatsThis>
#include <QTabWidget>

#include <ktoolinvocation.h>
#include <klocale.h>
#include <kdialog.h>
#include <kconfig.h>
#include <kstandarddirs.h>
#include <kdebug.h>
#include <kaboutdata.h>
#include <KPluginFactory>
#include <KPluginLoader>

#include <config-workspace.h>

#include "mouse.h"

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <kglobalsettings.h>
#include <QX11Info>

#undef Below

#include "../migrationlib/kdelibs4config.h"

K_PLUGIN_FACTORY(MouseConfigFactory,
        registerPlugin<MouseConfig>(); // mouse
        )
K_EXPORT_PLUGIN(MouseConfigFactory("kcminput"))

MouseConfig::MouseConfig(QWidget *parent, const QVariantList &args)
  : KCModule(parent, args)
{


   setQuickHelp( i18n("<h1>Mouse</h1> This module allows you to choose various"
     " options for the way in which your pointing device works. Your"
     " pointing device may be a mouse, trackball, or some other hardware"
     " that performs a similar function."));

    QString wtstr;

    QBoxLayout *top = new QVBoxLayout(this);
    top->setMargin(0);

    tabwidget = new QTabWidget(this);
    top->addWidget(tabwidget);

    generalTab = new KMouseDlg(this);
    QButtonGroup *group = new QButtonGroup( generalTab );
    group->setExclusive( true );
    group->addButton( generalTab->singleClick );
    group->addButton( generalTab->doubleClick );

    tabwidget->addTab(generalTab, i18n("&General"));

    group = new QButtonGroup( generalTab );
    group->setExclusive( true );
    group->addButton( generalTab->rightHanded,RIGHT_HANDED );
    group->addButton( generalTab->leftHanded,LEFT_HANDED );

    connect(group, SIGNAL(buttonClicked(int)), this, SLOT(changed()));
    connect(group, SIGNAL(buttonClicked(int)), this, SLOT(slotHandedChanged(int)));

    wtstr = i18n("If you are left-handed, you may prefer to swap the"
         " functions of the left and right buttons on your pointing device"
         " by choosing the 'left-handed' option. If your pointing device"
         " has more than two buttons, only those that function as the"
         " left and right buttons are affected. For example, if you have"
         " a three-button mouse, the middle button is unaffected.");
    generalTab->handedBox->setWhatsThis( wtstr );

    connect(generalTab->doubleClick, SIGNAL(clicked()), SLOT(changed()));

    wtstr = i18n("The default behavior in KDE is to select and activate"
         " icons with a single click of the left button on your pointing"
         " device. This behavior is consistent with what you would expect"
         " when you click links in most web browsers. If you would prefer"
         " to select with a single click, and activate with a double click,"
         " check this option.");
    generalTab->doubleClick->setWhatsThis( wtstr );

    wtstr = i18n("Activates and opens a file or folder with a single click.");
    generalTab->singleClick->setWhatsThis( wtstr );


    connect(generalTab->cbAutoSelect, SIGNAL(clicked()), this, SLOT(changed()));

    wtstr = i18n("If you check this option, pausing the mouse pointer"
         " over an icon on the screen will automatically select that icon."
         " This may be useful when single clicks activate icons, and you"
         " want only to select the icon without activating it.");
    generalTab->cbAutoSelect->setWhatsThis( wtstr );

    wtstr = i18n("If you have checked the option to automatically select"
         " icons, this slider allows you to select how long the mouse pointer"
         " must be paused over the icon before it is selected.");
    generalTab->slAutoSelect->setWhatsThis( wtstr );

    connect(generalTab->slAutoSelect, SIGNAL(valueChanged(int)), this, SLOT(changed()));

    connect(generalTab->cb_pointershape, SIGNAL(clicked()), this, SLOT(changed()));

    connect(generalTab->singleClick, SIGNAL(clicked()), this, SLOT(changed()));
    connect(generalTab->singleClick, SIGNAL(clicked()), this, SLOT(slotClick()));
    connect(generalTab->singleClick, SIGNAL(clicked()), this, SLOT(slotSmartSliderEnabling()));

    connect( generalTab->doubleClick, SIGNAL(clicked()), this, SLOT(slotClick()) );
    connect( generalTab->cbAutoSelect, SIGNAL(clicked()), this, SLOT(slotClick()) );
    connect(generalTab->cbAutoSelect, SIGNAL(clicked()), this, SLOT(slotSmartSliderEnabling()));


    // Only allow setting reversing scroll polarity if we have scroll buttons
    unsigned char map[20];
    if ( XGetPointerMapping(QX11Info::display(), map, 20) >= 5 )
    {
      generalTab->cbScrollPolarity->setEnabled( true );
      generalTab->cbScrollPolarity->show();
    }
    else
    {
      generalTab->cbScrollPolarity->setEnabled( false );
      generalTab->cbScrollPolarity->hide();
    }
    connect(generalTab->cbScrollPolarity, SIGNAL(clicked()), this, SLOT(changed()));
    connect(generalTab->cbScrollPolarity, SIGNAL(clicked()), this, SLOT(slotScrollPolarityChanged()));

    // Advanced tab
    advancedTab = new QWidget(0);
    advancedTab->setObjectName("Advanced Tab");
    tabwidget->addTab(advancedTab, i18n("Advanced"));

    QFormLayout *lay = new QFormLayout(advancedTab);

    accel = new KDoubleNumInput(0.1, 20, 2, advancedTab, 0.1, 1);
    accel->setSuffix(i18n(" x"));
    lay->addRow(i18n("Pointer acceleration:"), accel);
    connect(accel, SIGNAL(valueChanged(double)), this, SLOT(changed()));

    wtstr = i18n("<p>This option allows you to change the relationship"
         " between the distance that the mouse pointer moves on the"
         " screen and the relative movement of the physical device"
         " itself (which may be a mouse, trackball, or some other"
         " pointing device.)</p><p>"
         " A high value for the acceleration will lead to large"
         " movements of the mouse pointer on the screen even when"
         " you only make a small movement with the physical device."
         " Selecting very high values may result in the mouse pointer"
         " flying across the screen, making it hard to control.</p>");
    accel->setWhatsThis( wtstr );

    thresh = new KIntNumInput(20, advancedTab);
    thresh->setRange(0,20,1);
    thresh->setSteps(1,1);
    lay->addRow(i18n("Pointer threshold:"), thresh);
    connect(thresh, SIGNAL(valueChanged(int)), this, SLOT(changed()));
    connect(thresh, SIGNAL(valueChanged(int)), this, SLOT(slotThreshChanged(int)));
    slotThreshChanged(thresh->value());

    wtstr = i18n("<p>The threshold is the smallest distance that the"
         " mouse pointer must move on the screen before acceleration"
         " has any effect. If the movement is smaller than the threshold,"
         " the mouse pointer moves as if the acceleration was set to 1X;</p><p>"
         " thus, when you make small movements with the physical device,"
         " there is no acceleration at all, giving you a greater degree"
         " of control over the mouse pointer. With larger movements of"
         " the physical device, you can move the mouse pointer"
         " rapidly to different areas on the screen.</p>");
    thresh->setWhatsThis( wtstr );

    // It would be nice if the user had a test field.
    // Selecting such values in milliseconds is not intuitive
    doubleClickInterval = new KIntNumInput(2000, advancedTab);
    doubleClickInterval->setRange(0, 2000, 100);
    doubleClickInterval->setSuffix(i18n(" msec"));
    doubleClickInterval->setSteps(100, 100);
    lay->addRow(i18n("Double click interval:"), doubleClickInterval);
    connect(doubleClickInterval, SIGNAL(valueChanged(int)), this, SLOT(changed()));

    wtstr = i18n("The double click interval is the maximal time"
         " (in milliseconds) between two mouse clicks which"
         " turns them into a double click. If the second"
         " click happens later than this time interval after"
         " the first click, they are recognized as two"
         " separate clicks.");
    doubleClickInterval->setWhatsThis( wtstr );

    dragStartTime = new KIntNumInput(2000, advancedTab);
    dragStartTime->setRange(0, 2000, 100);
    dragStartTime->setSuffix(i18n(" msec"));
    dragStartTime->setSteps(100, 100);
    lay->addRow(i18n("Drag start time:"), dragStartTime);
    connect(dragStartTime, SIGNAL(valueChanged(int)), this, SLOT(changed()));

    wtstr = i18n("If you click with the mouse (e.g. in a multi-line"
         " editor) and begin to move the mouse within the"
         " drag start time, a drag operation will be initiated.");
    dragStartTime->setWhatsThis( wtstr );

    dragStartDist = new KIntNumInput(20, advancedTab);
    dragStartDist->setRange(1, 20, 1);
    dragStartDist->setSteps(1,1);
    lay->addRow(i18n("Drag start distance:"), dragStartDist);
    connect(dragStartDist, SIGNAL(valueChanged(int)), this, SLOT(changed()));
    connect(dragStartDist, SIGNAL(valueChanged(int)), this, SLOT(slotDragStartDistChanged(int)));
    slotDragStartDistChanged(dragStartDist->value());

    wtstr = i18n("If you click with the mouse and begin to move the"
         " mouse at least the drag start distance, a drag"
         " operation will be initiated.");
    dragStartDist->setWhatsThis( wtstr );

    wheelScrollLines = new KIntNumInput(3, advancedTab);
    wheelScrollLines->setRange(1, 12, 1);
    wheelScrollLines->setSteps(1,1);
    lay->addRow(i18n("Mouse wheel scrolls by:"), wheelScrollLines);
    connect(wheelScrollLines, SIGNAL(valueChanged(int)), this, SLOT(changed()));
    connect(wheelScrollLines, SIGNAL(valueChanged(int)), SLOT(slotWheelScrollLinesChanged(int)));
    slotWheelScrollLinesChanged(wheelScrollLines->value());

    wtstr = i18n("If you use the wheel of a mouse, this value determines the number of lines to scroll for each wheel movement. Note that if this number exceeds the number of visible lines, it will be ignored and the wheel movement will be handled as a page up/down movement.");
    wheelScrollLines->setWhatsThis(wtstr);

{
  QWidget *mouse = new QWidget(this);
  mouse->setObjectName("Mouse Navigation");
  tabwidget->addTab(mouse, i18n("Mouse Navigation"));

  QFormLayout *form = new QFormLayout(mouse);

  mouseKeys = new QCheckBox(i18n("&Move pointer with keyboard (using the num pad)"), mouse);
  form->addRow(mouseKeys);

  mk_delay = new KIntNumInput(mouse);
  mk_delay->setRange(1, 1000, 50);
  mk_delay->setSuffix(i18n(" msec"));
  form->addRow(i18n("&Acceleration delay:"), mk_delay);

  mk_interval = new KIntNumInput(0, mouse);
  mk_interval->setRange(1, 1000, 10);
  mk_interval->setSuffix(i18n(" msec"));
  form->addRow(i18n("R&epeat interval:"), mk_interval);

  mk_time_to_max = new KIntNumInput(0, mouse);
  mk_time_to_max->setRange(100, 10000, 200);
  mk_time_to_max->setSuffix(i18n(" msec"));
  form->addRow(i18n("Acceleration &time:"), mk_time_to_max);

  mk_max_speed = new KIntNumInput(0, mouse);
  mk_max_speed->setRange(1, 2000, 20);
  mk_max_speed->setSuffix(i18n(" pixel/sec"));
  form->addRow(i18n("Ma&ximum speed:"), mk_max_speed);

  mk_curve = new KIntNumInput(0, mouse);
  mk_curve->setRange(-1000, 1000, 100);
  form->addRow(i18n("Acceleration &profile:"), mk_curve);

  connect(mouseKeys, SIGNAL(clicked()), this, SLOT(checkAccess()));
  connect(mouseKeys, SIGNAL(clicked()), this, SLOT(changed()));
  connect(mk_delay, SIGNAL(valueChanged(int)), this, SLOT(changed()));
  connect(mk_interval, SIGNAL(valueChanged(int)), this, SLOT(changed()));
  connect(mk_time_to_max, SIGNAL(valueChanged(int)), this, SLOT(changed()));
  connect(mk_max_speed, SIGNAL(valueChanged(int)), this, SLOT(changed()));
  connect(mk_curve, SIGNAL(valueChanged(int)), this, SLOT(changed()));
}

  settings = new MouseSettings;

  // This part is for handling features on Logitech USB mice.
  // It only works if libusb is available.
#ifdef HAVE_LIBUSB

  static const
  struct device_table {
      int idVendor;
      int idProduct;
      const char* Model;
      const char* Name;
      int flags;
  } device_table[] = {
      { VENDOR_LOGITECH, 0xC00E, "M-BJ58", "Wheel Mouse Optical", HAS_RES },
      { VENDOR_LOGITECH, 0xC00F, "M-BJ79", "MouseMan Traveler", HAS_RES },
      { VENDOR_LOGITECH, 0xC012, "M-BL63B", "MouseMan Dual Optical", HAS_RES },
      { VENDOR_LOGITECH, 0xC01B, "M-BP86", "MX310 Optical Mouse", HAS_RES },
      { VENDOR_LOGITECH, 0xC01D, "M-BS81A", "MX510 Optical Mouse", HAS_RES | HAS_SS | HAS_SSR },
      { VENDOR_LOGITECH, 0xC024, "M-BP82", "MX300 Optical Mouse", HAS_RES },
      { VENDOR_LOGITECH, 0xC025, "M-BP81A", "MX500 Optical Mouse", HAS_RES | HAS_SS | HAS_SSR },
      { VENDOR_LOGITECH, 0xC031, "M-UT58A", "iFeel Mouse (silver)", HAS_RES },
      { VENDOR_LOGITECH, 0xC501, "C-BA4-MSE", "Mouse Receiver", HAS_CSR },
      { VENDOR_LOGITECH, 0xC502, "C-UA3-DUAL", "Dual Receiver", HAS_CSR | USE_CH2},
      { VENDOR_LOGITECH, 0xC504, "C-BD9-DUAL", "Cordless Freedom Optical", HAS_CSR | USE_CH2 },
      { VENDOR_LOGITECH, 0xC505, "C-BG17-DUAL", "Cordless Elite Duo", HAS_SS | HAS_SSR | HAS_CSR | USE_CH2},
      { VENDOR_LOGITECH, 0xC506, "C-BF16-MSE", "MX700 Optical Mouse", HAS_SS | HAS_CSR },
      { VENDOR_LOGITECH, 0xC508, "C-BA4-MSE", "Cordless Optical TrackMan", HAS_SS | HAS_CSR },
      { VENDOR_LOGITECH, 0xC50B, "967300-0403", "Cordless MX Duo Receiver", HAS_SS|HAS_CSR },
      { VENDOR_LOGITECH, 0xC50E, "M-RAG97", "MX1000 Laser Mouse", HAS_SS | HAS_CSR },
      { VENDOR_LOGITECH, 0xC702, "C-UF15", "Receiver for Cordless Presenter", HAS_CSR },
      { 0, 0, 0, 0, 0 }
  };

  usb_init();
  usb_find_busses();
  usb_find_devices();

  struct usb_bus *bus;
  struct usb_device *dev;

  for (bus = usb_busses; bus; bus = bus->next) {
      for (dev = bus->devices; dev; dev = dev->next) {
	  for (int n = 0; device_table[n].idVendor; n++)
	      if ( (device_table[n].idVendor == dev->descriptor.idVendor) &&
		   (device_table[n].idProduct == dev->descriptor.idProduct) ) {
		  // OK, we have a device that appears to be one of the ones we support
		  LogitechMouse *mouse = new LogitechMouse( dev, device_table[n].flags, this, device_table[n].Name );
		  settings->logitechMouseList.append(mouse);
		  tabwidget->addTab( (QWidget*)mouse, device_table[n].Name );
	      }
      }
  }

#endif

  KAboutData* about = new KAboutData(QStringLiteral("kcmmouse"), i18n("Mouse"),
        QStringLiteral("1.0"), QString(), KAboutLicense::GPL, i18n("(c) 1997 - 2005 Mouse developers"));
  about->addAuthor(i18n("Patrick Dowler"));
  about->addAuthor(i18n("Dirk A. Mueller"));
  about->addAuthor(i18n("David Faure"));
  about->addAuthor(i18n("Bernd Gehrmann"));
  about->addAuthor(i18n("Rik Hemsley"));
  about->addAuthor(i18n("Brad Hughes"));
  about->addAuthor(i18n("Ralf Nolden"));
  about->addAuthor(i18n("Brad Hards"));
  setAboutData( about );
}

void MouseConfig::checkAccess()
{
  mk_delay->setEnabled(mouseKeys->isChecked());
  mk_interval->setEnabled(mouseKeys->isChecked());
  mk_time_to_max->setEnabled(mouseKeys->isChecked());
  mk_max_speed->setEnabled(mouseKeys->isChecked());
  mk_curve->setEnabled(mouseKeys->isChecked());
}


MouseConfig::~MouseConfig()
{
    delete settings;
}

double MouseConfig::getAccel()
{
  return accel->value();
}

void MouseConfig::setAccel(double val)
{
  accel->setValue(val);
}

int MouseConfig::getThreshold()
{
  return thresh->value();
}

void MouseConfig::setThreshold(int val)
{
  thresh->setValue(val);
}


int MouseConfig::getHandedness()
{
  if (generalTab->rightHanded->isChecked())
    return RIGHT_HANDED;
  else
    return LEFT_HANDED;
}

void MouseConfig::setHandedness(int val)
{
  generalTab->rightHanded->setChecked(false);
  generalTab->leftHanded->setChecked(false);
  if (val == RIGHT_HANDED){
    generalTab->rightHanded->setChecked(true);
    generalTab->mousePix->setPixmap(KStandardDirs::locate("data", "kcminput/pics/mouse_rh.png"));
  }
  else{
    generalTab->leftHanded->setChecked(true);
    generalTab->mousePix->setPixmap(KStandardDirs::locate("data", "kcminput/pics/mouse_lh.png"));
  }
  settings->m_handedNeedsApply = true;
}

void MouseConfig::load()
{
  KConfig config( "kcminputrc" );
  settings->load(&config);

  generalTab->rightHanded->setEnabled(settings->handedEnabled);
  generalTab->leftHanded->setEnabled(settings->handedEnabled);
  if ( generalTab->cbScrollPolarity->isEnabled() )
    generalTab->cbScrollPolarity->setEnabled(settings->handedEnabled);
  generalTab->cbScrollPolarity->setChecked( settings->reverseScrollPolarity );

  setAccel(settings->accelRate);
  setThreshold(settings->thresholdMove);
  setHandedness(settings->handed);

  doubleClickInterval->setValue(settings->doubleClickInterval);
  dragStartTime->setValue(settings->dragStartTime);
  dragStartDist->setValue(settings->dragStartDist);
  wheelScrollLines->setValue(settings->wheelScrollLines);

  generalTab->singleClick->setChecked( settings->singleClick );
  generalTab->doubleClick->setChecked(!settings->singleClick);
  generalTab->cb_pointershape->setChecked(settings->changeCursor);
  generalTab->cbAutoSelect->setChecked( settings->autoSelectDelay >= 0 );
  if ( settings->autoSelectDelay < 0 )
     generalTab->slAutoSelect->setValue( 0 );
  else
     generalTab->slAutoSelect->setValue( settings->autoSelectDelay );
  slotClick();


  KConfig ac("kaccessrc");

  KConfigGroup group = ac.group("Mouse");
  mouseKeys->setChecked(group.readEntry("MouseKeys", false));
  mk_delay->setValue(group.readEntry("MKDelay", 160));

  int interval = group.readEntry("MKInterval", 5);
  mk_interval->setValue(interval);

  // Default time to reach maximum speed: 5000 msec
  int time_to_max = group.readEntry("MKTimeToMax", (5000+interval/2)/interval);
  time_to_max = group.readEntry("MK-TimeToMax", time_to_max*interval);
  mk_time_to_max->setValue(time_to_max);

  // Default maximum speed: 1000 pixels/sec
  //     (The old default maximum speed from KDE <= 3.4
  //     (100000 pixels/sec) was way too fast)
  long max_speed = group.readEntry("MKMaxSpeed", interval);
  max_speed = max_speed * 1000 / interval;
  if (max_speed > 2000)
     max_speed = 2000;
  max_speed = group.readEntry("MK-MaxSpeed", int(max_speed));
  mk_max_speed->setValue(max_speed);

  mk_curve->setValue(group.readEntry("MKCurve", 0));

  checkAccess();

  emit changed(false);
}

void MouseConfig::save()
{
  settings->accelRate = getAccel();
  settings->thresholdMove = getThreshold();
  settings->handed = getHandedness();

  settings->doubleClickInterval = doubleClickInterval->value();
  settings->dragStartTime = dragStartTime->value();
  settings->dragStartDist = dragStartDist->value();
  settings->wheelScrollLines = wheelScrollLines->value();
  settings->singleClick = !generalTab->doubleClick->isChecked();
  settings->autoSelectDelay = generalTab->cbAutoSelect->isChecked()? generalTab->slAutoSelect->value():-1;
//  settings->changeCursor = generalTab->singleClick->isChecked();
  settings->changeCursor = generalTab->cb_pointershape->isChecked();
  settings->reverseScrollPolarity = generalTab->cbScrollPolarity->isChecked();

  settings->apply();
  KConfig config( "kcminputrc" );
  settings->save(&config);

  KConfig ac("kaccessrc");

  KConfigGroup group = ac.group("Mouse");

  int interval = mk_interval->value();
  group.writeEntry("MouseKeys", mouseKeys->isChecked());
  group.writeEntry("MKDelay", mk_delay->value());
  group.writeEntry("MKInterval", interval);
  group.writeEntry("MK-TimeToMax", mk_time_to_max->value());
  group.writeEntry("MKTimeToMax",
                (mk_time_to_max->value() + interval/2)/interval);
  group.writeEntry("MK-MaxSpeed", mk_max_speed->value());
  group.writeEntry("MKMaxSpeed",
					 (mk_max_speed->value()*interval + 500)/1000);
   group.writeEntry("MKCurve", mk_curve->value());
  group.sync();
  group.writeEntry("MKCurve", mk_curve->value());

  // restart kaccess
  KToolInvocation::startServiceByDesktopName("kaccess");

  emit changed(false);
}

void MouseConfig::defaults()
{
    setThreshold(2);
    setAccel(2);
    setHandedness(RIGHT_HANDED);
    generalTab->cbScrollPolarity->setChecked( false );
    doubleClickInterval->setValue(400);
    dragStartTime->setValue(500);
    dragStartDist->setValue(4);
    wheelScrollLines->setValue(3);
    generalTab->doubleClick->setChecked( !KDE_DEFAULT_SINGLECLICK );
    generalTab->cbAutoSelect->setChecked( KDE_DEFAULT_AUTOSELECTDELAY != -1 );
    generalTab->slAutoSelect->setValue( KDE_DEFAULT_AUTOSELECTDELAY == -1 ? 50 : KDE_DEFAULT_AUTOSELECTDELAY );
    generalTab->singleClick->setChecked( KDE_DEFAULT_SINGLECLICK );
    generalTab->cb_pointershape->setChecked(KDE_DEFAULT_CHANGECURSOR);
    slotClick();

  mouseKeys->setChecked(false);
  mk_delay->setValue(160);
  mk_interval->setValue(5);
  mk_time_to_max->setValue(5000);
  mk_max_speed->setValue(1000);
  mk_curve->setValue(0);
  
  checkAccess();

  changed();
}

void MouseConfig::slotClick()
{
  // Autoselect has a meaning only in single-click mode
  generalTab->cbAutoSelect->setEnabled(!generalTab->doubleClick->isChecked() || generalTab->singleClick->isChecked());
  // Delay has a meaning only for autoselect
  bool bDelay = generalTab->cbAutoSelect->isChecked() && ! generalTab->doubleClick->isChecked();
   generalTab->slAutoSelect->setEnabled( bDelay );
}

/** No descriptions */
void MouseConfig::slotHandedChanged(int val){
  if(val==RIGHT_HANDED)
    generalTab->mousePix->setPixmap(KStandardDirs::locate("data", "kcminput/pics/mouse_rh.png"));
  else
    generalTab->mousePix->setPixmap(KStandardDirs::locate("data", "kcminput/pics/mouse_lh.png"));
  settings->m_handedNeedsApply = true;
}

void MouseSettings::load(KConfig *config)
{
  int accel_num, accel_den, threshold;
  double accel;
  XGetPointerControl( QX11Info::display(),
              &accel_num, &accel_den, &threshold );
  accel = float(accel_num) / float(accel_den);

  // get settings from X server
  int h = RIGHT_HANDED;
  unsigned char map[20];
  num_buttons = XGetPointerMapping(QX11Info::display(), map, 20);

  handedEnabled = true;

  // ## keep this in sync with KGlobalSettings::mouseSettings
  if( num_buttons == 1 )
  {
      /* disable button remapping */
      handedEnabled = false;
  }
  else if( num_buttons == 2 )
  {
      if ( (int)map[0] == 1 && (int)map[1] == 2 )
        h = RIGHT_HANDED;
      else if ( (int)map[0] == 2 && (int)map[1] == 1 )
        h = LEFT_HANDED;
      else
        /* custom button setup: disable button remapping */
        handedEnabled = false;
  }
  else
  {
      middle_button = (int)map[1];
      if ( (int)map[0] == 1 && (int)map[2] == 3 )
    h = RIGHT_HANDED;
      else if ( (int)map[0] == 3 && (int)map[2] == 1 )
    h = LEFT_HANDED;
      else
    {
      /* custom button setup: disable button remapping */
      handedEnabled = false;
    }
  }

  KConfigGroup group = config->group("Mouse");
  double a = group.readEntry("Acceleration",-1.0);
  if (a == -1)
    accelRate = accel;
  else
    accelRate = a;

  int t = group.readEntry("Threshold",-1);
  if (t == -1)
    thresholdMove = threshold;
  else
    thresholdMove = t;

  QString key = group.readEntry("MouseButtonMapping");
  if (key == "RightHanded")
    handed = RIGHT_HANDED;
  else if (key == "LeftHanded")
    handed = LEFT_HANDED;
#ifdef __GNUC__
#warning was key == NULL how was this working? is key.isNull() what the coder meant?
#endif
  else if (key.isNull())
    handed = h;
  reverseScrollPolarity = group.readEntry( "ReverseScrollPolarity", false);
  m_handedNeedsApply = false;

  // SC/DC/AutoSelect/ChangeCursor
  group = config->group("KDE");
  doubleClickInterval = group.readEntry("DoubleClickInterval", 400);
  dragStartTime = group.readEntry("StartDragTime", 500);
  dragStartDist = group.readEntry("StartDragDist", 4);
  wheelScrollLines = group.readEntry("WheelScrollLines", 3);

  singleClick = group.readEntry("SingleClick", KDE_DEFAULT_SINGLECLICK);
  autoSelectDelay = group.readEntry("AutoSelectDelay", KDE_DEFAULT_AUTOSELECTDELAY);
  changeCursor = group.readEntry("ChangeCursor", KDE_DEFAULT_CHANGECURSOR);
}

void MouseConfig::slotThreshChanged(int value)
{
  thresh->setSuffix(i18np(" pixel", " pixels", value));
}

void MouseConfig::slotDragStartDistChanged(int value)
{
  dragStartDist->setSuffix(i18np(" pixel", " pixels", value));
}

void MouseConfig::slotWheelScrollLinesChanged(int value)
{
  wheelScrollLines->setSuffix(i18np(" line", " lines", value));
}

void MouseSettings::apply(bool force)
{
  XChangePointerControl( QX11Info::display(),
                         true, true, int(qRound(accelRate*10)), 10, thresholdMove);

  // 256 might seems extreme, but X has already been known to return 32, 
  // and we don't want to truncate things. Xlib limits the table to 256 bytes,
  // so it's a good uper bound..
  unsigned char map[256];
  num_buttons = XGetPointerMapping(QX11Info::display(), map, 256);

  int remap=(num_buttons>=1);
  if (handedEnabled && (m_handedNeedsApply || force)) {
      if( num_buttons == 1 )
      {
          map[0] = (unsigned char) 1;
      }
      else if( num_buttons == 2 )
      {
          if (handed == RIGHT_HANDED)
          {
              map[0] = (unsigned char) 1;
              map[1] = (unsigned char) 3;
          }
          else
          {
              map[0] = (unsigned char) 3;
              map[1] = (unsigned char) 1;
          }
      }
      else // 3 buttons and more
      {
          if (handed == RIGHT_HANDED)
          {
              map[0] = (unsigned char) 1;
              map[1] = (unsigned char) middle_button;
              map[2] = (unsigned char) 3;
          }
          else
          {
              map[0] = (unsigned char) 3;
              map[1] = (unsigned char) middle_button;
              map[2] = (unsigned char) 1;
          }
          if( num_buttons >= 5 )
          {
          // Apps seem to expect logical buttons 4,5 are the vertical wheel.
          // With mice with more than 3 buttons (not including wheel) the physical
          // buttons mapped to logical 4,5 may not be physical 4,5 , so keep
          // this mapping, only possibly reversing them.
              int pos;
              for( pos = 0; pos < num_buttons; ++pos )
                  if( map[pos] == 4 || map[pos] == 5 )
                      break;
              if( pos < num_buttons - 1 )
              {
                  map[pos] = reverseScrollPolarity ? (unsigned char) 5 : (unsigned char) 4;
                  map[pos+1] = reverseScrollPolarity ? (unsigned char) 4 : (unsigned char) 5;
              }
          }
      }
      int retval;
      if (remap)
          while ((retval=XSetPointerMapping(QX11Info::display(), map,
                                            num_buttons)) == MappingBusy)
              /* keep trying until the pointer is free */
          { };
      m_handedNeedsApply = false;
  }

  // This iterates through the various Logitech mice, if we have support.
  #ifdef HAVE_LIBUSB
  LogitechMouse *logitechMouse;
  Q_FOREACH( logitechMouse, logitechMouseList ) {
      logitechMouse->applyChanges();
  }
  #endif
}

void MouseSettings::save(KConfig *config)
{
  KConfigGroup group = config->group("Mouse");
  group.writeEntry("Acceleration",accelRate);
  group.writeEntry("Threshold",thresholdMove);
  if (handed == RIGHT_HANDED)
      group.writeEntry("MouseButtonMapping",QString("RightHanded"));
  else
      group.writeEntry("MouseButtonMapping",QString("LeftHanded"));
  group.writeEntry( "ReverseScrollPolarity", reverseScrollPolarity );

  Kdelibs4SharedConfig::syncConfigGroup(&group, "kinputrc");

  group = KConfigGroup(KSharedConfig::openConfig("kdeglobals"), "KDE");
  group.writeEntry("DoubleClickInterval", doubleClickInterval, KConfig::Persistent);
  group.writeEntry("StartDragTime", dragStartTime, KConfig::Persistent);
  group.writeEntry("StartDragDist", dragStartDist, KConfig::Persistent);
  group.writeEntry("WheelScrollLines", wheelScrollLines, KConfig::Persistent);
  group.writeEntry("SingleClick", singleClick, KConfig::Persistent);
  group.writeEntry("AutoSelectDelay", autoSelectDelay, KConfig::Persistent);
  group.writeEntry("ChangeCursor", changeCursor,KConfig::Persistent);

  Kdelibs4SharedConfig::syncConfigGroup(&group, "kdeglobals");

  // This iterates through the various Logitech mice, if we have support.
#ifdef HAVE_LIBUSB
  LogitechMouse *logitechMouse;
  Q_FOREACH( logitechMouse, logitechMouseList ) {
      logitechMouse->save(config);
  }
#endif
  config->sync();

  KGlobalSettings::self()->emitChange(KGlobalSettings::SettingsChanged, KGlobalSettings::SETTINGS_MOUSE);
}

void MouseConfig::slotScrollPolarityChanged()
{
  settings->m_handedNeedsApply = true;
}

void MouseConfig::slotSmartSliderEnabling()
{
  bool enabled = generalTab->singleClick->isChecked() ? generalTab->cbAutoSelect->isChecked() : false;
  generalTab->slAutoSelect->setEnabled(enabled);
}

#include "mouse.moc"
