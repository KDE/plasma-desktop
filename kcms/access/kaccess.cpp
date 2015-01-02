/*
    Copyright 2000 Matthias Hölzer-Klüpfel <hoelzer@kde.org>
    Copyright 2014 Frederik Gladhorn <gladhorn@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) version 3, or any
    later version accepted by the membership of KDE e.V. (or its
    successor approved by the membership of KDE e.V.), which shall
    act as a proxy defined in Section 6 of version 3 of the license.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <unistd.h>

#include "kaccess.h"

#include <QTimer>
#include <QPainter>

#include <QLabel>
#include <QDesktopWidget>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include <KDialog>
#include <KMessageBox>
#include <KComboBox>
#include <KIconLoader>
#include <KDebug>
#include <KNotification>
#include <KConfig>
#include <KGlobal>
#include <KLocalizedString>
#include <netwm.h>
#include <KShortcut>
#include <kwindowsystem.h>
#include <kkeyserver.h>

#include <X11/XKBlib.h>
#define XK_MISCELLANY
#define XK_XKB_KEYS
#include <X11/keysymdef.h>

#include <QX11Info>
#include <kvbox.h>

struct ModifierKey {
   const unsigned int mask;
   const KeySym keysym;
   const char *name;
   const char *lockedText;
   const char *latchedText;
   const char *unlatchedText;
};

static const ModifierKey modifierKeys[] = {
    { ShiftMask, 0, "Shift",
      I18N_NOOP("The Shift key has been locked and is now active for all of the following keypresses."),
      I18N_NOOP("The Shift key is now active."),
      I18N_NOOP("The Shift key is now inactive.") },
    { ControlMask, 0, "Control",
      I18N_NOOP("The Control key has been locked and is now active for all of the following keypresses."),
      I18N_NOOP("The Control key is now active."),
      I18N_NOOP("The Control key is now inactive.") },
    { 0, XK_Alt_L, "Alt",
      I18N_NOOP("The Alt key has been locked and is now active for all of the following keypresses."),
      I18N_NOOP("The Alt key is now active."),
      I18N_NOOP("The Alt key is now inactive.") },
    { 0, 0, "Win",
      I18N_NOOP("The Win key has been locked and is now active for all of the following keypresses."),
      I18N_NOOP("The Win key is now active."),
      I18N_NOOP("The Win key is now inactive.") },
    { 0, XK_Meta_L, "Meta",
      I18N_NOOP("The Meta key has been locked and is now active for all of the following keypresses."),
      I18N_NOOP("The Meta key is now active."),
      I18N_NOOP("The Meta key is now inactive.") },
    { 0, XK_Super_L, "Super",
      I18N_NOOP("The Super key has been locked and is now active for all of the following keypresses."),
      I18N_NOOP("The Super key is now active."),
      I18N_NOOP("The Super key is now inactive.") },
    { 0, XK_Hyper_L, "Hyper",
      I18N_NOOP("The Hyper key has been locked and is now active for all of the following keypresses."),
      I18N_NOOP("The Hyper key is now active."),
      I18N_NOOP("The Hyper key is now inactive.") },
    { 0, 0, "Alt Graph",
      I18N_NOOP("The Alt Graph key has been locked and is now active for all of the following keypresses."),
      I18N_NOOP("The Alt Graph key is now active."),
      I18N_NOOP("The Alt Graph key is now inactive.") },
    { 0, XK_Num_Lock, "Num Lock",
      I18N_NOOP("The Num Lock key has been activated."),
      "",
      I18N_NOOP("The Num Lock key is now inactive.") },
    { LockMask, 0, "Caps Lock",
      I18N_NOOP("The Caps Lock key has been activated."),
      "",
      I18N_NOOP("The Caps Lock key is now inactive.") },
    { 0, XK_Scroll_Lock, "Scroll Lock",
      I18N_NOOP("The Scroll Lock key has been activated."),
      "",
      I18N_NOOP("The Scroll Lock key is now inactive.") },
    { 0, 0, "", "", "", "" }
};


/********************************************************************/


KAccessApp::KAccessApp(bool allowStyles, bool GUIenabled)
  : KUniqueApplication(allowStyles, GUIenabled),
  overlay(0), _player(0)
{
  _activeWindow = KWindowSystem::activeWindow();
  connect(KWindowSystem::self(), &KWindowSystem::activeWindowChanged, this, &KAccessApp::activeWindowChanged);

  features = 0;
  requestedFeatures = 0;
  dialog = 0;

  initMasks();
  XkbStateRec state_return;
  XkbGetState (QX11Info::display(), XkbUseCoreKbd, &state_return);
  unsigned char latched = XkbStateMods (&state_return);
  unsigned char locked  = XkbModLocks  (&state_return);
  state = ((int)locked)<<8 | latched;
}

