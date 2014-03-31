/**
 *  kcmaccess.cpp
 *
 *  Copyright (c) 2000 Matthias H�zer-Klpfel
 *
 */

#include "kcmaccess.h"

#include <stdlib.h>
#include <math.h>


#include <QTabWidget>
#include <QLabel>
#include <QCheckBox>
#include <QLineEdit>
#include <QRadioButton>
#include <QX11Info>
#include <QtDBus/QtDBus>

//Added by qt3to4:
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QFileDialog>
#include <KLocalizedString>

#include <KPluginFactory>
#include <kcombobox.h>
#include <kstandarddirs.h>
#include <kcolorbutton.h>
#include <kfiledialog.h>
#include <kapplication.h>
#include <kaboutdata.h>
#include <kshortcut.h>
#include <knotifyconfigwidget.h>
#include <kkeyserver.h>
#include <kdemacros.h>
#include <KConfigGroup>
#include <KGlobal>

#include <X11/Xlib.h>
#include <X11/XKBlib.h>
#define XK_MISCELLANY
#define XK_XKB_KEYS
#include <X11/keysymdef.h>
#include <ktoolinvocation.h>

K_PLUGIN_FACTORY(KAccessConfigFactory, registerPlugin<KAccessConfig>();)
K_EXPORT_PLUGIN(KAccessConfigFactory("kcmaccess"))

QString mouseKeysShortcut (Display *display) {
  // Calculate the keycode
  KeySym sym = XK_MouseKeys_Enable;
  KeyCode code = XKeysymToKeycode(display, sym);
  if (code == 0) {
     sym = XK_Pointer_EnableKeys;
     code = XKeysymToKeycode(display, sym);
     if (code == 0)
        return ""; // No shortcut available?
  }

  // Calculate the modifiers by searching the keysym in the X keyboard mapping
  XkbDescPtr xkbdesc = XkbGetMap(display, XkbKeyTypesMask | XkbKeySymsMask, XkbUseCoreKbd);
  if (!xkbdesc)
     return ""; // Failed to obtain the mapping from server

  bool found = false;
  unsigned char modifiers = 0;
  int groups = XkbKeyNumGroups(xkbdesc, code);
  for (int grp = 0; grp < groups && !found; grp++)
  {
     int levels = XkbKeyGroupWidth(xkbdesc, code, grp);
     for (int level = 0; level < levels && !found; level++)
     {
        if (sym == XkbKeySymEntry(xkbdesc, code, level, grp))
        {
           // keysym found => determine modifiers
           int typeIdx = xkbdesc->map->key_sym_map[code].kt_index[grp];
           XkbKeyTypePtr type = &(xkbdesc->map->types[typeIdx]);
           for (int i = 0; i < type->map_count && !found; i++)
           {
              if (type->map[i].active && (type->map[i].level == level))
              {
                 modifiers = type->map[i].mods.mask;
                 found = true;
              }
           }
        }
     }
  }
  XkbFreeClientMap (xkbdesc, 0, true);

  if (!found)
     return ""; // Somehow the keycode -> keysym mapping is flawed

  XEvent ev;
  ev.type = KeyPress;
  ev.xkey.display = display;
  ev.xkey.keycode = code;
  ev.xkey.state = 0;
  int key;
  KKeyServer::xEventToQt(&ev, &key);
  QString keyname = QKeySequence(key).toString();

  unsigned int AltMask   = KKeyServer::modXAlt();
  unsigned int WinMask   = KKeyServer::modXMeta();
  unsigned int NumMask   = KKeyServer::modXNumLock();
  unsigned int ScrollMask= KKeyServer::modXScrollLock();

  unsigned int MetaMask  = XkbKeysymToModifiers (display, XK_Meta_L);
  unsigned int SuperMask = XkbKeysymToModifiers (display, XK_Super_L);
  unsigned int HyperMask = XkbKeysymToModifiers (display, XK_Hyper_L);
  unsigned int AltGrMask = XkbKeysymToModifiers (display, XK_Mode_switch)
                         | XkbKeysymToModifiers (display, XK_ISO_Level3_Shift)
                         | XkbKeysymToModifiers (display, XK_ISO_Level3_Latch)
                         | XkbKeysymToModifiers (display, XK_ISO_Level3_Lock);

  unsigned int mods = ShiftMask | ControlMask | AltMask | WinMask
                    | LockMask | NumMask | ScrollMask;

  AltGrMask &= ~mods;
  MetaMask  &= ~(mods | AltGrMask);
  SuperMask &= ~(mods | AltGrMask | MetaMask);
  HyperMask &= ~(mods | AltGrMask | MetaMask | SuperMask);

  if ((modifiers & AltGrMask) != 0)
    keyname = i18n("AltGraph") + '+' + keyname;
  if ((modifiers & HyperMask) != 0)
    keyname = i18n("Hyper") + '+' + keyname;
  if ((modifiers & SuperMask) != 0)
    keyname = i18n("Super") + '+' + keyname;
  if ((modifiers & WinMask) != 0)
    keyname = QKeySequence(Qt::META).toString() + '+' + keyname;
  if ((modifiers & AltMask) != 0)
    keyname = QKeySequence(Qt::ALT).toString() + '+' + keyname;
  if ((modifiers & ControlMask) != 0)
    keyname = QKeySequence(Qt::CTRL).toString() + '+' + keyname;
  if ((modifiers & ShiftMask) != 0)
    keyname = QKeySequence(Qt::SHIFT).toString() + '+' + keyname;

  QString result;
  if ((modifiers & ScrollMask) != 0)
    if ((modifiers & LockMask) != 0)
      if ((modifiers & NumMask) != 0)
        result = i18n("Press %1 while NumLock, CapsLock and ScrollLock are active", keyname);
      else
        result = i18n("Press %1 while CapsLock and ScrollLock are active", keyname);
    else if ((modifiers & NumMask) != 0)
        result = i18n("Press %1 while NumLock and ScrollLock are active", keyname);
      else
        result = i18n("Press %1 while ScrollLock is active", keyname);
  else if ((modifiers & LockMask) != 0)
      if ((modifiers & NumMask) != 0)
        result = i18n("Press %1 while NumLock and CapsLock are active", keyname);
      else
        result = i18n("Press %1 while CapsLock is active", keyname);
    else if ((modifiers & NumMask) != 0)
        result = i18n("Press %1 while NumLock is active", keyname);
      else
        result = i18n("Press %1", keyname);

  return result;
}

