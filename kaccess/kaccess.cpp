/*
    SPDX-FileCopyrightText: 2000 Matthias Hölzer-Klüpfel <hoelzer@kde.org>
    SPDX-FileCopyrightText: 2014 Frederik Gladhorn <gladhorn@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include <cmath>
#include <unistd.h>

#include "kaccess.h"

#include <QAction>
#include <QApplication>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFile>
#include <QHBoxLayout>
#include <QLoggingCategory>
#include <QPainter>
#include <QProcess>
#include <QScreen>
#include <QStyle>
#include <QTimer>
#include <QVBoxLayout>
#include <QtGui/private/qtx11extras_p.h>

#include <KAboutData>
#include <KComboBox>
#include <KConfig>
#include <KConfigGroup>
#include <KDBusService>
#include <KGlobalAccel>
#include <KKeyServer>
#include <KLazyLocalizedString>
#include <KLocalizedString>
#include <KNotification>
#include <KSharedConfig>
#include <KUserTimestamp>
#include <KWindowSystem>
#include <KX11Extras>

#include <netwm.h>
#define XK_MISCELLANY
#define XK_XKB_KEYS
#include <X11/keysymdef.h>

#include <canberra.h>

Q_LOGGING_CATEGORY(logKAccess, "kcm_kaccess")

struct ModifierKey {
    const unsigned int mask;
    const KeySym keysym;
    const char *name;
    const KLazyLocalizedString lockedText;
    const KLazyLocalizedString latchedText;
    const KLazyLocalizedString unlatchedText;
};

static const ModifierKey modifierKeys[] = {
    {ShiftMask,
     0,
     "Shift",
     kli18n("The Shift key has been locked and is now active for all of the following keypresses."),
     kli18n("The Shift key is now active."),
     kli18n("The Shift key is now inactive.")},
    {ControlMask,
     0,
     "Control",
     kli18n("The Control key has been locked and is now active for all of the following keypresses."),
     kli18n("The Control key is now active."),
     kli18n("The Control key is now inactive.")},
    {0,
     XK_Alt_L,
     "Alt",
     kli18n("The Alt key has been locked and is now active for all of the following keypresses."),
     kli18n("The Alt key is now active."),
     kli18n("The Alt key is now inactive.")},
    {0,
     0,
     "Win",
     kli18n("The Win key has been locked and is now active for all of the following keypresses."),
     kli18n("The Win key is now active."),
     kli18n("The Win key is now inactive.")},
    {0,
     XK_Meta_L,
     "Meta",
     kli18n("The Meta key has been locked and is now active for all of the following keypresses."),
     kli18n("The Meta key is now active."),
     kli18n("The Meta key is now inactive.")},
    {0,
     XK_Super_L,
     "Super",
     kli18n("The Super key has been locked and is now active for all of the following keypresses."),
     kli18n("The Super key is now active."),
     kli18n("The Super key is now inactive.")},
    {0,
     XK_Hyper_L,
     "Hyper",
     kli18n("The Hyper key has been locked and is now active for all of the following keypresses."),
     kli18n("The Hyper key is now active."),
     kli18n("The Hyper key is now inactive.")},
    {0,
     0,
     "Alt Graph",
     kli18n("The Alt Graph key has been locked and is now active for all of the following keypresses."),
     kli18n("The Alt Graph key is now active."),
     kli18n("The Alt Graph key is now inactive.")},
    {0, XK_Num_Lock, "Num Lock", kli18n("The Num Lock key has been activated."), {}, kli18n("The Num Lock key is now inactive.")},
    {LockMask, 0, "Caps Lock", kli18n("The Caps Lock key has been activated."), {}, kli18n("The Caps Lock key is now inactive.")},
    {0, XK_Scroll_Lock, "Scroll Lock", kli18n("The Scroll Lock key has been activated."), {}, kli18n("The Scroll Lock key is now inactive.")},
    {0, 0, "", {}, {}, {}}};

/********************************************************************/

KAccessApp::KAccessApp()
    : m_bellSettings(new BellSettings(this))
    , m_keyboardSettings(new KeyboardSettings(this))
    , m_keyboardFiltersSettings(new KeyboardFiltersSettings(this))
    , m_mouseSettings(new MouseSettings(this))
    , m_screenReaderSettings(new ScreenReaderSettings(this))
    , m_kdeglobals(QStringLiteral("kdeglobals"))
    , overlay(nullptr)
    , toggleScreenReaderAction(new QAction(this))
{
    m_error = false;

    features = 0;
    requestedFeatures = 0;
    dialog = nullptr;

    if (!QX11Info::isPlatformX11()) {
        m_error = true;
        return;
    }
    _activeWindow = KX11Extras::activeWindow();
    connect(KX11Extras::self(), &KX11Extras::activeWindowChanged, this, &KAccessApp::activeWindowChanged);

    initMasks();
    XkbStateRec state_return;
    XkbGetState(QX11Info::display(), XkbUseCoreKbd, &state_return);
    unsigned char latched = XkbStateMods(&state_return);
    unsigned char locked = XkbModLocks(&state_return);
    state = ((int)locked) << 8 | latched;

    auto service = new KDBusService(KDBusService::Unique, this);
    connect(service, &KDBusService::activateRequested, this, &KAccessApp::newInstance);

    QTimer::singleShot(0, this, &KAccessApp::readSettings);
}