int KAccessApp::newInstance()
{
  KSharedConfig::openConfig()->reparseConfiguration();
  readSettings();
  return 0;
}

void KAccessApp::readSettings()
{
  KSharedConfig::Ptr _config = KSharedConfig::openConfig();
  KConfigGroup cg(_config, "Bell");

  // bell ---------------------------------------------------------------
  _systemBell = cg.readEntry("SystemBell", true);
  _artsBell = cg.readEntry("ArtsBell", false);
  _currentPlayerSource = cg.readPathEntry("ArtsBellFile", QString());
  _visibleBell = cg.readEntry("VisibleBell", false);
  _visibleBellInvert = cg.readEntry("VisibleBellInvert", false);
  _visibleBellColor = cg.readEntry("VisibleBellColor", QColor(Qt::red));
  _visibleBellPause = cg.readEntry("VisibleBellPause", 500);

  // select bell events if we need them
  int state = (_artsBell || _visibleBell) ? XkbBellNotifyMask : 0;
  XkbSelectEvents(QX11Info::display(), XkbUseCoreKbd, XkbBellNotifyMask, state);

  // deactivate system bell if not needed
  if (!_systemBell)
    XkbChangeEnabledControls(QX11Info::display(), XkbUseCoreKbd, XkbAudibleBellMask, 0);
  else
    XkbChangeEnabledControls(QX11Info::display(), XkbUseCoreKbd, XkbAudibleBellMask, XkbAudibleBellMask);

  // keyboard -------------------------------------------------------------
  KConfigGroup keyboardGroup(_config,"Keyboard");

  // get keyboard state
  XkbDescPtr xkb = XkbGetMap(QX11Info::display(), 0, XkbUseCoreKbd);
  if (!xkb)
    return;
  if (XkbGetControls(QX11Info::display(), XkbAllControlsMask, xkb) != Success)
    return;

  // sticky keys
  if (keyboardGroup.readEntry("StickyKeys", false))
    {
      if (keyboardGroup.readEntry("StickyKeysLatch", true))
        xkb->ctrls->ax_options |= XkbAX_LatchToLockMask;
      else
        xkb->ctrls->ax_options &= ~XkbAX_LatchToLockMask;
      if (keyboardGroup.readEntry("StickyKeysAutoOff", false))
         xkb->ctrls->ax_options |= XkbAX_TwoKeysMask;
      else
         xkb->ctrls->ax_options &= ~XkbAX_TwoKeysMask;
      if (keyboardGroup.readEntry("StickyKeysBeep", false))
         xkb->ctrls->ax_options |= XkbAX_StickyKeysFBMask;
      else
         xkb->ctrls->ax_options &= ~XkbAX_StickyKeysFBMask;
      xkb->ctrls->enabled_ctrls |= XkbStickyKeysMask;
    }
  else
    xkb->ctrls->enabled_ctrls &= ~XkbStickyKeysMask;

  // toggle keys
  if (keyboardGroup.readEntry("ToggleKeysBeep", false))
     xkb->ctrls->ax_options |= XkbAX_IndicatorFBMask;
  else
     xkb->ctrls->ax_options &= ~XkbAX_IndicatorFBMask;

  // slow keys
  if (keyboardGroup.readEntry("SlowKeys", false)) {
      if (keyboardGroup.readEntry("SlowKeysPressBeep", false))
         xkb->ctrls->ax_options |= XkbAX_SKPressFBMask;
      else
         xkb->ctrls->ax_options &= ~XkbAX_SKPressFBMask;
      if (keyboardGroup.readEntry("SlowKeysAcceptBeep", false))
         xkb->ctrls->ax_options |= XkbAX_SKAcceptFBMask;
      else
         xkb->ctrls->ax_options &= ~XkbAX_SKAcceptFBMask;
      if (keyboardGroup.readEntry("SlowKeysRejectBeep", false))
         xkb->ctrls->ax_options |= XkbAX_SKRejectFBMask;
      else
         xkb->ctrls->ax_options &= ~XkbAX_SKRejectFBMask;
      xkb->ctrls->enabled_ctrls |= XkbSlowKeysMask;
    }
  else
      xkb->ctrls->enabled_ctrls &= ~XkbSlowKeysMask;
  xkb->ctrls->slow_keys_delay = keyboardGroup.readEntry("SlowKeysDelay", 500);

  // bounce keys
  if (keyboardGroup.readEntry("BounceKeys", false)) {
      if (keyboardGroup.readEntry("BounceKeysRejectBeep", false))
         xkb->ctrls->ax_options |= XkbAX_BKRejectFBMask;
      else
         xkb->ctrls->ax_options &= ~XkbAX_BKRejectFBMask;
      xkb->ctrls->enabled_ctrls |= XkbBounceKeysMask;
    }
  else
      xkb->ctrls->enabled_ctrls &= ~XkbBounceKeysMask;
  xkb->ctrls->debounce_delay = keyboardGroup.readEntry("BounceKeysDelay", 500);

  // gestures for enabling the other features
  _gestures = keyboardGroup.readEntry("Gestures", false);
  if (_gestures)
      xkb->ctrls->enabled_ctrls |= XkbAccessXKeysMask;
  else
      xkb->ctrls->enabled_ctrls &= ~XkbAccessXKeysMask;

  // timeout
  if (keyboardGroup.readEntry("AccessXTimeout", false))
    {
      xkb->ctrls->ax_timeout = keyboardGroup.readEntry("AccessXTimeoutDelay", 30)*60;
      xkb->ctrls->axt_opts_mask = 0;
      xkb->ctrls->axt_opts_values = 0;
      xkb->ctrls->axt_ctrls_mask = XkbStickyKeysMask | XkbSlowKeysMask;
      xkb->ctrls->axt_ctrls_values = 0;
      xkb->ctrls->enabled_ctrls |= XkbAccessXTimeoutMask;
    }
  else
    xkb->ctrls->enabled_ctrls &= ~XkbAccessXTimeoutMask;

  // gestures for enabling the other features
  if (keyboardGroup.readEntry("AccessXBeep", true))
     xkb->ctrls->ax_options |= XkbAX_FeatureFBMask | XkbAX_SlowWarnFBMask;
  else
     xkb->ctrls->ax_options &= ~(XkbAX_FeatureFBMask | XkbAX_SlowWarnFBMask);

  _gestureConfirmation = keyboardGroup.readEntry("GestureConfirmation", false);

  _kNotifyModifiers = keyboardGroup.readEntry("kNotifyModifiers", false);
  _kNotifyAccessX = keyboardGroup.readEntry("kNotifyAccessX", false);

  // mouse-by-keyboard ----------------------------------------------

  KConfigGroup mouseGroup(_config,"Mouse");

  if (mouseGroup.readEntry("MouseKeys", false))
    {
      xkb->ctrls->mk_delay = mouseGroup.readEntry("MKDelay", 160);

      // Default for initial velocity: 200 pixels/sec
      int interval = mouseGroup.readEntry("MKInterval", 5);
      xkb->ctrls->mk_interval = interval;

      // Default time to reach maximum speed: 5000 msec
      xkb->ctrls->mk_time_to_max = mouseGroup.readEntry("MKTimeToMax",
                                             (5000+interval/2)/interval);

      // Default maximum speed: 1000 pixels/sec
      //     (The old default maximum speed from KDE <= 3.4
      //     (100000 pixels/sec) was way too fast)
      xkb->ctrls->mk_max_speed = mouseGroup.readEntry("MKMaxSpeed", interval);

      xkb->ctrls->mk_curve = mouseGroup.readEntry("MKCurve", 0);
      xkb->ctrls->mk_dflt_btn = mouseGroup.readEntry("MKDefaultButton", 0);

      xkb->ctrls->enabled_ctrls |= XkbMouseKeysMask;
    }
  else
    xkb->ctrls->enabled_ctrls &= ~XkbMouseKeysMask;

   features = xkb->ctrls->enabled_ctrls & (XkbSlowKeysMask | XkbBounceKeysMask | XkbStickyKeysMask | XkbMouseKeysMask);
   if (dialog == 0)
      requestedFeatures = features;
  // set state
  XkbSetControls(QX11Info::display(), XkbControlsEnabledMask | XkbMouseKeysAccelMask | XkbStickyKeysMask | XkbSlowKeysMask | XkbBounceKeysMask | XkbAccessXKeysMask | XkbAccessXTimeoutMask, xkb);

  // select AccessX events
  XkbSelectEvents(QX11Info::display(), XkbUseCoreKbd, XkbAllEventsMask, XkbAllEventsMask);

  // do we need to stay running to perform notifications?
  if (!_artsBell && !_visibleBell && !(_gestures && _gestureConfirmation)
      && !_kNotifyModifiers && !_kNotifyAccessX) {

     // We will exit, but the features need to stay configured
     uint ctrls = XkbStickyKeysMask | XkbSlowKeysMask | XkbBounceKeysMask | XkbMouseKeysMask | XkbAudibleBellMask | XkbControlsNotifyMask;
     uint values = xkb->ctrls->enabled_ctrls & ctrls;
     XkbSetAutoResetControls(QX11Info::display(), ctrls, &ctrls, &values);
     exit(0);
  } else {
     // reset them after program exit
     uint ctrls = XkbStickyKeysMask | XkbSlowKeysMask | XkbBounceKeysMask | XkbMouseKeysMask | XkbAudibleBellMask | XkbControlsNotifyMask;
     uint values = XkbAudibleBellMask;
     XkbSetAutoResetControls(QX11Info::display(), ctrls, &ctrls, &values);
  }

  delete overlay;
  overlay = 0;
}

