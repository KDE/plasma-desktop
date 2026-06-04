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
#include <QDBusConnection>
#include <QDBusMessage>
#include <QLoggingCategory>
#include <QProcess>
#include <QScreen>
#include <QTimer>
#include <QtGui/private/qtx11extras_p.h>

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

    initMasks();
    XkbStateRec state_return;
    XkbGetState(QX11Info::display(), XkbUseCoreKbd, &state_return);
    unsigned char latched = XkbStateMods(&state_return);
    unsigned char locked = XkbModLocks(&state_return);
    state = ((int)locked) << 8 | latched;

    toggleScreenReaderAction->setText(i18n("Toggle Screen Reader On and Off"));
    toggleScreenReaderAction->setObjectName(QStringLiteral("Toggle Screen Reader On and Off"));
    toggleScreenReaderAction->setProperty("componentDisplayName", i18nc("Name for kaccess shortcuts category", "Accessibility"));
    KGlobalAccel::self()->setGlobalShortcut(toggleScreenReaderAction, Qt::META | Qt::ALT | Qt::Key_S);
    connect(toggleScreenReaderAction, &QAction::triggered, this, &KAccessApp::toggleScreenReader);

    auto service = new KDBusService(KDBusService::Unique, this);
    connect(service, &KDBusService::activateRequested, this, &KAccessApp::newInstance);

    QTimer::singleShot(0, this, &KAccessApp::readSettings);
}

KAccessApp::~KAccessApp()
{
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

    setScreenReaderEnabled(m_screenReaderSettings.enabled());
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
            case XCB_XKB_EVENT_TYPE_BELL_NOTIFY:
                xkbBellNotify(reinterpret_cast<xcb_xkb_bell_notify_event_t *>(event));
                break;
            }
            return true;
        }
    }
    return false;
}

void KAccessApp::xkbBellNotify(xcb_xkb_bell_notify_event_t *event)
{
    // bail out if we should not really ring
    if (event->eventOnly)
        return;

    QDBusMessage msg = QDBusMessage::createMethodCall(QStringLiteral("org.kde.KWin"),
                                                      QStringLiteral("/org/kde/KWin/Effect/SystemBell1"),
                                                      QStringLiteral("org.kde.KWin.Effect.SystemBell1"),
                                                      QStringLiteral("triggerWindow"));

    QDBusConnection::sessionBus().call(msg, QDBus::NoBlock);
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

void KAccessApp::setXkbOpcode(int opcode)
{
    xkb_opcode = opcode;
}

#include "moc_kaccess.cpp"