KAccessApp::~KAccessApp()
{
    if (m_caContext) {
        ca_context_destroy(m_caContext);
    }
}

void KAccessApp::newInstance()
{
    KSharedConfig::openConfig()->reparseConfiguration();
    m_bellSettings.load();
    m_keyboardSettings.load();
    m_keyboardFiltersSettings.load();
    m_mouseSettings.load();
    m_screenReaderSettings.load();
    readSettings();
}

void KAccessApp::readSettings()
{
    // select bell events
    XkbSelectEvents(QX11Info::display(), XkbUseCoreKbd, XkbBellNotifyMask, XkbBellNotifyMask);

    // deactivate system bell to allow playing our own sound
    XkbChangeEnabledControls(QX11Info::display(), XkbUseCoreKbd, XkbAudibleBellMask, 0);

    // get keyboard state
    XkbDescPtr xkb = XkbGetMap(QX11Info::display(), 0, XkbUseCoreKbd);
    if (!xkb)
        return;
    if (XkbGetControls(QX11Info::display(), XkbAllControlsMask, xkb) != Success)
        return;

    // sticky keys
    if (m_keyboardSettings.stickyKeys()) {
        if (m_keyboardSettings.stickyKeysLatch())
            xkb->ctrls->ax_options |= XkbAX_LatchToLockMask;
        else
            xkb->ctrls->ax_options &= ~XkbAX_LatchToLockMask;
        if (m_keyboardSettings.stickyKeysAutoOff())
            xkb->ctrls->ax_options |= XkbAX_TwoKeysMask;
        else
            xkb->ctrls->ax_options &= ~XkbAX_TwoKeysMask;
        if (m_keyboardSettings.stickyKeysBeep())
            xkb->ctrls->ax_options |= XkbAX_StickyKeysFBMask;
        else
            xkb->ctrls->ax_options &= ~XkbAX_StickyKeysFBMask;
        xkb->ctrls->enabled_ctrls |= XkbStickyKeysMask;
    } else
        xkb->ctrls->enabled_ctrls &= ~XkbStickyKeysMask;

    // toggle keys
    if (m_keyboardSettings.toggleKeysBeep())
        xkb->ctrls->ax_options |= XkbAX_IndicatorFBMask;
    else
        xkb->ctrls->ax_options &= ~XkbAX_IndicatorFBMask;

    // slow keys
    if (m_keyboardFiltersSettings.slowKeys()) {
        if (m_keyboardFiltersSettings.slowKeysPressBeep())
            xkb->ctrls->ax_options |= XkbAX_SKPressFBMask;
        else
            xkb->ctrls->ax_options &= ~XkbAX_SKPressFBMask;
        if (m_keyboardFiltersSettings.slowKeysAcceptBeep())
            xkb->ctrls->ax_options |= XkbAX_SKAcceptFBMask;
        else
            xkb->ctrls->ax_options &= ~XkbAX_SKAcceptFBMask;
        if (m_keyboardFiltersSettings.slowKeysRejectBeep())
            xkb->ctrls->ax_options |= XkbAX_SKRejectFBMask;
        else
            xkb->ctrls->ax_options &= ~XkbAX_SKRejectFBMask;
        xkb->ctrls->enabled_ctrls |= XkbSlowKeysMask;
    } else
        xkb->ctrls->enabled_ctrls &= ~XkbSlowKeysMask;
    xkb->ctrls->slow_keys_delay = m_keyboardFiltersSettings.slowKeysDelay();

    // bounce keys
    if (m_keyboardFiltersSettings.bounceKeys()) {
        if (m_keyboardFiltersSettings.bounceKeysRejectBeep())
            xkb->ctrls->ax_options |= XkbAX_BKRejectFBMask;
        else
            xkb->ctrls->ax_options &= ~XkbAX_BKRejectFBMask;
        xkb->ctrls->enabled_ctrls |= XkbBounceKeysMask;
    } else
        xkb->ctrls->enabled_ctrls &= ~XkbBounceKeysMask;
    xkb->ctrls->debounce_delay = m_keyboardFiltersSettings.bounceKeysDelay();

    // gestures for enabling the other features
    if (m_mouseSettings.gestures())
        xkb->ctrls->enabled_ctrls |= XkbAccessXKeysMask;
    else
        xkb->ctrls->enabled_ctrls &= ~XkbAccessXKeysMask;

    // timeout
    if (m_keyboardSettings.accessXTimeout()) {
        xkb->ctrls->ax_timeout = m_keyboardSettings.accessXTimeoutDelay() * 60;
        xkb->ctrls->axt_opts_mask = 0;
        xkb->ctrls->axt_opts_values = 0;
        xkb->ctrls->axt_ctrls_mask = XkbStickyKeysMask | XkbSlowKeysMask;
        xkb->ctrls->axt_ctrls_values = 0;
        xkb->ctrls->enabled_ctrls |= XkbAccessXTimeoutMask;
    } else
        xkb->ctrls->enabled_ctrls &= ~XkbAccessXTimeoutMask;

    // gestures for enabling the other features
    if (m_keyboardSettings.accessXBeep())
        xkb->ctrls->ax_options |= XkbAX_FeatureFBMask | XkbAX_SlowWarnFBMask;
    else
        xkb->ctrls->ax_options &= ~(XkbAX_FeatureFBMask | XkbAX_SlowWarnFBMask);

    // mouse-by-keyboard

    if (m_mouseSettings.mouseKeys()) {
        xkb->ctrls->mk_delay = m_mouseSettings.accelerationDelay();

        const int interval = m_mouseSettings.repetitionInterval();
        xkb->ctrls->mk_interval = interval;

        xkb->ctrls->mk_time_to_max = m_mouseSettings.accelerationTime();

        xkb->ctrls->mk_max_speed = m_mouseSettings.maxSpeed();

        xkb->ctrls->mk_curve = m_mouseSettings.profileCurve();
        xkb->ctrls->mk_dflt_btn = 0;

        xkb->ctrls->enabled_ctrls |= XkbMouseKeysMask;
    } else
        xkb->ctrls->enabled_ctrls &= ~XkbMouseKeysMask;

    features = xkb->ctrls->enabled_ctrls & (XkbSlowKeysMask | XkbBounceKeysMask | XkbStickyKeysMask | XkbMouseKeysMask);
    if (dialog == nullptr)
        requestedFeatures = features;
    // set state
    XkbSetControls(QX11Info::display(),
                   XkbControlsEnabledMask | XkbMouseKeysAccelMask | XkbStickyKeysMask | XkbSlowKeysMask | XkbBounceKeysMask | XkbAccessXKeysMask
                       | XkbAccessXTimeoutMask,
                   xkb);

    // select AccessX events
    XkbSelectEvents(QX11Info::display(), XkbUseCoreKbd, XkbAllEventsMask, XkbAllEventsMask);

    if (!(m_mouseSettings.gestures() && m_mouseSettings.gestureConfirmation()) && !m_keyboardSettings.keyboardNotifyModifiers()
        && !m_mouseSettings.keyboardNotifyAccess()) {
        uint ctrls = XkbStickyKeysMask | XkbSlowKeysMask | XkbBounceKeysMask | XkbMouseKeysMask | XkbAudibleBellMask | XkbControlsNotifyMask;
        uint values = xkb->ctrls->enabled_ctrls & ctrls;
        XkbSetAutoResetControls(QX11Info::display(), ctrls, &ctrls, &values);
    } else {
        // reset them after program exit
        uint ctrls = XkbStickyKeysMask | XkbSlowKeysMask | XkbBounceKeysMask | XkbMouseKeysMask | XkbAudibleBellMask | XkbControlsNotifyMask;
        uint values = XkbAudibleBellMask;
        XkbSetAutoResetControls(QX11Info::display(), ctrls, &ctrls, &values);
    }

    delete overlay;
    overlay = nullptr;

    setScreenReaderEnabled(m_screenReaderSettings.enabled());

    toggleScreenReaderAction->setText(i18n("Toggle Screen Reader On and Off"));
    toggleScreenReaderAction->setObjectName(QStringLiteral("Toggle Screen Reader On and Off"));
    toggleScreenReaderAction->setProperty("componentDisplayName", i18nc("Name for kaccess shortcuts category", "Accessibility"));
    KGlobalAccel::self()->setGlobalShortcut(toggleScreenReaderAction, Qt::META | Qt::ALT | Qt::Key_S);
    connect(toggleScreenReaderAction, &QAction::triggered, this, &KAccessApp::toggleScreenReader);
}