static int maskToBit (int mask) {
   for (int i = 0; i < 8; i++)
      if (mask & (1 << i))
         return i;
   return -1;
}

void KAccessApp::initMasks() {
   for (int i = 0; i < 8; i++)
      keys [i] = -1;
   state = 0;

   for (int i = 0; strcmp (modifierKeys[i].name, "") != 0; i++) {
      int mask = modifierKeys[i].mask;
      if (mask == 0) {
          if (modifierKeys[i].keysym != 0) {
              mask = XkbKeysymToModifiers (QX11Info::display(), modifierKeys[i].keysym); 
          } else {
              if (!strcmp(modifierKeys[i].name, "Win")) {
                  mask = KKeyServer::modXMeta(); 
              } else {
                  mask = XkbKeysymToModifiers (QX11Info::display(), XK_Mode_switch)
                      | XkbKeysymToModifiers (QX11Info::display(), XK_ISO_Level3_Shift)
                      | XkbKeysymToModifiers (QX11Info::display(), XK_ISO_Level3_Latch)
                      | XkbKeysymToModifiers (QX11Info::display(), XK_ISO_Level3_Lock);
              }
          }
      }
      

      int bit = maskToBit (mask);
      if (bit != -1 && keys[bit] == -1)
         keys[bit] = i;
   }
}