KAccessConfig::KAccessConfig(QWidget *parent, const QVariantList& args)
  : KCModule(parent, args)
{

  KAboutData *about =
  new KAboutData(I18N_NOOP("kaccess"), QString(), i18n("KDE Accessibility Tool"),
                 QString(), QString(), KAboutData::License_GPL,
                 i18n("(c) 2000, Matthias Hoelzer-Kluepfel"));

  about->addAuthor(i18n("Matthias Hoelzer-Kluepfel"), i18n("Author") , QStringLiteral("hoelzer@kde.org"));

  setAboutData( about );

  QVBoxLayout *main = new QVBoxLayout(this);
  main->setMargin(0);
  QTabWidget *tab = new QTabWidget(this);
  main->addWidget(tab);

  // bell settings ---------------------------------------
  QWidget *bell = new QWidget(this);

  QVBoxLayout *vbox = new QVBoxLayout(bell);

  QGroupBox *grp = new QGroupBox(i18n("Audible Bell"), bell);
  QHBoxLayout *layout = new QHBoxLayout;
  grp->setLayout(layout);
  vbox->addWidget(grp);

  QVBoxLayout *vvbox = new QVBoxLayout();
  layout->addLayout( vvbox );

  systemBell = new QCheckBox(i18n("Use &system bell"), grp);
  vvbox->addWidget(systemBell);
  customBell = new QCheckBox(i18n("Us&e customized bell"), grp);
  vvbox->addWidget(customBell);
  systemBell->setWhatsThis( i18n("If this option is checked, the default system bell will be used. See the"
    " \"System Bell\" control module for how to customize the system bell."
    " Normally, this is just a \"beep\".") );
  customBell->setWhatsThis( i18n("<p>Check this option if you want to use a customized bell, playing"
    " a sound file. If you do this, you will probably want to turn off the system bell.</p><p> Please note"
    " that on slow machines this may cause a \"lag\" between the event causing the bell and the sound being played.</p>") );

  QHBoxLayout *hbox = new QHBoxLayout();
  vvbox->addLayout( hbox );
  hbox->addSpacing(24);
  soundEdit = new QLineEdit(grp);
  soundLabel = new QLabel(i18n("Sound &to play:"), grp);
  soundLabel->setBuddy(soundEdit);
  hbox->addWidget(soundLabel);
  hbox->addWidget(soundEdit);
  soundButton = new QPushButton(i18n("Browse..."), grp);
  hbox->addWidget(soundButton);
  QString wtstr = i18n("If the option \"Use customized bell\" is enabled, you can choose a sound file here."
    " Click \"Browse...\" to choose a sound file using the file dialog.");
  soundEdit->setWhatsThis( wtstr );
  soundLabel->setWhatsThis( wtstr );
  soundButton->setWhatsThis( wtstr );

  connect(soundButton, SIGNAL(clicked()), this, SLOT(selectSound()));

  connect(customBell, SIGNAL(clicked()), this, SLOT(checkAccess()));

  connect(systemBell, SIGNAL(clicked()), this, SLOT(configChanged()));
  connect(customBell, SIGNAL(clicked()), this, SLOT(configChanged()));
  connect(soundEdit, SIGNAL(textChanged(QString)), this, SLOT(configChanged()));

  // -----------------------------------------------------

  // visible bell ----------------------------------------
  grp = new QGroupBox(i18n("Visible Bell"), bell);
  layout = new QHBoxLayout;
  grp->setLayout(layout);
  vbox->addWidget(grp);

  vvbox = new QVBoxLayout();
  layout->addLayout( vvbox );

  visibleBell = new QCheckBox(i18n("&Use visible bell"), grp);
  vvbox->addWidget(visibleBell);
  visibleBell->setWhatsThis( i18n("This option will turn on the \"visible bell\", i.e. a visible"
    " notification shown every time that normally just a bell would occur. This is especially useful"
    " for deaf people.") );

  hbox = new QHBoxLayout();
  vvbox->addLayout(hbox);
  hbox->addSpacing(24);
  invertScreen = new QRadioButton(i18n("I&nvert screen"), grp);
  hbox->addWidget(invertScreen);
  hbox = new QHBoxLayout();
  vvbox->addLayout(hbox);
  invertScreen->setWhatsThis( i18n("All screen colors will be inverted for the amount of time specified below.") );
  hbox->addSpacing(24);
  flashScreen = new QRadioButton(i18n("F&lash screen"), grp);
  hbox->addWidget(flashScreen);
  flashScreen->setWhatsThis( i18n("The screen will turn to a custom color for the amount of time specified below.") );
  hbox->addSpacing(12);
  colorButton = new KColorButton(grp);
  colorButton->setFixedWidth(colorButton->sizeHint().height()*2);
  hbox->addWidget(colorButton);
  hbox->addStretch();
  colorButton->setWhatsThis( i18n("Click here to choose the color used for the \"flash screen\" visible bell.") );

  hbox = new QHBoxLayout();
  vvbox->addLayout(hbox);
  hbox->addSpacing(24);

  durationSlider = new KDoubleNumInput(grp);
  durationSlider->setRange(100, 2000, 100);
  durationSlider->setExponentRatio(2);
  durationSlider->setDecimals(0);
  durationSlider->setLabel(i18n("Duration:"));
  durationSlider->setSuffix(i18n(" msec"));
  hbox->addWidget(durationSlider);
  durationSlider->setWhatsThis( i18n("Here you can customize the duration of the \"visible bell\" effect being shown.") );

  connect(invertScreen, SIGNAL(clicked()), this, SLOT(configChanged()));
  connect(flashScreen, SIGNAL(clicked()), this, SLOT(configChanged()));
  connect(visibleBell, SIGNAL(clicked()), this, SLOT(configChanged()));
  connect(visibleBell, SIGNAL(clicked()), this, SLOT(checkAccess()));
  connect(colorButton, SIGNAL(clicked()), this, SLOT(changeFlashScreenColor()));

  connect(invertScreen, SIGNAL(clicked()), this, SLOT(invertClicked()));
  connect(flashScreen, SIGNAL(clicked()), this, SLOT(flashClicked()));

  connect(durationSlider, SIGNAL(valueChanged(double)), this, SLOT(configChanged()));

  vbox->addStretch();

  // -----------------------------------------------------

  tab->addTab(bell, i18n("&Bell"));


  // modifier key settings -------------------------------
  QWidget *modifiers = new QWidget(this);

  vbox = new QVBoxLayout(modifiers);

  grp = new QGroupBox(i18n("S&ticky Keys"), modifiers);
  layout = new QHBoxLayout;
  grp->setLayout(layout);
  vbox->addWidget(grp);

  vvbox = new QVBoxLayout();
  layout->addLayout(vvbox);

  stickyKeys = new QCheckBox(i18n("Use &sticky keys"), grp);
  vvbox->addWidget(stickyKeys);

  hbox = new QHBoxLayout();
  vvbox->addLayout(hbox);
  hbox->addSpacing(24);
  stickyKeysLock = new QCheckBox(i18n("&Lock sticky keys"), grp);
  hbox->addWidget(stickyKeysLock);

  hbox = new QHBoxLayout();
  vvbox->addLayout(hbox);
  hbox->addSpacing(24);
  stickyKeysAutoOff = new QCheckBox(i18n("Turn sticky keys off when two keys are pressed simultaneously"), grp);
  hbox->addWidget(stickyKeysAutoOff);

  hbox = new QHBoxLayout();
  vvbox->addLayout(hbox);
  hbox->addSpacing(24);
  stickyKeysBeep = new QCheckBox(i18n("Use system bell whenever a modifier gets latched, locked or unlocked"), grp);
  hbox->addWidget(stickyKeysBeep);

  grp = new QGroupBox(i18n("Locking Keys"), modifiers);
  layout = new QHBoxLayout;
  grp->setLayout(layout);
  vbox->addWidget(grp);

  vvbox = new QVBoxLayout();
  layout->addLayout(vvbox);

  toggleKeysBeep = new QCheckBox(i18n("Use system bell whenever a locking key gets activated or deactivated"), grp);
  vvbox->addWidget(toggleKeysBeep);

  kNotifyModifiers = new QCheckBox(i18n("Use KDE's system notification mechanism whenever a modifier or locking key changes its state"), grp);
  vvbox->addWidget(kNotifyModifiers);

  hbox = new QHBoxLayout();
  vvbox->addLayout(hbox);
  hbox->addStretch(1);
  kNotifyModifiersButton = new QPushButton(i18n("Configure &Notifications..."), grp);
  kNotifyModifiersButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  hbox->addWidget(kNotifyModifiersButton);

  connect(stickyKeys, SIGNAL(clicked()), this, SLOT(configChanged()));
  connect(stickyKeysLock, SIGNAL(clicked()), this, SLOT(configChanged()));
  connect(stickyKeysAutoOff, SIGNAL(clicked()), this, SLOT(configChanged()));
  connect(stickyKeys, SIGNAL(clicked()), this, SLOT(checkAccess()));

  connect(stickyKeysBeep, SIGNAL(clicked()), this, SLOT(configChanged()));
  connect(toggleKeysBeep, SIGNAL(clicked()), this, SLOT(configChanged()));
  connect(kNotifyModifiers, SIGNAL(clicked()), this, SLOT(configChanged()));
  connect(kNotifyModifiers, SIGNAL(clicked()), this, SLOT(checkAccess()));
  connect(kNotifyModifiersButton, SIGNAL(clicked()), this, SLOT(configureKNotify()));

  vbox->addStretch();

  tab->addTab(modifiers, i18n("&Modifier Keys"));

  // key filter settings ---------------------------------
  QWidget *filters = new QWidget(this);

  vbox = new QVBoxLayout(filters);
  grp = new QGroupBox(i18n("Slo&w Keys"), filters);
  layout = new QHBoxLayout;
  grp->setLayout(layout);
  vbox->addWidget(grp);

  vvbox = new QVBoxLayout();
  layout->addLayout(vvbox);

  slowKeys = new QCheckBox(i18n("&Use slow keys"), grp);
  vvbox->addWidget(slowKeys);

  hbox = new QHBoxLayout();
  vvbox->addLayout(hbox);
  hbox->addSpacing(24);
  slowKeysDelay = new KDoubleNumInput(grp);
  slowKeysDelay->setRange(50, 10000, 100);
  slowKeysDelay->setExponentRatio(2);
  slowKeysDelay->setDecimals(0);
  slowKeysDelay->setSuffix(i18n(" msec"));
  slowKeysDelay->setLabel(i18n("Acceptance dela&y:"), Qt::AlignVCenter|Qt::AlignLeft);
  hbox->addWidget(slowKeysDelay);

  hbox = new QHBoxLayout();
  vvbox->addLayout(hbox);
  hbox->addSpacing(24);
  slowKeysPressBeep = new QCheckBox(i18n("&Use system bell whenever a key is pressed"), grp);
  hbox->addWidget(slowKeysPressBeep);

  hbox = new QHBoxLayout();
  vvbox->addLayout(hbox);
  hbox->addSpacing(24);
  slowKeysAcceptBeep = new QCheckBox(i18n("&Use system bell whenever a key is accepted"), grp);
  hbox->addWidget(slowKeysAcceptBeep);

  hbox = new QHBoxLayout();
  vvbox->addLayout(hbox);
  hbox->addSpacing(24);
  slowKeysRejectBeep = new QCheckBox(i18n("&Use system bell whenever a key is rejected"), grp);
  hbox->addWidget(slowKeysRejectBeep);

  grp = new QGroupBox(i18n("Bounce Keys"), filters);
  layout= new QHBoxLayout;
  grp->setLayout(layout);
  vbox->addWidget(grp);

  vvbox = new QVBoxLayout();
  layout->addLayout( vvbox );

  bounceKeys = new QCheckBox(i18n("Use bou&nce keys"), grp);
  vvbox->addWidget(bounceKeys);

  hbox = new QHBoxLayout();
  vvbox->addLayout(hbox);
  hbox->addSpacing(24);
  bounceKeysDelay = new KDoubleNumInput(grp);
  bounceKeysDelay->setRange(100, 5000, 100);
  bounceKeysDelay->setExponentRatio(2);
  bounceKeysDelay->setDecimals(0);
  bounceKeysDelay->setSuffix(i18n(" msec"));
  bounceKeysDelay->setLabel(i18n("D&ebounce time:"), Qt::AlignVCenter|Qt::AlignLeft);;
  hbox->addWidget(bounceKeysDelay);

  hbox = new QHBoxLayout();
  vvbox->addLayout(hbox);
  hbox->addSpacing(24);
  bounceKeysRejectBeep = new QCheckBox(i18n("Use the system bell whenever a key is rejected"), grp);
  hbox->addWidget(bounceKeysRejectBeep);

  connect(slowKeysDelay, SIGNAL(valueChanged(double)), this, SLOT(configChanged()));
  connect(slowKeys, SIGNAL(clicked()), this, SLOT(configChanged()));
  connect(slowKeys, SIGNAL(clicked()), this, SLOT(checkAccess()));

  connect(slowKeysPressBeep, SIGNAL(clicked()), this, SLOT(configChanged()));
  connect(slowKeysAcceptBeep, SIGNAL(clicked()), this, SLOT(configChanged()));
  connect(slowKeysRejectBeep, SIGNAL(clicked()), this, SLOT(configChanged()));

  connect(bounceKeysDelay, SIGNAL(valueChanged(double)), this, SLOT(configChanged()));
  connect(bounceKeys, SIGNAL(clicked()), this, SLOT(configChanged()));
  connect(bounceKeysRejectBeep, SIGNAL(clicked()), this, SLOT(configChanged()));
  connect(bounceKeys, SIGNAL(clicked()), this, SLOT(checkAccess()));

  vbox->addStretch();

  tab->addTab(filters, i18n("&Keyboard Filters"));

  // gestures --------------------------------------------
  QWidget *features = new QWidget(this);

  vbox = new QVBoxLayout(features);

  grp = new QGroupBox(i18n("Activation Gestures"), features);
  layout = new QHBoxLayout;
  grp->setLayout(layout);
  vbox->addWidget(grp);

  vvbox = new QVBoxLayout();
  layout->addLayout( vvbox );

  gestures = new QCheckBox(i18n("Use gestures for activating sticky keys and slow keys"), grp);
  vvbox->addWidget(gestures);
  QString shortcut = mouseKeysShortcut(QX11Info::display());
  if (shortcut.isEmpty())
	  gestures->setWhatsThis( i18n("Here you can activate keyboard gestures that turn on the following features: \n"
			  "Sticky keys: Press Shift key 5 consecutive times\n"
					  "Slow keys: Hold down Shift for 8 seconds"));
  else
	  gestures->setWhatsThis( i18n("Here you can activate keyboard gestures that turn on the following features: \n"
			  "Mouse Keys: %1\n"
					  "Sticky keys: Press Shift key 5 consecutive times\n"
					  "Slow keys: Hold down Shift for 8 seconds", shortcut));

  timeout = new QCheckBox(i18n("Turn sticky keys and slow keys off after a certain period of inactivity."), grp);
  vvbox->addWidget(timeout);

  hbox = new QHBoxLayout();
  vvbox->addLayout(hbox);
  hbox->addSpacing(24);
  timeoutDelay = new KIntNumInput(grp);
  timeoutDelay->setSuffix(i18n(" min"));
  timeoutDelay->setRange(1, 30, 4);
  timeoutDelay->setLabel(i18n("Timeout:"), Qt::AlignVCenter|Qt::AlignLeft);;
  hbox->addWidget(timeoutDelay);

  grp = new QGroupBox(i18n("Notification"), features);
  layout = new QHBoxLayout;
  grp->setLayout(layout);
  vbox->addWidget(grp);

  vvbox = new QVBoxLayout();
  layout->addLayout(vvbox);

  accessxBeep = new QCheckBox(i18n("Use the system bell whenever a gesture is used to turn an accessibility feature on or off"), grp);
  vvbox->addWidget(accessxBeep);

  gestureConfirmation = new QCheckBox(i18n("Show a confirmation dialog whenever a keyboard accessibility feature is turned on or off"), grp);
  vvbox->addWidget(gestureConfirmation);
  gestureConfirmation->setWhatsThis( i18n("If this option is checked, KDE will show a confirmation dialog whenever a keyboard accessibility feature is turned on or off.\nEnsure you know what you are doing if you uncheck it, as the keyboard accessibility settings will then always be applied without confirmation.") );

  kNotifyAccessX = new QCheckBox(i18n("Use KDE's system notification mechanism whenever a keyboard accessibility feature is turned on or off"), grp);
  vvbox->addWidget(kNotifyAccessX);

  hbox = new QHBoxLayout();
  vvbox->addLayout(hbox);
  hbox->addStretch(1);
  kNotifyAccessXButton = new QPushButton(i18n("Configure &Notifications..."), grp);
  kNotifyAccessXButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  hbox->addWidget(kNotifyAccessXButton);

  connect(gestures, SIGNAL(clicked()), this, SLOT(configChanged()));
  connect(timeout, SIGNAL(clicked()), this, SLOT(configChanged()));
  connect(timeout, SIGNAL(clicked()), this, SLOT(checkAccess()));
  connect(timeoutDelay, SIGNAL(valueChanged(int)), this, SLOT(configChanged()));
  connect(accessxBeep, SIGNAL(clicked()), this, SLOT(configChanged()));
  connect(gestureConfirmation, SIGNAL(clicked()), this, SLOT(configChanged()));
  connect(kNotifyAccessX, SIGNAL(clicked()), this, SLOT(configChanged()));
  connect(kNotifyAccessX, SIGNAL(clicked()), this, SLOT(checkAccess()));
  connect(kNotifyAccessXButton, SIGNAL(clicked()), this, SLOT(configureKNotify()));

  vbox->addStretch();

  tab->addTab(features, i18n("Activation Gestures"));
}