void KAccessApp::toggleScreenReader()
{
    KSharedConfig::Ptr _config = KSharedConfig::openConfig();
    KConfigGroup screenReaderGroup(_config, QStringLiteral("ScreenReader"));
    bool enabled = !screenReaderGroup.readEntry("Enabled", false);
    screenReaderGroup.writeEntry("Enabled", enabled);
    setScreenReaderEnabled(enabled);
}

void KAccessApp::setScreenReaderEnabled(bool enabled)
{
    if (enabled) {
        QStringList args = {QStringLiteral("set"),
                            QStringLiteral("org.gnome.desktop.a11y.applications"),
                            QStringLiteral("screen-reader-enabled"),
                            QStringLiteral("true")};
        int ret = QProcess::execute(QStringLiteral("gsettings"), args);
        if (ret == 0) {
            qint64 pid = 0;
            QProcess::startDetached(QStringLiteral("orca"), {QStringLiteral("--replace")}, QString(), &pid);
            qCDebug(logKAccess) << "Launching Orca, pid:" << pid;
        }
    } else {
        QProcess::startDetached(
            QStringLiteral("gsettings"),
            {QStringLiteral("set"), QStringLiteral("org.gnome.desktop.a11y.applications"), QStringLiteral("screen-reader-enabled"), QStringLiteral("false")});
    }
}

static int maskToBit(int mask)
{
    for (int i = 0; i < 8; i++)
        if (mask & (1 << i))
            return i;
    return -1;
}

