/*
    SPDX-FileCopyrightText: 2000 Matthias Hölzer-Klüpfel <hoelzer@kde.org>
    SPDX-FileCopyrightText: 2014 Frederik Gladhorn <gladhorn@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "kcmaccess.h"

#include <math.h>
#include <stdlib.h>

#include <QApplication>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QFileDialog>
#include <QProcess>
#include <QQuickItem>
#include <QStandardPaths>
#include <QValidator>
#include <QWindow>
#include <QtGui/private/qtx11extras_p.h>

#include <KActionCollection>
#include <KConfigGroup>
#include <KGlobalAccel>
#include <KKeyServer>
#include <KLocalizedString>
#include <KNotifyConfigWidget>
#include <KPluginFactory>
#include <KSharedConfig>
#include <KShortcutsDialog>
#include <X11/XKBlib.h>
#include <X11/Xlib.h>

#define XK_MISCELLANY
#define XK_XKB_KEYS
#include <X11/keysymdef.h>

#include "kcmaccessibilityactivationgestures.h"
#include "kcmaccessibilitybell.h"
#include "kcmaccessibilitycolorblindnesscorrection.h"
#include "kcmaccessibilitydata.h"
#include "kcmaccessibilityinvert.h"
#include "kcmaccessibilitykeyboard.h"
#include "kcmaccessibilitykeyboardfilters.h"
#include "kcmaccessibilitymouse.h"
#include "kcmaccessibilityscreenreader.h"
#include "kcmaccessibilityshakecursor.h"

K_PLUGIN_FACTORY_WITH_JSON(KCMAccessFactory, "kcm_access.json", registerPlugin<KAccessConfig>(); registerPlugin<AccessibilityData>();)

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
        keyname = QKeySequence(Qt::META).toString() + QLatin1Char('+') + keyname;
    if ((modifiers & AltMask) != 0)
        keyname = QKeySequence(Qt::ALT).toString() + QLatin1Char('+') + keyname;
    if ((modifiers & ControlMask) != 0)
        keyname = QKeySequence(Qt::CTRL).toString() + QLatin1Char('+') + keyname;
    if ((modifiers & ShiftMask) != 0)
        keyname = QKeySequence(Qt::SHIFT).toString() + QLatin1Char('+') + keyname;

    return modifiers & ScrollMask & LockMask & NumMask ? i18n("Press %1 while NumLock, CapsLock and ScrollLock are active", keyname)
        : modifiers & ScrollMask & LockMask            ? i18n("Press %1 while CapsLock and ScrollLock are active", keyname)
        : modifiers & ScrollMask & NumMask             ? i18n("Press %1 while NumLock and ScrollLock are active", keyname)
        : modifiers & ScrollMask                       ? i18n("Press %1 while ScrollLock is active", keyname)
        : modifiers & LockMask & NumMask               ? i18n("Press %1 while NumLock and CapsLock are active", keyname)
        : modifiers & LockMask                         ? i18n("Press %1 while CapsLock is active", keyname)
        : modifiers & NumMask                          ? i18n("Press %1 while NumLock is active", keyname)
                                                       : i18n("Press %1", keyname);
}

class IntValidatorWithSuffix : public QIntValidator
{
    Q_GADGET
public:
    State validate(QString &text, int &pos) const override
    {
        QString strippedText;
        QRegularExpression re("\\d+");
        QRegularExpressionMatchIterator i = re.globalMatch(text);

        QString numbers;
        while (i.hasNext()) {
            QRegularExpressionMatch match = i.next();
            strippedText += match.captured();
        }
        return QIntValidator::validate(strippedText, pos);
    }
};

KAccessConfig::KAccessConfig(QObject *parent, const KPluginMetaData &metaData)
    : KQuickManagedConfigModule(parent, metaData)
    , m_data(new AccessibilityData(this))
    , m_desktopShortcutInfo(QX11Info::isPlatformX11() ? mouseKeysShortcut(QX11Info::display()) : QString())
{
    qmlRegisterAnonymousType<MouseSettings>("org.kde.plasma.access.kcm", 0);
    qmlRegisterAnonymousType<BellSettings>("org.kde.plasma.access.kcm", 0);
    qmlRegisterAnonymousType<KeyboardSettings>("org.kde.plasma.access.kcm", 0);
    qmlRegisterAnonymousType<KeyboardFiltersSettings>("org.kde.plasma.access.kcm", 0);
    qmlRegisterAnonymousType<ActivationGesturesSettings>("org.kde.plasma.access.kcm", 0);
    qmlRegisterAnonymousType<ScreenReaderSettings>("org.kde.plasma.access.kcm", 0);
    qmlRegisterAnonymousType<ShakeCursorSettings>("org.kde.plasma.access.kcm", 0);
    qmlRegisterAnonymousType<ColorblindnessCorrectionSettings>("org.kde.plasma.access.kcm", 0);
    qmlRegisterAnonymousType<InvertSettings>("org.kde.plasma.access.kcm", 0);
    qmlRegisterType<IntValidatorWithSuffix>("org.kde.plasma.access.kcm", 0, 0, "IntValidatorWithSuffix");

    int tryOrcaRun = QProcess::execute(QStringLiteral("orca"), {QStringLiteral("--version")});
    m_screenReaderInstalled = tryOrcaRun != -2;

    setButtons(KAbstractConfigModule::Apply | KAbstractConfigModule::Default | KAbstractConfigModule::Help);

    connect(m_data->bellSettings(), &BellSettings::configChanged, this, &KAccessConfig::bellIsDefaultsChanged);
    connect(m_data->mouseSettings(), &MouseSettings::configChanged, this, &KAccessConfig::mouseIsDefaultsChanged);
    connect(m_data->activationGesturesSettings(), &ActivationGesturesSettings::configChanged, this, &KAccessConfig::activationGesturesIsDefaultsChanged);
    connect(m_data->keyboardFiltersSettings(), &KeyboardFiltersSettings::configChanged, this, &KAccessConfig::keyboardFiltersIsDefaultsChanged);
    connect(m_data->keyboardSettings(), &KeyboardSettings::configChanged, this, &KAccessConfig::keyboardModifiersIsDefaultsChanged);
    connect(m_data->screenReaderSettings(), &ScreenReaderSettings::configChanged, this, &KAccessConfig::screenReaderIsDefaultsChanged);
    connect(m_data->shakeCursorSettings(), &ShakeCursorSettings::configChanged, this, &KAccessConfig::shakeCursorIsDefaultsChanged);
    connect(m_data->colorblindnessCorrectionSettings(),
            &ColorblindnessCorrectionSettings::configChanged,
            this,
            &KAccessConfig::colorblindnessCorrectionIsDefaultsChanged);
    connect(m_data->invertSettings(), &InvertSettings::configChanged, this, &KAccessConfig::invertIsDefaultsChanged);
}

KAccessConfig::~KAccessConfig()
{
}

void KAccessConfig::configureKNotify()
{
    KNotifyConfigWidget::configure(QApplication::activeWindow(), QStringLiteral("kaccess"));
}

void KAccessConfig::configureInvertShortcuts()
{
    KShortcutsDialog *dialog = new KShortcutsDialog(KShortcutsEditor::GlobalAction, KShortcutsEditor::LetterShortcutsDisallowed);

    KActionCollection *actionCollection = new KActionCollection(dialog, QStringLiteral("kwin"));
    actionCollection->setComponentDisplayName(i18n("KWin"));

    QAction *a = actionCollection->addAction(QStringLiteral("Invert"));
    a->setText(i18n("Toggle Invert Effect"));
    a->setProperty("isConfigurationAction", true);
    KGlobalAccel::self()->setDefaultShortcut(a, QList<QKeySequence>() << (Qt::CTRL | Qt::META | Qt::Key_I));
    KGlobalAccel::self()->setShortcut(a, QList<QKeySequence>() << (Qt::CTRL | Qt::META | Qt::Key_I));

    QAction *b = actionCollection->addAction(QStringLiteral("InvertWindow"));
    b->setText(i18n("Toggle Invert Effect on Window"));
    b->setProperty("isConfigurationAction", true);
    KGlobalAccel::self()->setDefaultShortcut(b, QList<QKeySequence>() << (Qt::CTRL | Qt::META | Qt::Key_U));
    KGlobalAccel::self()->setShortcut(b, QList<QKeySequence>() << (Qt::CTRL | Qt::META | Qt::Key_U));

    dialog->addCollection(actionCollection);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->winId();
    dialog->windowHandle()->setTransientParent(QApplication::activeWindow()->windowHandle());
    dialog->setWindowModality(Qt::WindowModal);

    dialog->configure();
}

void KAccessConfig::launchOrcaConfiguration()
{
    const QStringList gsettingArgs = {QStringLiteral("set"),
                                      QStringLiteral("org.gnome.desktop.a11y.applications"),
                                      QStringLiteral("screen-reader-enabled"),
                                      QStringLiteral("true")};

    int ret = QProcess::execute(QStringLiteral("gsettings"), gsettingArgs);
    if (ret) {
        const QString errorStr = QLatin1String("gsettings ") + gsettingArgs.join(QLatin1Char(' '));
        setOrcaLaunchFeedback(i18n("Could not set gsettings for Orca: \"%1\" failed", errorStr));
        return;
    }

    qint64 pid = 0;
    bool started = QProcess::startDetached(QStringLiteral("orca"), {QStringLiteral("--setup")}, QString(), &pid);
    if (!started) {
        setOrcaLaunchFeedback(i18n("Error: Could not launch \"orca --setup\""));
    }
}

void KAccessConfig::save()
{
    const bool shakeCursorSaveNeeded = m_data->shakeCursorSettings()->findItem(QStringLiteral("ShakeCursor"))->isSaveNeeded();
    const bool shakeCursorMagnificationSaveNeeded = m_data->shakeCursorSettings()->findItem(QStringLiteral("ShakeCursorMagnification"))->isSaveNeeded();

    const bool colorblindnessCorrectionSaveNeeded =
        m_data->colorblindnessCorrectionSettings()->findItem(QStringLiteral("ColorblindnessCorrection"))->isSaveNeeded();
    const bool colorblindnessCorrectionSettingsSaveNeeded = m_data->colorblindnessCorrectionSettings()->findItem(QStringLiteral("Mode"))->isSaveNeeded()
        || m_data->colorblindnessCorrectionSettings()->findItem(QStringLiteral("Intensity"))->isSaveNeeded();

    const bool invertSaveNeeded = m_data->invertSettings()->findItem(QStringLiteral("Invert"))->isSaveNeeded();

    KQuickManagedConfigModule::save();

    if (bellSettings()->systemBell() || bellSettings()->customBell() || bellSettings()->visibleBell()) {
        KConfig _cfg(QStringLiteral("kdeglobals"), KConfig::NoGlobals);
        KConfigGroup cfg(&_cfg, QStringLiteral("General"));
        cfg.writeEntry("UseSystemBell", true);
        cfg.sync();
    }

    // make kaccess reread the configuration
    // turning a11y features off needs to be done by kaccess
    // so run it to clear any enabled features and it will exit if it should
    QProcess::startDetached(QStringLiteral("kaccess"), {});

    if (shakeCursorSaveNeeded) {
        QDBusMessage reloadMessage =
            QDBusMessage::createMethodCall(QStringLiteral("org.kde.KWin"),
                                           QStringLiteral("/Effects"),
                                           QStringLiteral("org.kde.kwin.Effects"),
                                           shakeCursorSettings()->shakeCursor() ? QStringLiteral("loadEffect") : QStringLiteral("unloadEffect"));
        reloadMessage.setArguments({QStringLiteral("shakecursor")});
        QDBusConnection::sessionBus().call(reloadMessage);
    }
    if (shakeCursorMagnificationSaveNeeded) {
        QDBusMessage reconfigureMessage = QDBusMessage::createMethodCall(QStringLiteral("org.kde.KWin"),
                                                                         QStringLiteral("/Effects"),
                                                                         QStringLiteral("org.kde.kwin.Effects"),
                                                                         QStringLiteral("reconfigureEffect"));
        reconfigureMessage.setArguments({QStringLiteral("shakecursor")});
        QDBusConnection::sessionBus().call(reconfigureMessage);
    }

    if (colorblindnessCorrectionSaveNeeded) {
        QDBusMessage reloadMessage = QDBusMessage::createMethodCall(
            QStringLiteral("org.kde.KWin"),
            QStringLiteral("/Effects"),
            QStringLiteral("org.kde.kwin.Effects"),
            colorblindnessCorrectionSettings()->colorblindnessCorrection() ? QStringLiteral("loadEffect") : QStringLiteral("unloadEffect"));
        reloadMessage.setArguments({QStringLiteral("colorblindnesscorrection")});
        QDBusConnection::sessionBus().call(reloadMessage);
    }
    if (colorblindnessCorrectionSettingsSaveNeeded) {
        QDBusMessage reconfigureMessage = QDBusMessage::createMethodCall(QStringLiteral("org.kde.KWin"),
                                                                         QStringLiteral("/Effects"),
                                                                         QStringLiteral("org.kde.kwin.Effects"),
                                                                         QStringLiteral("reconfigureEffect"));
        reconfigureMessage.setArguments({QStringLiteral("colorblindnesscorrection")});
        QDBusConnection::sessionBus().call(reconfigureMessage);
    }

    if (invertSaveNeeded) {
        QDBusMessage reloadMessage = QDBusMessage::createMethodCall(QStringLiteral("org.kde.KWin"),
                                                                    QStringLiteral("/Effects"),
                                                                    QStringLiteral("org.kde.kwin.Effects"),
                                                                    invertSettings()->invert() ? QStringLiteral("loadEffect") : QStringLiteral("unloadEffect"));
        reloadMessage.setArguments({QStringLiteral("invert")});
        QDBusConnection::sessionBus().call(reloadMessage);
    }
}

QString KAccessConfig::orcaLaunchFeedback() const
{
    return m_orcaLaunchFeedback;
}

void KAccessConfig::setOrcaLaunchFeedback(const QString &value)
{
    if (m_orcaLaunchFeedback != value) {
        m_orcaLaunchFeedback = value;
        Q_EMIT orcaLaunchFeedbackChanged();
    }
}

bool KAccessConfig::orcaInstalled()
{
    int tryOrcaRun = QProcess::execute(QStringLiteral("orca"), {QStringLiteral("--version")});
    // If the process cannot be started, -2 is returned.
    return tryOrcaRun != -2;
}

MouseSettings *KAccessConfig::mouseSettings() const
{
    return m_data->mouseSettings();
}

BellSettings *KAccessConfig::bellSettings() const
{
    return m_data->bellSettings();
}

KeyboardSettings *KAccessConfig::keyboardSettings() const
{
    return m_data->keyboardSettings();
}

KeyboardFiltersSettings *KAccessConfig::keyboardFiltersSettings() const
{
    return m_data->keyboardFiltersSettings();
}

ActivationGesturesSettings *KAccessConfig::activationGesturesSettings() const
{
    return m_data->activationGesturesSettings();
}

ScreenReaderSettings *KAccessConfig::screenReaderSettings() const
{
    return m_data->screenReaderSettings();
}

ShakeCursorSettings *KAccessConfig::shakeCursorSettings() const
{
    return m_data->shakeCursorSettings();
}

ColorblindnessCorrectionSettings *KAccessConfig::colorblindnessCorrectionSettings() const
{
    return m_data->colorblindnessCorrectionSettings();
}

InvertSettings *KAccessConfig::invertSettings() const
{
    return m_data->invertSettings();
}

bool KAccessConfig::bellIsDefaults() const
{
    return bellSettings()->isDefaults();
}

bool KAccessConfig::mouseIsDefaults() const
{
    return mouseSettings()->isDefaults();
}

bool KAccessConfig::keyboardFiltersIsDefaults() const
{
    return keyboardFiltersSettings()->isDefaults();
}

bool KAccessConfig::keyboardModifiersIsDefaults() const
{
    return keyboardSettings()->isDefaults();
}

bool KAccessConfig::activationGesturesIsDefaults() const
{
    return activationGesturesSettings()->isDefaults();
}

bool KAccessConfig::screenReaderIsDefaults() const
{
    return screenReaderSettings()->isDefaults();
}

bool KAccessConfig::shakeCursorIsDefaults() const
{
    return shakeCursorSettings()->isDefaults();
}

bool KAccessConfig::colorblindnessCorrectionIsDefaults() const
{
    return colorblindnessCorrectionSettings()->isDefaults();
}

bool KAccessConfig::invertIsDefaults() const
{
    return invertSettings()->isDefaults();
}

#include "kcmaccess.moc"
#include "moc_kcmaccess.cpp"