KAccessConfig::~KAccessConfig()
{
}

void KAccessConfig::configureKNotify()
{
	KNotifyConfigWidget::configure (this, "kaccess");
}

void KAccessConfig::changeFlashScreenColor()
{
	invertScreen->setChecked(false);
	flashScreen->setChecked(true);
	configChanged();
}

void KAccessConfig::selectSound()
{
  QStringList list = KGlobal::dirs()->findDirs("sound", "");
  QString start;
  if (list.count()>0)
    start = list[0];
  QString fname = QFileDialog::getOpenFileName(this, QString(), start);
  if (!fname.isEmpty())
    soundEdit->setText(fname);
}


void KAccessConfig::configChanged()
{
  emit changed(true);
}


void KAccessConfig::load()
{
  KConfigGroup cg(KSharedConfig::openConfig("kaccessrc"), "Bell");

  systemBell->setChecked(cg.readEntry("SystemBell", true));
  customBell->setChecked(cg.readEntry("ArtsBell", false));
  soundEdit->setText(cg.readPathEntry("ArtsBellFile", QString()));

  visibleBell->setChecked(cg.readEntry("VisibleBell", false));
  invertScreen->setChecked(cg.readEntry("VisibleBellInvert", true));
  flashScreen->setChecked(!invertScreen->isChecked());
  colorButton->setColor(cg.readEntry("VisibleBellColor", QColor(Qt::red)));

  durationSlider->setValue(cg.readEntry("VisibleBellPause", 500));

  KConfigGroup keyboardGroup(KSharedConfig::openConfig("kaccessrc"),"Keyboard");

  stickyKeys->setChecked(keyboardGroup.readEntry("StickyKeys", false));
  stickyKeysLock->setChecked(keyboardGroup.readEntry("StickyKeysLatch", true));
  stickyKeysAutoOff->setChecked(keyboardGroup.readEntry("StickyKeysAutoOff", false));
  stickyKeysBeep->setChecked(keyboardGroup.readEntry("StickyKeysBeep", true));
  toggleKeysBeep->setChecked(keyboardGroup.readEntry("ToggleKeysBeep", false));
  kNotifyModifiers->setChecked(keyboardGroup.readEntry("kNotifyModifiers", false));

  slowKeys->setChecked(keyboardGroup.readEntry("SlowKeys", false));
  slowKeysDelay->setValue(keyboardGroup.readEntry("SlowKeysDelay", 500));
  slowKeysPressBeep->setChecked(keyboardGroup.readEntry("SlowKeysPressBeep", true));
  slowKeysAcceptBeep->setChecked(keyboardGroup.readEntry("SlowKeysAcceptBeep", true));
  slowKeysRejectBeep->setChecked(keyboardGroup.readEntry("SlowKeysRejectBeep", true));

  bounceKeys->setChecked(keyboardGroup.readEntry("BounceKeys", false));
  bounceKeysDelay->setValue(keyboardGroup.readEntry("BounceKeysDelay", 500));
  bounceKeysRejectBeep->setChecked(keyboardGroup.readEntry("BounceKeysRejectBeep", true));

  gestures->setChecked(keyboardGroup.readEntry("Gestures", false));
  timeout->setChecked(keyboardGroup.readEntry("AccessXTimeout", false));
  timeoutDelay->setValue(keyboardGroup.readEntry("AccessXTimeoutDelay", 30));

  accessxBeep->setChecked(keyboardGroup.readEntry("AccessXBeep", true));
  gestureConfirmation->setChecked(keyboardGroup.readEntry("GestureConfirmation", false));
  kNotifyAccessX->setChecked(keyboardGroup.readEntry("kNotifyAccessX", false));

  checkAccess();

  emit changed(false);
}