void KAccessApp::initMasks()
{
    for (int i = 0; i < 8; i++)
        keys[i] = -1;
    state = 0;

    for (int i = 0; strcmp(modifierKeys[i].name, "") != 0; i++) {
        int mask = modifierKeys[i].mask;
        if (mask == 0) {
            if (modifierKeys[i].keysym != 0) {
                mask = XkbKeysymToModifiers(QX11Info::display(), modifierKeys[i].keysym);
            } else {
                if (!strcmp(modifierKeys[i].name, "Win")) {
                    mask = KKeyServer::modXMeta();
                } else {
                    mask = XkbKeysymToModifiers(QX11Info::display(), XK_Mode_switch) | XkbKeysymToModifiers(QX11Info::display(), XK_ISO_Level3_Shift)
                        | XkbKeysymToModifiers(QX11Info::display(), XK_ISO_Level3_Latch) | XkbKeysymToModifiers(QX11Info::display(), XK_ISO_Level3_Lock);
                }
            }
        }

        int bit = maskToBit(mask);
        if (bit != -1 && keys[bit] == -1)
            keys[bit] = i;
    }
}

struct xkb_any_ {
    uint8_t response_type;
    uint8_t xkbType;
    uint16_t sequence;
    xcb_timestamp_t time;
    uint8_t deviceID;
};

bool KAccessApp::nativeEventFilter(const QByteArray &eventType, void *message, qintptr *result)
{
    Q_UNUSED(result);

    if (eventType == "xcb_generic_event_t") {
        xcb_generic_event_t *event = static_cast<xcb_generic_event_t *>(message);
        if ((event->response_type & ~0x80) == XkbEventCode + xkb_opcode) {
            xkb_any_ *ev = reinterpret_cast<xkb_any_ *>(event);
            // Workaround for an XCB bug. xkbType comes from an EventType that is defined with bits, like
            // <item name="BellNotify">             <bit>8</bit>
            // while the generated XCB event type enum is defined as a bitmask, like
            //     XCB_XKB_EVENT_TYPE_BELL_NOTIFY = 256
            // This means if xbkType is 8, we need to set the 8th bit to 1, thus raising 2 to power of 8.
            // See also https://bugs.freedesktop.org/show_bug.cgi?id=51295
            const int eventType = pow(2, ev->xkbType);
            switch (eventType) {
            case XCB_XKB_EVENT_TYPE_STATE_NOTIFY:
                xkbStateNotify();
                break;
            case XCB_XKB_EVENT_TYPE_BELL_NOTIFY:
                xkbBellNotify(reinterpret_cast<xcb_xkb_bell_notify_event_t *>(event));
                break;
            case XCB_XKB_EVENT_TYPE_CONTROLS_NOTIFY:
                xkbControlsNotify(reinterpret_cast<xcb_xkb_controls_notify_event_t *>(event));
                break;
            }
            return true;
        }
    }
    return false;
}

void VisualBell::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);
    QTimer::singleShot(_pause, this, &QWidget::hide);
}

void KAccessApp::activeWindowChanged(WId wid)
{
    _activeWindow = wid;
}

void KAccessApp::xkbStateNotify()
{
    XkbStateRec state_return;
    XkbGetState(QX11Info::display(), XkbUseCoreKbd, &state_return);
    unsigned char latched = XkbStateMods(&state_return);
    unsigned char locked = XkbModLocks(&state_return);
    int mods = ((int)locked) << 8 | latched;

    if (state != mods) {
        if (m_keyboardSettings.keyboardNotifyModifiers())
            for (int i = 0; i < 8; i++) {
                if (keys[i] != -1) {
                    if (modifierKeys[keys[i]].latchedText.isEmpty() && ((((mods >> i) & 0x101) != 0) != (((state >> i) & 0x101) != 0))) {
                        if ((mods >> i) & 1) {
                            KNotification::event(QStringLiteral("lockkey-locked"), modifierKeys[keys[i]].lockedText.toString());
                        } else {
                            KNotification::event(QStringLiteral("lockkey-unlocked"), modifierKeys[keys[i]].unlatchedText.toString());
                        }
                    } else if (!modifierKeys[keys[i]].latchedText.isEmpty() && (((mods >> i) & 0x101) != ((state >> i) & 0x101))) {
                        if ((mods >> i) & 0x100) {
                            KNotification::event(QStringLiteral("modifierkey-locked"), modifierKeys[keys[i]].lockedText.toString());
                        } else if ((mods >> i) & 1) {
                            KNotification::event(QStringLiteral("modifierkey-latched"), modifierKeys[keys[i]].latchedText.toString());
                        } else {
                            KNotification::event(QStringLiteral("modifierkey-unlatched"), modifierKeys[keys[i]].unlatchedText.toString());
                        }
                    }
                }
            }
        state = mods;
    }
}