#warning x11EventFilter needs porting to QAbstractNativeEventFilter
#if 0
bool KAccessApp::x11EventFilter(XEvent *event)
{
  // handle XKB events
  if (event->type == xkb_opcode)
    {
      XkbAnyEvent *ev = (XkbAnyEvent*) event;

      switch (ev->xkb_type) {
      case XkbStateNotify:
         xkbStateNotify();
         break;
      case XkbBellNotify:
         xkbBellNotify((XkbBellNotifyEvent*)event);
         break;
      case XkbControlsNotify:
         xkbControlsNotify((XkbControlsNotifyEvent*)event);
         break;
      }
      return true;
    }

  // process other events as usual
  return KApplication::x11EventFilter(event);
}
#endif


void VisualBell::paintEvent(QPaintEvent *event)
{
  QWidget::paintEvent(event);
  QTimer::singleShot(_pause, this, SLOT(hide()));
}


void KAccessApp::activeWindowChanged(WId wid)
{
  _activeWindow = wid;
}


void KAccessApp::xkbStateNotify () {
   XkbStateRec state_return;
   XkbGetState (QX11Info::display(), XkbUseCoreKbd, &state_return);
   unsigned char latched = XkbStateMods (&state_return);
   unsigned char locked  = XkbModLocks  (&state_return);
   int mods = ((int)locked)<<8 | latched;

   if (state != mods) {
      if (_kNotifyModifiers)
      for (int i = 0; i < 8; i++) {
         if (keys[i] != -1) {
            if ( !strcmp(modifierKeys[keys[i]].latchedText, "")
                && ( (((mods >> i) & 0x101) != 0) != (((state >> i) & 0x101) != 0) ))
            {
               if ((mods >> i) & 1) {
                  KNotification::event ("lockkey-locked", i18n(modifierKeys[keys[i]].lockedText));
               }
               else {
                  KNotification::event ("lockkey-unlocked", i18n(modifierKeys[keys[i]].unlatchedText));
               }
            }
            else if (strcmp(modifierKeys[keys[i]].latchedText, "")
                && ( ((mods >> i) & 0x101) != ((state >> i) & 0x101) ))
            {
               if ((mods >> i) & 0x100) {
                  KNotification::event ("modifierkey-locked", i18n(modifierKeys[keys[i]].lockedText));
               }
               else if ((mods >> i) & 1) {
                  KNotification::event ( "modifierkey-latched", i18n(modifierKeys[keys[i]].latchedText));
               }
               else {
                  KNotification::event ("modifierkey-unlatched", i18n(modifierKeys[keys[i]].unlatchedText));
               }
            }
         }
      }
      state = mods;
   }
}