void KAccessConfig::save()
{
  KConfigGroup cg(KSharedConfig::openConfig("kaccessrc"), "Bell");

  cg.writeEntry("SystemBell", systemBell->isChecked());
  cg.writeEntry("ArtsBell", customBell->isChecked());
  cg.writePathEntry("ArtsBellFile", soundEdit->text());

  cg.writeEntry("VisibleBell", visibleBell->isChecked());
  cg.writeEntry("VisibleBellInvert", invertScreen->isChecked());
  cg.writeEntry("VisibleBellColor", colorButton->color());

  cg.writeEntry("VisibleBellPause", durationSlider->value());

  KConfigGroup keyboardGroup(KSharedConfig::openConfig("kaccessrc"),"Keyboard");

  keyboardGroup.writeEntry("StickyKeys", stickyKeys->isChecked());
  keyboardGroup.writeEntry("StickyKeysLatch", stickyKeysLock->isChecked());
  keyboardGroup.writeEntry("StickyKeysAutoOff", stickyKeysAutoOff->isChecked());
  keyboardGroup.writeEntry("StickyKeysBeep", stickyKeysBeep->isChecked());
  keyboardGroup.writeEntry("ToggleKeysBeep", toggleKeysBeep->isChecked());
  keyboardGroup.writeEntry("kNotifyModifiers", kNotifyModifiers->isChecked());

  keyboardGroup.writeEntry("SlowKeys", slowKeys->isChecked());
  keyboardGroup.writeEntry("SlowKeysDelay", slowKeysDelay->value());
  keyboardGroup.writeEntry("SlowKeysPressBeep", slowKeysPressBeep->isChecked());
  keyboardGroup.writeEntry("SlowKeysAcceptBeep", slowKeysAcceptBeep->isChecked());
  keyboardGroup.writeEntry("SlowKeysRejectBeep", slowKeysRejectBeep->isChecked());


  keyboardGroup.writeEntry("BounceKeys", bounceKeys->isChecked());
  keyboardGroup.writeEntry("BounceKeysDelay", bounceKeysDelay->value());
  keyboardGroup.writeEntry("BounceKeysRejectBeep", bounceKeysRejectBeep->isChecked());

  keyboardGroup.writeEntry("Gestures", gestures->isChecked());
  keyboardGroup.writeEntry("AccessXTimeout", timeout->isChecked());
  keyboardGroup.writeEntry("AccessXTimeoutDelay", timeoutDelay->value());

  keyboardGroup.writeEntry("AccessXBeep", accessxBeep->isChecked());
  keyboardGroup.writeEntry("GestureConfirmation", gestureConfirmation->isChecked());
  keyboardGroup.writeEntry("kNotifyAccessX", kNotifyAccessX->isChecked());


  cg.sync();
  keyboardGroup.sync();

  if (systemBell->isChecked() ||
      customBell->isChecked() ||
      visibleBell->isChecked())
  {
    KConfig _cfg("kdeglobals", KConfig::NoGlobals);
    KConfigGroup cfg(&_cfg, "General");
    cfg.writeEntry("UseSystemBell", true);
    cfg.sync();
  }

  // make kaccess reread the configuration
  // turning a11y features off needs to be done by kaccess
  // so run it to clear any enabled features and it will exit if it should
  KToolInvocation::startServiceByDesktopName("kaccess");

  emit changed(false);
}