void KAccessApp::xkbBellNotify(xcb_xkb_bell_notify_event_t *event)
{
    // bail out if we should not really ring
    if (event->eventOnly)
        return;

    // flash the visible bell
    if (m_bellSettings.visibleBell()) {
        // create overlay widget
        if (!overlay)
            overlay = new VisualBell(m_bellSettings.visibleBellPause());

        WId id = _activeWindow;

        NETRect frame, window;
        NETWinInfo net(QX11Info::connection(), id, QX11Info::appRootWindow(), NET::Properties(), NET::Properties2());

        net.kdeGeometry(frame, window);

        overlay->setGeometry(window.pos.x, window.pos.y, window.size.width, window.size.height);

        if (m_bellSettings.invertScreen()) {
            QPixmap screen = QGuiApplication::primaryScreen()->grabWindow(id, 0, 0, window.size.width, window.size.height);

            // is this the best way to invert a pixmap?
            
            //    QPixmap invert(window.size.width, window.size.height);
            QPalette pal = overlay->palette();
            {
                QImage i = screen.toImage();
                i.invertPixels();
                pal.setBrush(overlay->backgroundRole(), QBrush(QPixmap::fromImage(std::move(i))));
            }
            overlay->setPalette(pal);
            /*
                  QPainter p(&invert);
                  p.setRasterOp(QPainter::NotCopyROP);
                  p.drawPixmap(0, 0, screen);
                  overlay->setBackgroundPixmap(invert);
            */
        } else {
            QPalette pal = overlay->palette();
            pal.setColor(overlay->backgroundRole(), m_bellSettings.visibleBellColor());
            overlay->setPalette(pal);
        }

        // flash the overlay widget
        overlay->raise();
        overlay->show();
        QCoreApplication::sendPostedEvents();
    }

    // ask canberra to ring a nice bell
    if (m_bellSettings.systemBell()) {
        if (!m_caContext) {
            int ret = ca_context_create(&m_caContext);
            if (ret != CA_SUCCESS) {
                qCWarning(logKAccess) << "Failed to initialize canberra context for audio notification:" << ca_strerror(ret);
                m_caContext = nullptr;
                return;
            }

            ret = ca_context_change_props(m_caContext,
                                          CA_PROP_APPLICATION_NAME,
                                          qApp->applicationDisplayName().toUtf8().constData(),
                                          CA_PROP_APPLICATION_ID,
                                          qApp->desktopFileName().toUtf8().constData(),
                                          nullptr);
            if (ret != CA_SUCCESS) {
                qCWarning(logKAccess) << "Failed to set application properties on canberra context for audio notification:" << ca_strerror(ret);
            }
        } else {
            ca_context_cancel(m_caContext, 0);
        }

        if (m_bellSettings.customBell()) {
            ca_context_play(m_caContext,
                            0,
                            CA_PROP_MEDIA_FILENAME,
                            QFile::encodeName(QUrl(m_bellSettings.customBellFile()).toLocalFile()).constData(),
                            CA_PROP_MEDIA_ROLE,
                            "event",
                            CA_PROP_CANBERRA_CACHE_CONTROL,
                            "permanent",
                            nullptr);
        } else {
            const QString themeName = m_kdeglobals.group(QStringLiteral("Sounds")).readEntry("Theme", QStringLiteral("ocean"));
            ca_context_play(m_caContext,
                            0,
                            CA_PROP_EVENT_ID,
                            "bell",
                            CA_PROP_MEDIA_ROLE,
                            "event",
                            CA_PROP_CANBERRA_CACHE_CONTROL,
                            "permanent",
                            CA_PROP_CANBERRA_XDG_THEME_NAME,
                            themeName.toUtf8().constData(),
                            nullptr);
        }
    }
}

QString mouseKeysShortcut(Display *display)
{
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
    for (int grp = 0; grp < groups && !found; grp++) {
        int levels = XkbKeyGroupWidth(xkbdesc, code, grp);
        for (int level = 0; level < levels && !found; level++) {
            if (sym == XkbKeySymEntry(xkbdesc, code, level, grp)) {
                // keysym found => determine modifiers
                int typeIdx = xkbdesc->map->key_sym_map[code].kt_index[grp];
                XkbKeyTypePtr type = &(xkbdesc->map->types[typeIdx]);
                for (int i = 0; i < type->map_count && !found; i++) {
                    if (type->map[i].active && (type->map[i].level == level)) {
                        modifiers = type->map[i].mods.mask;
                        found = true;
                    }
                }
            }
        }
    }
    XkbFreeClientMap(xkbdesc, 0, true);

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

    unsigned int AltMask = KKeyServer::modXAlt();
    unsigned int WinMask = KKeyServer::modXMeta();
    unsigned int NumMask = KKeyServer::modXNumLock();
    unsigned int ScrollMask = KKeyServer::modXScrollLock();

    unsigned int MetaMask = XkbKeysymToModifiers(display, XK_Meta_L);
    unsigned int SuperMask = XkbKeysymToModifiers(display, XK_Super_L);
    unsigned int HyperMask = XkbKeysymToModifiers(display, XK_Hyper_L);
    unsigned int AltGrMask = XkbKeysymToModifiers(display, XK_Mode_switch) | XkbKeysymToModifiers(display, XK_ISO_Level3_Shift)
        | XkbKeysymToModifiers(display, XK_ISO_Level3_Latch) | XkbKeysymToModifiers(display, XK_ISO_Level3_Lock);

    unsigned int mods = ShiftMask | ControlMask | AltMask | WinMask | LockMask | NumMask | ScrollMask;

    AltGrMask &= ~mods;
    MetaMask &= ~(mods | AltGrMask);
    SuperMask &= ~(mods | AltGrMask | MetaMask);
    HyperMask &= ~(mods | AltGrMask | MetaMask | SuperMask);

    if ((modifiers & AltGrMask) != 0)
        keyname = i18n("AltGraph") + QLatin1Char('+') + keyname;
    if ((modifiers & HyperMask) != 0)
        keyname = i18n("Hyper") + QLatin1Char('+') + keyname;
    if ((modifiers & SuperMask) != 0)
        keyname = i18n("Super") + QLatin1Char('+') + keyname;
    if ((modifiers & WinMask) != 0)
        keyname = i18n("Meta") + QLatin1Char('+') + keyname;
    if ((modifiers & WinMask) != 0)
        keyname = QKeySequence(Qt::META).toString() + QLatin1Char('+') + keyname;
    if ((modifiers & AltMask) != 0)
        keyname = QKeySequence(Qt::ALT).toString() + QLatin1Char('+') + keyname;
    if ((modifiers & ControlMask) != 0)
        keyname = QKeySequence(Qt::CTRL).toString() + QLatin1Char('+') + keyname;
    if ((modifiers & ShiftMask) != 0)
        keyname = QKeySequence(Qt::SHIFT).toString() + QLatin1Char('+') + keyname;

    return keyname;
}