void KAccessApp::xkbBellNotify(XkbBellNotifyEvent *event)
{
  // bail out if we should not really ring
  if (event->event_only)
    return;

  // flash the visible bell
  if (_visibleBell)
    {
      // create overlay widget
      if (!overlay)
        overlay = new VisualBell(_visibleBellPause);

      WId id = _activeWindow;

      NETRect frame, window;
      NETWinInfo net(QX11Info::connection(), id, desktop()->winId(), 0);

      net.kdeGeometry(frame, window);

      overlay->setGeometry(window.pos.x, window.pos.y, window.size.width, window.size.height);

      if (_visibleBellInvert)
        {
	  QPixmap screen = QPixmap::grabWindow(id, 0, 0, window.size.width, window.size.height);
#ifdef __GNUC__
#warning is this the best way to invert a pixmap?
#endif
//	  QPixmap invert(window.size.width, window.size.height);
	  QImage i = screen.toImage();
	  i.invertPixels();
	  QPalette pal = overlay->palette();
	  pal.setBrush(overlay->backgroundRole(), QBrush(QPixmap::fromImage(i)));
	  overlay->setPalette(pal);
/*
	  QPainter p(&invert);
	  p.setRasterOp(QPainter::NotCopyROP);
	  p.drawPixmap(0, 0, screen);
	  overlay->setBackgroundPixmap(invert);
*/
	}
      else
        {
	  QPalette pal = overlay->palette();
	  pal.setColor(overlay->backgroundRole(), _visibleBellColor);
	  overlay->setPalette(pal);
        }

      // flash the overlay widget
      overlay->raise();
      overlay->show();
      flush();
    }

  // ask Phonon to ring a nice bell
  if (_artsBell) {
    if (!_player) { // as creating the player is expensive, delay the creation
      _player = Phonon::createPlayer(Phonon::AccessibilityCategory);
      _player->setParent(this);
      _player->setCurrentSource(_currentPlayerSource);
    }
    _player->play();
  }
}

QString mouseKeysShortcut (Display *display) {
  // Calculate the keycode
  KeySym sym = XK_MouseKeys_Enable;
  KeyCode code = XKeysymToKeycode(display, sym);
  if (code == 0) {
     sym = XK_Pointer_EnableKeys;
     code = XKeysymToKeycode(display, sym);
     if (code == 0)
        return QString(); // No shortcut available?
  }

  // Calculate the modifiers by searching the keysym in the X keyboard mapping
  XkbDescPtr xkbdesc = XkbGetMap(display, XkbKeyTypesMask | XkbKeySymsMask, XkbUseCoreKbd);

  if (!xkbdesc)
      return QString(); // Failed to obtain the mapping from server

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
     return QString(); // Somehow the keycode -> keysym mapping is flawed

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
    keyname = i18n("Meta") + '+' + keyname;
  if ((modifiers & WinMask) != 0)
    keyname = QKeySequence(Qt::META).toString() + '+' + keyname;
  if ((modifiers & AltMask) != 0)
    keyname = QKeySequence(Qt::ALT).toString() + '+' + keyname;
  if ((modifiers & ControlMask) != 0)
    keyname = QKeySequence(Qt::CTRL).toString() + '+' + keyname;
  if ((modifiers & ShiftMask) != 0)
    keyname = QKeySequence(Qt::SHIFT).toString() + '+' + keyname;

  return keyname;
}