void KAccessConfig::defaults()
{
  systemBell->setChecked(true);
  customBell->setChecked(false);
  soundEdit->setText("");

  visibleBell->setChecked(false);
  invertScreen->setChecked(true);
  flashScreen->setChecked(false);
  colorButton->setColor(QColor(Qt::red));

  durationSlider->setValue(500);

  slowKeys->setChecked(false);
  slowKeysDelay->setValue(500);
  slowKeysPressBeep->setChecked(true);
  slowKeysAcceptBeep->setChecked(true);
  slowKeysRejectBeep->setChecked(true);

  bounceKeys->setChecked(false);
  bounceKeysDelay->setValue(500);
  bounceKeysRejectBeep->setChecked(true);

  stickyKeys->setChecked(false);
  stickyKeysLock->setChecked(true);
  stickyKeysAutoOff->setChecked(false);
  stickyKeysBeep->setChecked(true);
  toggleKeysBeep->setChecked(false);
  kNotifyModifiers->setChecked(false);

  gestures->setChecked(false);
  timeout->setChecked(false);
  timeoutDelay->setValue(30);

  accessxBeep->setChecked(true);
  gestureConfirmation->setChecked(true);
  kNotifyAccessX->setChecked(false);

  checkAccess();

  emit changed(true);
}