void KAccessApp::createDialogContents()
{
    if (dialog == nullptr) {
        dialog = new QDialog(nullptr);
        dialog->setWindowTitle(i18n("Warning"));
        dialog->setObjectName(QStringLiteral("AccessXWarning"));
        dialog->setModal(true);

        QVBoxLayout *topLayout = new QVBoxLayout();

        QHBoxLayout *lay = new QHBoxLayout();

        QLabel *label1 = new QLabel();
        QIcon icon = QIcon::fromTheme(QStringLiteral("dialog-warning"));
        if (icon.isNull())
            icon = qApp->style()->standardIcon(QStyle::SP_MessageBoxWarning);
        label1->setPixmap(icon.pixmap(64, 64));

        lay->addWidget(label1, 0, Qt::AlignCenter);

        QVBoxLayout *vlay = new QVBoxLayout();
        lay->addItem(vlay);

        featuresLabel = new QLabel();
        featuresLabel->setAlignment(Qt::AlignVCenter);
        featuresLabel->setWordWrap(true);
        vlay->addWidget(featuresLabel);
        vlay->addStretch();

        QHBoxLayout *hlay = new QHBoxLayout();
        vlay->addItem(hlay);

        QLabel *showModeLabel = new QLabel(i18n("&When a gesture was used:"));
        hlay->addWidget(showModeLabel);

        showModeCombobox = new KComboBox();
        hlay->addWidget(showModeCombobox);
        showModeLabel->setBuddy(showModeCombobox);
        showModeCombobox->insertItem(0, i18n("Change Settings Without Asking"));
        showModeCombobox->insertItem(1, i18n("Show This Confirmation Dialog"));
        showModeCombobox->insertItem(2, i18n("Deactivate All AccessX Features & Gestures"));
        showModeCombobox->setCurrentIndex(1);
        topLayout->addLayout(lay);

        auto buttons = new QDialogButtonBox(QDialogButtonBox::Yes | QDialogButtonBox::No, dialog);

        topLayout->addWidget(buttons);
        dialog->setLayout(topLayout);

        connect(buttons, &QDialogButtonBox::accepted, dialog, &QDialog::accept);
        connect(buttons, &QDialogButtonBox::rejected, dialog, &QDialog::reject);
        connect(dialog, &QDialog::accepted, this, &KAccessApp::yesClicked);
        connect(dialog, &QDialog::rejected, this, &KAccessApp::noClicked);
    }
}