void KAccessApp::createDialogContents() {
   if (dialog == 0) {
      dialog = new KDialog( 0 );
      dialog->setCaption( i18n("Warning") );
      dialog->setButtons( KDialog::Yes | KDialog::No );
      dialog->setButtonGuiItem( KDialog::Yes, KStandardGuiItem::yes() );
      dialog->setButtonGuiItem( KDialog::No, KStandardGuiItem::no() );
      dialog->setDefaultButton( KDialog::No );
      dialog->setEscapeButton( KDialog::Close );
      dialog->setObjectName( "AccessXWarning" );
      dialog->setModal( true );

      KVBox *topcontents = new KVBox (dialog);
      topcontents->setSpacing(KDialog::spacingHint()*2);
#ifdef __GNUC__
#warning "kde4 fixme"
#endif
	  //topcontents->setMargin(KDialog::marginHint());

      QWidget *contents = new QWidget(topcontents);
      QHBoxLayout * lay = new QHBoxLayout(contents);
      lay->setSpacing(KDialog::spacingHint());

      QLabel *label1 = new QLabel( contents);
      QPixmap pixmap = KIconLoader::global()->loadIcon("dialog-warning", KIconLoader::NoGroup, KIconLoader::SizeMedium, KIconLoader::DefaultState, QStringList(), 0, true);
      if (pixmap.isNull())
         pixmap = QMessageBox::standardIcon(QMessageBox::Warning);
      label1->setPixmap(pixmap);

      lay->addWidget( label1, 0, Qt::AlignCenter );
      lay->addSpacing(KDialog::spacingHint());

      QVBoxLayout * vlay = new QVBoxLayout();
      lay->addItem( vlay );

      featuresLabel = new QLabel( QString(), contents );
      featuresLabel->setAlignment( Qt::AlignVCenter );
      featuresLabel->setWordWrap( true );
      vlay->addWidget( featuresLabel );
      vlay->addStretch();

      QHBoxLayout * hlay = new QHBoxLayout();
      vlay->addItem(hlay);

      QLabel *showModeLabel = new QLabel( i18n("&When a gesture was used:"), contents );
      hlay->addWidget( showModeLabel );

      showModeCombobox = new KComboBox (contents);
      hlay->addWidget( showModeCombobox );
      showModeLabel->setBuddy(showModeCombobox);
      showModeCombobox->insertItem ( 0, i18n("Change Settings Without Asking"));
      showModeCombobox->insertItem ( 1, i18n("Show This Confirmation Dialog"));
      showModeCombobox->insertItem ( 2, i18n("Deactivate All AccessX Features & Gestures"));
      showModeCombobox->setCurrentIndex (1);

      dialog->setMainWidget(topcontents);

      connect(dialog, &KDialog::yesClicked, this, &KAccessApp::yesClicked);
      connect(dialog, &KDialog::noClicked, this, &KAccessApp::noClicked);
      connect(dialog, &KDialog::closeClicked, this, &KAccessApp::dialogClosed);
   }
}