void KAccessConfig::invertClicked()
{
  flashScreen->setChecked(false);
}


void KAccessConfig::flashClicked()
{
  invertScreen->setChecked(false);
}


void KAccessConfig::checkAccess()
{
  bool custom = customBell->isChecked();
  soundEdit->setEnabled(custom);
  soundButton->setEnabled(custom);
  soundLabel->setEnabled(custom);

  bool visible = visibleBell->isChecked();
  invertScreen->setEnabled(visible);
  flashScreen->setEnabled(visible);
  colorButton->setEnabled(visible);
  durationSlider->setEnabled(visible);

  bool sticky = stickyKeys->isChecked();
  stickyKeysLock->setEnabled(sticky);
  stickyKeysAutoOff->setEnabled(sticky);
  stickyKeysBeep->setEnabled(sticky);

  bool slow = slowKeys->isChecked();
  slowKeysDelay->setEnabled(slow);
  slowKeysPressBeep->setEnabled(slow);
  slowKeysAcceptBeep->setEnabled(slow);
  slowKeysRejectBeep->setEnabled(slow);

  bool bounce = bounceKeys->isChecked();
  bounceKeysDelay->setEnabled(bounce);
  bounceKeysRejectBeep->setEnabled(bounce);

  bool useTimeout = timeout->isChecked();
  timeoutDelay->setEnabled(useTimeout);
}

extern "C"
{
  /* This one gets called by kcminit

   */
  Q_DECL_EXPORT void kcminit_access()
  {
    KConfig config("kaccessrc", KConfig::NoGlobals);
    KToolInvocation::startServiceByDesktopName("kaccess");
  }
}

#include "kcmaccess.moc"