void KAccessApp::xkbControlsNotify(xcb_xkb_controls_notify_event_t *event)
{
    unsigned int newFeatures =
        event->enabledControls & (XCB_XKB_BOOL_CTRL_SLOW_KEYS | XCB_XKB_BOOL_CTRL_BOUNCE_KEYS | XCB_XKB_BOOL_CTRL_STICKY_KEYS | XCB_XKB_BOOL_CTRL_MOUSE_KEYS);

    if (newFeatures != features) {
        unsigned int enabled = newFeatures & ~features;
        unsigned int disabled = features & ~newFeatures;

        if (!m_mouseSettings.gestureConfirmation()) {
            requestedFeatures = enabled | (requestedFeatures & ~disabled);
            notifyChanges();
            features = newFeatures;
        } else {
            // set the AccessX features back to what they were. We will
            // apply the changes later if the user allows us to do that.
            readSettings();

            requestedFeatures = enabled | (requestedFeatures & ~disabled);

            enabled = requestedFeatures & ~features;
            disabled = features & ~requestedFeatures;

            QStringList enabledFeatures;
            QStringList disabledFeatures;

            if (enabled & XCB_XKB_BOOL_CTRL_SLOW_KEYS)
                enabledFeatures << i18n("Slow keys");
            else if (disabled & XCB_XKB_BOOL_CTRL_SLOW_KEYS)
                disabledFeatures << i18n("Slow keys");

            if (enabled & XCB_XKB_BOOL_CTRL_BOUNCE_KEYS)
                enabledFeatures << i18n("Bounce keys");
            else if (disabled & XCB_XKB_BOOL_CTRL_BOUNCE_KEYS)
                disabledFeatures << i18n("Bounce keys");

            if (enabled & XCB_XKB_BOOL_CTRL_STICKY_KEYS)
                enabledFeatures << i18n("Sticky keys");
            else if (disabled & XCB_XKB_BOOL_CTRL_STICKY_KEYS)
                disabledFeatures << i18n("Sticky keys");

            if (enabled & XCB_XKB_BOOL_CTRL_MOUSE_KEYS)
                enabledFeatures << i18n("Mouse keys");
            else if (disabled & XCB_XKB_BOOL_CTRL_MOUSE_KEYS)
                disabledFeatures << i18n("Mouse keys");

            QString question;
            switch (enabledFeatures.count()) {
            case 0:
                switch (disabledFeatures.count()) {
                case 1:
                    question = i18n("Do you really want to deactivate \"%1\"?", disabledFeatures[0]);
                    break;
                case 2:
                    question = i18n("Do you really want to deactivate \"%1\" and \"%2\"?", disabledFeatures[0], disabledFeatures[1]);
                    break;
                case 3:
                    question =
                        i18n("Do you really want to deactivate \"%1\", \"%2\" and \"%3\"?", disabledFeatures[0], disabledFeatures[1], disabledFeatures[2]);
                    break;
                case 4:
                    question = i18n("Do you really want to deactivate \"%1\", \"%2\", \"%3\" and \"%4\"?",
                                    disabledFeatures[0],
                                    disabledFeatures[1],
                                    disabledFeatures[2],
                                    disabledFeatures[3]);
                    break;
                }
                break;
            case 1:
                switch (disabledFeatures.count()) {
                case 0:
                    question = i18n("Do you really want to activate \"%1\"?", enabledFeatures[0]);
                    break;
                case 1:
                    question = i18n("Do you really want to activate \"%1\" and to deactivate \"%2\"?", enabledFeatures[0], disabledFeatures[0]);
                    break;
                case 2:
                    question = i18n("Do you really want to activate \"%1\" and to deactivate \"%2\" and \"%3\"?",
                                    enabledFeatures[0],
                                    disabledFeatures[0],
                                    disabledFeatures[1]);
                    break;
                case 3:
                    question = i18n("Do you really want to activate \"%1\" and to deactivate \"%2\", \"%3\" and \"%4\"?",
                                    enabledFeatures[0],
                                    disabledFeatures[0],
                                    disabledFeatures[1],
                                    disabledFeatures[2]);
                    break;
                }
                break;
            case 2:
                switch (disabledFeatures.count()) {
                case 0:
                    question = i18n("Do you really want to activate \"%1\" and \"%2\"?", enabledFeatures[0], enabledFeatures[1]);
                    break;
                case 1:
                    question = i18n("Do you really want to activate \"%1\" and \"%2\" and to deactivate \"%3\"?",
                                    enabledFeatures[0],
                                    enabledFeatures[1],
                                    disabledFeatures[0]);
                    break;
                case 2:
                    question = i18n("Do you really want to activate \"%1\", and \"%2\" and to deactivate \"%3\" and \"%4\"?",
                                    enabledFeatures[0],
                                    enabledFeatures[1],
                                    enabledFeatures[0],
                                    disabledFeatures[1]);
                    break;
                }
                break;
            case 3:
                switch (disabledFeatures.count()) {
                case 0:
                    question = i18n("Do you really want to activate \"%1\", \"%2\" and \"%3\"?", enabledFeatures[0], enabledFeatures[1], enabledFeatures[2]);
                    break;
                case 1:
                    question = i18n("Do you really want to activate \"%1\", \"%2\" and \"%3\" and to deactivate \"%4\"?",
                                    enabledFeatures[0],
                                    enabledFeatures[1],
                                    enabledFeatures[2],
                                    disabledFeatures[0]);
                    break;
                }
                break;
            case 4:
                question = i18n("Do you really want to activate \"%1\", \"%2\", \"%3\" and \"%4\"?",
                                enabledFeatures[0],
                                enabledFeatures[1],
                                enabledFeatures[2],
                                enabledFeatures[3]);
                break;
            }
            QString explanation;
            if (enabledFeatures.count() + disabledFeatures.count() == 1) {
                explanation = i18n("An application has requested to change this setting.");

                if (m_mouseSettings.gestures()) {
                    if ((enabled | disabled) == XCB_XKB_BOOL_CTRL_SLOW_KEYS)
                        explanation = i18n("You held down the Shift key for 8 seconds or an application has requested to change this setting.");
                    else if ((enabled | disabled) == XCB_XKB_BOOL_CTRL_STICKY_KEYS)
                        explanation = i18n("You pressed the Shift key 5 consecutive times or an application has requested to change this setting.");
                    else if ((enabled | disabled) == XCB_XKB_BOOL_CTRL_MOUSE_KEYS) {
                        QString shortcut = mouseKeysShortcut(QX11Info::display());
                        if (!shortcut.isEmpty() && !shortcut.isNull())
                            explanation = i18n("You pressed %1 or an application has requested to change this setting.", shortcut);
                    }
                }
            } else {
                if (m_mouseSettings.gestures())
                    explanation = i18n("An application has requested to change these settings, or you used a combination of several keyboard gestures.");
                else
                    explanation = i18n("An application has requested to change these settings.");
            }

            createDialogContents();
            featuresLabel->setText(question + QStringLiteral("\n\n") + explanation + QStringLiteral(" ")
                                   + i18n("These AccessX settings are needed for some users with motion impairments and can be configured in the KDE System "
                                          "Settings. You can also turn them on and off with standardized keyboard gestures.\n\nIf you do not need them, you "
                                          "can select \"Deactivate all AccessX features and gestures\"."));

            dialog->setWindowFlag(Qt::WindowStaysOnTopHint);
            KUserTimestamp::updateUserTimestamp(0);
            dialog->show();
        }
    }
}