void KAccessApp::xkbControlsNotify(XkbControlsNotifyEvent *event)
{
   unsigned int newFeatures = event->enabled_ctrls & (XkbSlowKeysMask | XkbBounceKeysMask | XkbStickyKeysMask | XkbMouseKeysMask);

   if (newFeatures != features) {
     unsigned int enabled  = newFeatures & ~features;
     unsigned int disabled = features & ~newFeatures;

     if (!_gestureConfirmation) {
        requestedFeatures = enabled | (requestedFeatures & ~disabled);
        notifyChanges();
        features = newFeatures;
     }
     else {
        // set the AccessX features back to what they were. We will
        // apply the changes later if the user allows us to do that.
        readSettings();

        requestedFeatures = enabled | (requestedFeatures & ~disabled);

        enabled  = requestedFeatures & ~features;
        disabled = features & ~requestedFeatures;

        QStringList enabledFeatures;
        QStringList disabledFeatures;

        if (enabled & XkbStickyKeysMask)
           enabledFeatures << i18n("Sticky keys");
        else if (disabled & XkbStickyKeysMask)
           disabledFeatures << i18n("Sticky keys");

        if (enabled & XkbSlowKeysMask)
           enabledFeatures << i18n("Slow keys");
        else if (disabled & XkbSlowKeysMask)
           disabledFeatures << i18n("Slow keys");

        if (enabled & XkbBounceKeysMask)
           enabledFeatures << i18n("Bounce keys");
        else if (disabled & XkbBounceKeysMask)
           disabledFeatures << i18n("Bounce keys");

        if (enabled & XkbMouseKeysMask)
           enabledFeatures << i18n("Mouse keys");
        else if (disabled & XkbMouseKeysMask)
           disabledFeatures << i18n("Mouse keys");

        QString question;
        switch (enabledFeatures.count()) {
           case 0: switch (disabledFeatures.count()) {
              case 1: question = i18n("Do you really want to deactivate \"%1\"?",
                     disabledFeatures[0]);
              break;
              case 2: question = i18n("Do you really want to deactivate \"%1\" and \"%2\"?",
                     disabledFeatures[0], disabledFeatures[1]);
              break;
              case  3: question = i18n("Do you really want to deactivate \"%1\", \"%2\" and \"%3\"?",
                     disabledFeatures[0], disabledFeatures[1],
                     disabledFeatures[2]);
              break;
              case 4: question = i18n("Do you really want to deactivate \"%1\", \"%2\", \"%3\" and \"%4\"?",
                     disabledFeatures[0], disabledFeatures[1],
                     disabledFeatures[2], disabledFeatures[3]);
              break;
           }
           break;
           case 1: switch (disabledFeatures.count()) {
              case 0: question = i18n("Do you really want to activate \"%1\"?",
                     enabledFeatures[0]);
              break;
              case 1: question = i18n("Do you really want to activate \"%1\" and to deactivate \"%2\"?",
                     enabledFeatures[0], disabledFeatures[0]);
              break;
              case 2: question = i18n("Do you really want to activate \"%1\" and to deactivate \"%2\" and \"%3\"?",
                     enabledFeatures[0], disabledFeatures[0],
                     disabledFeatures[1]);
              break;
              case 3: question = i18n("Do you really want to activate \"%1\" and to deactivate \"%2\", \"%3\" and \"%4\"?",
                     enabledFeatures[0], disabledFeatures[0],
                     disabledFeatures[1], disabledFeatures[2]);
              break;
           }
           break;
           case 2: switch (disabledFeatures.count()) {
              case 0: question = i18n("Do you really want to activate \"%1\" and \"%2\"?",
                     enabledFeatures[0], enabledFeatures[1]);
              break;
              case 1: question = i18n("Do you really want to activate \"%1\" and \"%2\" and to deactivate \"%3\"?",
                     enabledFeatures[0], enabledFeatures[1],
                     disabledFeatures[0]);
              break;
              case 2: question = i18n("Do you really want to activate \"%1\", and \"%2\" and to deactivate \"%3\" and \"%4\"?",
                     enabledFeatures[0], enabledFeatures[1],
                     enabledFeatures[0], disabledFeatures[1]);
              break;
           }
           break;
           case 3: switch (disabledFeatures.count()) {
              case 0: question = i18n("Do you really want to activate \"%1\", \"%2\" and \"%3\"?",
                     enabledFeatures[0], enabledFeatures[1],
                     enabledFeatures[2]);
              break;
              case 1: question = i18n("Do you really want to activate \"%1\", \"%2\" and \"%3\" and to deactivate \"%4\"?",
                     enabledFeatures[0], enabledFeatures[1],
                     enabledFeatures[2], disabledFeatures[0]);
              break;
           }
           break;
           case 4: question = i18n("Do you really want to activate \"%1\", \"%2\", \"%3\" and \"%4\"?",
                  enabledFeatures[0], enabledFeatures[1],
                  enabledFeatures[2], enabledFeatures[3]);
           break;
        }
        QString explanation;
        if (enabledFeatures.count()+disabledFeatures.count() == 1) {
           explanation = i18n("An application has requested to change this setting.");

           if (_gestures) {
              if ((enabled | disabled) == XkbSlowKeysMask)
                 explanation = i18n("You held down the Shift key for 8 seconds or an application has requested to change this setting.");
              else if ((enabled | disabled) == XkbStickyKeysMask)
                 explanation = i18n("You pressed the Shift key 5 consecutive times or an application has requested to change this setting.");
              else if ((enabled | disabled) == XkbMouseKeysMask) {
                 QString shortcut = mouseKeysShortcut(QX11Info::display());
                 if (!shortcut.isEmpty() && !shortcut.isNull())
                    explanation = i18n("You pressed %1 or an application has requested to change this setting.", shortcut);
              }
           }
        }
        else {
           if (_gestures)
              explanation = i18n("An application has requested to change these settings, or you used a combination of several keyboard gestures.");
           else
              explanation = i18n("An application has requested to change these settings.");
        }

        createDialogContents();
        featuresLabel->setText ( question+"\n\n"+explanation
              +" "+i18n("These AccessX settings are needed for some users with motion impairments and can be configured in the KDE System Settings. You can also turn them on and off with standardized keyboard gestures.\n\nIf you do not need them, you can select \"Deactivate all AccessX features and gestures\".") );

        KWindowSystem::setState( dialog->winId(), NET::KeepAbove );
        kapp->updateUserTimestamp();
        dialog->show();
     }
  }
}