void KAccessApp::notifyChanges()
{
    if (!m_mouseSettings.keyboardNotifyAccess())
        return;

    unsigned int enabled = requestedFeatures & ~features;
    unsigned int disabled = features & ~requestedFeatures;

    if (enabled & XCB_XKB_BOOL_CTRL_SLOW_KEYS)
        KNotification::event(QStringLiteral("slowkeys"),
                             i18n("Slow keys has been enabled. From now on, you need to press each key for a certain length of time before it gets accepted."));
    else if (disabled & XCB_XKB_BOOL_CTRL_SLOW_KEYS)
        KNotification::event(QStringLiteral("slowkeys"), i18n("Slow keys has been disabled."));

    if (enabled & XCB_XKB_BOOL_CTRL_BOUNCE_KEYS)
        KNotification::event(QStringLiteral("bouncekeys"),
                             i18n("Bounce keys has been enabled. From now on, each key will be blocked for a certain length of time after it was used."));
    else if (disabled & XCB_XKB_BOOL_CTRL_BOUNCE_KEYS)
        KNotification::event(QStringLiteral("bouncekeys"), i18n("Bounce keys has been disabled."));

    if (enabled & XCB_XKB_BOOL_CTRL_STICKY_KEYS)
        KNotification::event(QStringLiteral("stickykeys"),
                             i18n("Sticky keys has been enabled. From now on, modifier keys will stay latched after you have released them."));
    else if (disabled & XCB_XKB_BOOL_CTRL_STICKY_KEYS)
        KNotification::event(QStringLiteral("stickykeys"), i18n("Sticky keys has been disabled."));

    if (enabled & XCB_XKB_BOOL_CTRL_MOUSE_KEYS)
        KNotification::event(QStringLiteral("mousekeys"),
                             i18n("Mouse keys has been enabled. From now on, you can use the number pad of your keyboard in order to control the mouse."));
    else if (disabled & XCB_XKB_BOOL_CTRL_MOUSE_KEYS)
        KNotification::event(QStringLiteral("mousekeys"), i18n("Mouse keys has been disabled."));
}

void KAccessApp::applyChanges()
{
    notifyChanges();
    unsigned int enabled = requestedFeatures & ~features;
    unsigned int disabled = features & ~requestedFeatures;

    KConfigGroup config(KSharedConfig::openConfig(), QStringLiteral("Keyboard"));

    if (enabled & XCB_XKB_BOOL_CTRL_SLOW_KEYS)
        config.writeEntry("SlowKeys", true);
    else if (disabled & XCB_XKB_BOOL_CTRL_SLOW_KEYS)
        config.writeEntry("SlowKeys", false);

    if (enabled & XCB_XKB_BOOL_CTRL_BOUNCE_KEYS)
        config.writeEntry("BounceKeys", true);
    else if (disabled & XCB_XKB_BOOL_CTRL_BOUNCE_KEYS)
        config.writeEntry("BounceKeys", false);

    if (enabled & XCB_XKB_BOOL_CTRL_STICKY_KEYS)
        config.writeEntry("StickyKeys", true);
    else if (disabled & XCB_XKB_BOOL_CTRL_STICKY_KEYS)
        config.writeEntry("StickyKeys", false);

    KConfigGroup mousegrp(KSharedConfig::openConfig(), QStringLiteral("Mouse"));

    if (enabled & XCB_XKB_BOOL_CTRL_MOUSE_KEYS)
        mousegrp.writeEntry("MouseKeys", true);
    else if (disabled & XCB_XKB_BOOL_CTRL_MOUSE_KEYS)
        mousegrp.writeEntry("MouseKeys", false);
    mousegrp.sync();
    config.sync();
}

void KAccessApp::yesClicked()
{
    if (dialog)
        dialog->deleteLater();
    dialog = nullptr;

    KConfigGroup config(KSharedConfig::openConfig(), QStringLiteral("Keyboard"));
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

void KAccessApp::noClicked()
{
    if (dialog)
        dialog->deleteLater();
    dialog = nullptr;
    requestedFeatures = features;

    KConfigGroup config(KSharedConfig::openConfig(), QStringLiteral("Keyboard"));
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

void KAccessApp::dialogClosed()
{
    if (dialog != nullptr)
        dialog->deleteLater();
    dialog = nullptr;

    requestedFeatures = features;
}

void KAccessApp::setXkbOpcode(int opcode)
{
    xkb_opcode = opcode;
}