void KAccessApp::notifyChanges() {
   if (!_kNotifyAccessX)
      return;

   unsigned int enabled  = requestedFeatures & ~features;
   unsigned int disabled = features & ~requestedFeatures;

   if (enabled & XkbSlowKeysMask)
      KNotification::event ("slowkeys", i18n("Slow keys has been enabled. From now on, you need to press each key for a certain length of time before it gets accepted."));
   else if (disabled & XkbSlowKeysMask)
      KNotification::event ("slowkeys", i18n("Slow keys has been disabled."));

   if (enabled & XkbBounceKeysMask)
      KNotification::event ("bouncekeys", i18n("Bounce keys has been enabled. From now on, each key will be blocked for a certain length of time after it was used."));
   else if (disabled & XkbBounceKeysMask)
      KNotification::event ("bouncekeys", i18n("Bounce keys has been disabled."));

   if (enabled & XkbStickyKeysMask)
      KNotification::event ("stickykeys", i18n("Sticky keys has been enabled. From now on, modifier keys will stay latched after you have released them."));
   else if (disabled & XkbStickyKeysMask)
      KNotification::event ("stickykeys", i18n("Sticky keys has been disabled."));

   if (enabled & XkbMouseKeysMask)
      KNotification::event ("mousekeys", i18n("Mouse keys has been enabled. From now on, you can use the number pad of your keyboard in order to control the mouse."));
   else if (disabled & XkbMouseKeysMask)
      KNotification::event ("mousekeys", i18n("Mouse keys has been disabled."));
}

void KAccessApp::applyChanges() {
   notifyChanges();
   unsigned int enabled  = requestedFeatures & ~features;
   unsigned int disabled = features & ~requestedFeatures;

   KConfigGroup config(KSharedConfig::openConfig(), "Keyboard");

   if (enabled & XkbSlowKeysMask)
      config.writeEntry("SlowKeys", true);
   else if (disabled & XkbSlowKeysMask)
      config.writeEntry("SlowKeys", false);

   if (enabled & XkbBounceKeysMask)
      config.writeEntry("BounceKeys", true);
   else if (disabled & XkbBounceKeysMask)
      config.writeEntry("BounceKeys", false);

   if (enabled & XkbStickyKeysMask)
      config.writeEntry("StickyKeys", true);
   else if (disabled & XkbStickyKeysMask)
      config.writeEntry("StickyKeys", false);

   KConfigGroup mousegrp(KSharedConfig::openConfig(),"Mouse");

   if (enabled & XkbMouseKeysMask)
      mousegrp.writeEntry("MouseKeys", true);
   else if (disabled & XkbMouseKeysMask)
      mousegrp.writeEntry("MouseKeys", false);
   mousegrp.sync();
   config.sync();
}

void KAccessApp::yesClicked() {
   if (dialog != 0)
      dialog->deleteLater();
   dialog = 0;

   KConfigGroup config(KSharedConfig::openConfig(), "Keyboard");
   switch (showModeCombobox->currentIndex()) {
      case 0:
         config.writeEntry("Gestures", true);
         config.writeEntry("GestureConfirmation", false);
         break;
      default:
         config.writeEntry("Gestures", true);
         config.writeEntry("GestureConfirmation", true);
         break;
      case 2:
         requestedFeatures = 0;
         config.writeEntry("Gestures", false);
         config.writeEntry("GestureConfirmation", true);
   }
   config.sync();

   if (features != requestedFeatures) {
      notifyChanges();
      applyChanges();
   }
   readSettings();
}

void KAccessApp::noClicked() {
   if (dialog != 0)
      dialog->deleteLater();
   dialog = 0;
   requestedFeatures = features;

   KConfigGroup config(KSharedConfig::openConfig(), "Keyboard");
   switch (showModeCombobox->currentIndex()) {
      case 0:
         config.writeEntry("Gestures", true);
         config.writeEntry("GestureConfirmation", false);
         break;
      default:
         config.writeEntry("Gestures", true);
         config.writeEntry("GestureConfirmation", true);
         break;
      case 2:
         requestedFeatures = 0;
         config.writeEntry("Gestures", false);
         config.writeEntry("GestureConfirmation", true);
   }
   config.sync();

   if (features != requestedFeatures)
      applyChanges();
   readSettings();
}

void KAccessApp::dialogClosed() {
   if (dialog != 0)
      dialog->deleteLater();
   dialog = 0;

   requestedFeatures = features;
}

void KAccessApp::setXkbOpcode(int opcode) {
   xkb_opcode = opcode;
}

