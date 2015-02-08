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
#include <QStandardPaths>

K_PLUGIN_FACTORY(KAccessConfigFactory, registerPlugin<KAccessConfig>();)
K_EXPORT_PLUGIN(KAccessConfigFactory("kcmaccess"))

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

    unsigned int AltMask   = KKeyServer::modXAlt();
    unsigned int WinMask   = KKeyServer::modXMeta();
    unsigned int NumMask   = KKeyServer::modXNumLock();
    unsigned int ScrollMask = KKeyServer::modXScrollLock();

    unsigned int MetaMask  = XkbKeysymToModifiers(display, XK_Meta_L);
    unsigned int SuperMask = XkbKeysymToModifiers(display, XK_Super_L);
    unsigned int HyperMask = XkbKeysymToModifiers(display, XK_Hyper_L);
    unsigned int AltGrMask = XkbKeysymToModifiers(display, XK_Mode_switch)
                             | XkbKeysymToModifiers(display, XK_ISO_Level3_Shift)
                             | XkbKeysymToModifiers(display, XK_ISO_Level3_Latch)
                             | XkbKeysymToModifiers(display, XK_ISO_Level3_Lock);

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
        new KAboutData("kcmaccess", i18n("KDE Accessibility Tool"), "1.0",
                       QString(), KAboutLicense::GPL, i18n("(c) 2000, Matthias Hoelzer-Kluepfel"));

    about->addAuthor(i18n("Matthias Hoelzer-Kluepfel"), i18n("Author") , QStringLiteral("hoelzer@kde.org"));
    setAboutData(about);

    ui.setupUi(this);

    connect(ui.soundButton, &QPushButton::clicked, this, &KAccessConfig::selectSound);
    connect(ui.customBell, &QCheckBox::clicked, this, &KAccessConfig::checkAccess);
    connect(ui.systemBell, &QCheckBox::clicked, this, &KAccessConfig::configChanged);
    connect(ui.customBell, &QCheckBox::clicked, this, &KAccessConfig::configChanged);
    connect(ui.soundEdit, &QLineEdit::textChanged, this, &KAccessConfig::configChanged);

    connect(ui.invertScreen, &QRadioButton::clicked, this, &KAccessConfig::configChanged);
    connect(ui.flashScreen, &QRadioButton::clicked, this, &KAccessConfig::configChanged);
    connect(ui.visibleBell, &QCheckBox::clicked, this, &KAccessConfig::configChanged);
    connect(ui.visibleBell, &QCheckBox::clicked, this, &KAccessConfig::checkAccess);
    connect(ui.colorButton, &KColorButton::clicked, this, &KAccessConfig::changeFlashScreenColor);

    connect(ui.invertScreen, &QRadioButton::clicked, this, &KAccessConfig::invertClicked);
    connect(ui.flashScreen, &QRadioButton::clicked, this, &KAccessConfig::flashClicked);

    connect(ui.duration, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &KAccessConfig::configChanged);


    // modifier key settings -------------------------------

    connect(ui.stickyKeys, &QCheckBox::clicked, this, &KAccessConfig::configChanged);
    connect(ui.stickyKeysLock, &QCheckBox::clicked, this, &KAccessConfig::configChanged);
    connect(ui.stickyKeysAutoOff, &QCheckBox::clicked, this, &KAccessConfig::configChanged);
    connect(ui.stickyKeys, &QCheckBox::clicked, this, &KAccessConfig::checkAccess);

    connect(ui.stickyKeysBeep, &QCheckBox::clicked, this, &KAccessConfig::configChanged);
    connect(ui.toggleKeysBeep, &QCheckBox::clicked, this, &KAccessConfig::configChanged);
    connect(ui.kNotifyModifiers, &QCheckBox::clicked, this, &KAccessConfig::configChanged);
    connect(ui.kNotifyModifiers, &QCheckBox::clicked, this, &KAccessConfig::checkAccess);
    connect(ui.kNotifyModifiersButton, &QPushButton::clicked, this, &KAccessConfig::configureKNotify);

    // key filter settings ---------------------------------

    QVBoxLayout* vbox = new QVBoxLayout(ui.tabKeyFilters);
    QGroupBox* grp = new QGroupBox(i18n("Slo&w Keys"), ui.tabKeyFilters);
    QBoxLayout* layout = new QHBoxLayout;
    grp->setLayout(layout);
    vbox->addWidget(grp);

    QVBoxLayout* vvbox = new QVBoxLayout();
    layout->addLayout(vvbox);

    slowKeys = new QCheckBox(i18n("&Use slow keys"), grp);
    vvbox->addWidget(slowKeys);

    QHBoxLayout* hbox = new QHBoxLayout();
    vvbox->addLayout(hbox);
    hbox->addSpacing(24);
    slowKeysDelay = new KDoubleNumInput(grp);
    slowKeysDelay->setRange(50, 10000, 100);
    slowKeysDelay->setExponentRatio(2);
    slowKeysDelay->setDecimals(0);
    slowKeysDelay->setSuffix(i18n(" msec"));
    slowKeysDelay->setLabel(i18n("Acceptance dela&y:"), Qt::AlignVCenter | Qt::AlignLeft);
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

    grp = new QGroupBox(i18n("Bounce Keys"), ui.tabKeyFilters);
    layout = new QHBoxLayout;
    grp->setLayout(layout);
    vbox->addWidget(grp);

    vvbox = new QVBoxLayout();
    layout->addLayout(vvbox);

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
    bounceKeysDelay->setLabel(i18n("D&ebounce time:"), Qt::AlignVCenter | Qt::AlignLeft);;
    hbox->addWidget(bounceKeysDelay);

    hbox = new QHBoxLayout();
    vvbox->addLayout(hbox);
    hbox->addSpacing(24);
    bounceKeysRejectBeep = new QCheckBox(i18n("Use the system bell whenever a key is rejected"), grp);
    hbox->addWidget(bounceKeysRejectBeep);

    connect(slowKeysDelay, &KDoubleNumInput::valueChanged, this, &KAccessConfig::configChanged);
    connect(slowKeys, &QCheckBox::clicked, this, &KAccessConfig::configChanged);
    connect(slowKeys, &QCheckBox::clicked, this, &KAccessConfig::checkAccess);

    connect(slowKeysPressBeep, &QCheckBox::clicked, this, &KAccessConfig::configChanged);
    connect(slowKeysAcceptBeep, &QCheckBox::clicked, this, &KAccessConfig::configChanged);
    connect(slowKeysRejectBeep, &QCheckBox::clicked, this, &KAccessConfig::configChanged);

    connect(bounceKeysDelay, &KDoubleNumInput::valueChanged, this, &KAccessConfig::configChanged);
    connect(bounceKeys, &QCheckBox::clicked, this, &KAccessConfig::configChanged);
    connect(bounceKeysRejectBeep, &QCheckBox::clicked, this, &KAccessConfig::configChanged);
    connect(bounceKeys, &QCheckBox::clicked, this, &KAccessConfig::checkAccess);

    vbox->addStretch();


    // gestures --------------------------------------------
    vbox = new QVBoxLayout(ui.tabActivationGestures);

    grp = new QGroupBox(i18n("Activation Gestures"), ui.tabActivationGestures);
    layout = new QHBoxLayout;
    grp->setLayout(layout);
    vbox->addWidget(grp);

    vvbox = new QVBoxLayout();
    layout->addLayout(vvbox);

    gestures = new QCheckBox(i18n("Use gestures for activating sticky keys and slow keys"), grp);
    vvbox->addWidget(gestures);
    QString shortcut = mouseKeysShortcut(QX11Info::display());
    if (shortcut.isEmpty())
        gestures->setWhatsThis(i18n("Here you can activate keyboard gestures that turn on the following features: \n"
                                    "Sticky keys: Press Shift key 5 consecutive times\n"
                                    "Slow keys: Hold down Shift for 8 seconds"));
    else
        gestures->setWhatsThis(i18n("Here you can activate keyboard gestures that turn on the following features: \n"
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
    timeoutDelay->setLabel(i18n("Timeout:"), Qt::AlignVCenter | Qt::AlignLeft);;
    hbox->addWidget(timeoutDelay);

    grp = new QGroupBox(i18n("Notification"), ui.tabActivationGestures);
    layout = new QHBoxLayout;
    grp->setLayout(layout);
    vbox->addWidget(grp);

    vvbox = new QVBoxLayout();
    layout->addLayout(vvbox);

    accessxBeep = new QCheckBox(i18n("Use the system bell whenever a gesture is used to turn an accessibility feature on or off"), grp);
    vvbox->addWidget(accessxBeep);

    gestureConfirmation = new QCheckBox(i18n("Show a confirmation dialog whenever a keyboard accessibility feature is turned on or off"), grp);
    vvbox->addWidget(gestureConfirmation);
    gestureConfirmation->setWhatsThis(i18n("If this option is checked, KDE will show a confirmation dialog whenever a keyboard accessibility feature is turned on or off.\nEnsure you know what you are doing if you uncheck it, as the keyboard accessibility settings will then always be applied without confirmation."));

    kNotifyAccessX = new QCheckBox(i18n("Use Plasma's system notification mechanism whenever a keyboard accessibility feature is turned on or off"), grp);
    vvbox->addWidget(kNotifyAccessX);

    hbox = new QHBoxLayout();
    vvbox->addLayout(hbox);
    hbox->addStretch(1);
    kNotifyAccessXButton = new QPushButton(i18n("Configure &Notifications..."), grp);
    kNotifyAccessXButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    hbox->addWidget(kNotifyAccessXButton);

    connect(gestures, &QCheckBox::clicked, this, &KAccessConfig::configChanged);
    connect(timeout, &QCheckBox::clicked, this, &KAccessConfig::configChanged);
    connect(timeout, &QCheckBox::clicked, this, &KAccessConfig::checkAccess);
    connect(timeoutDelay, &KIntNumInput::valueChanged, this, &KAccessConfig::configChanged);
    connect(accessxBeep, &QCheckBox::clicked, this, &KAccessConfig::configChanged);
    connect(gestureConfirmation, &QCheckBox::clicked, this, &KAccessConfig::configChanged);
    connect(kNotifyAccessX, &QCheckBox::clicked, this, &KAccessConfig::configChanged);
    connect(kNotifyAccessX, &QCheckBox::clicked, this, &KAccessConfig::checkAccess);
    connect(kNotifyAccessXButton, &QPushButton::clicked, this, &KAccessConfig::configureKNotify);

    vbox->addStretch();
}


KAccessConfig::~KAccessConfig()
{
}

void KAccessConfig::configureKNotify()
{
    KNotifyConfigWidget::configure(this, "kaccess");
}

void KAccessConfig::changeFlashScreenColor()
{
    ui.invertScreen->setChecked(false);
    ui.flashScreen->setChecked(true);
    configChanged();
}

void KAccessConfig::selectSound()
{
    QStringList list = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, QLatin1String("sound/"));
    QString start;
    if (list.count() > 0)
        start = list[0];
    QString fname = QFileDialog::getOpenFileName(this, QString(), start);
    if (!fname.isEmpty())
        ui.soundEdit->setText(fname);
}


void KAccessConfig::configChanged()
{
    emit changed(true);
}


void KAccessConfig::load()
{
    KConfigGroup cg(KSharedConfig::openConfig("kaccessrc"), "Bell");

    ui.systemBell->setChecked(cg.readEntry("SystemBell", true));
    ui.customBell->setChecked(cg.readEntry("ArtsBell", false));
    ui.soundEdit->setText(cg.readPathEntry("ArtsBellFile", QString()));

    ui.visibleBell->setChecked(cg.readEntry("VisibleBell", false));
    ui.invertScreen->setChecked(cg.readEntry("VisibleBellInvert", true));
    ui.flashScreen->setChecked(!ui.invertScreen->isChecked());
    ui.colorButton->setColor(cg.readEntry("VisibleBellColor", QColor(Qt::red)));

    ui.duration->setValue(cg.readEntry("VisibleBellPause", 500));

    KConfigGroup keyboardGroup(KSharedConfig::openConfig("kaccessrc"), "Keyboard");

    ui.stickyKeys->setChecked(keyboardGroup.readEntry("StickyKeys", false));
    ui.stickyKeysLock->setChecked(keyboardGroup.readEntry("StickyKeysLatch", true));
    ui.stickyKeysAutoOff->setChecked(keyboardGroup.readEntry("StickyKeysAutoOff", false));
    ui.stickyKeysBeep->setChecked(keyboardGroup.readEntry("StickyKeysBeep", true));
    ui.toggleKeysBeep->setChecked(keyboardGroup.readEntry("ToggleKeysBeep", false));
    ui.kNotifyModifiers->setChecked(keyboardGroup.readEntry("kNotifyModifiers", false));

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

    cg.writeEntry("SystemBell", ui.systemBell->isChecked());
    cg.writeEntry("ArtsBell", ui.customBell->isChecked());
    cg.writePathEntry("ArtsBellFile", ui.soundEdit->text());

    cg.writeEntry("VisibleBell", ui.visibleBell->isChecked());
    cg.writeEntry("VisibleBellInvert", ui.invertScreen->isChecked());
    cg.writeEntry("VisibleBellColor", ui.colorButton->color());

    cg.writeEntry("VisibleBellPause", ui.duration->value());

    KConfigGroup keyboardGroup(KSharedConfig::openConfig("kaccessrc"), "Keyboard");

    keyboardGroup.writeEntry("StickyKeys", ui.stickyKeys->isChecked());
    keyboardGroup.writeEntry("StickyKeysLatch", ui.stickyKeysLock->isChecked());
    keyboardGroup.writeEntry("StickyKeysAutoOff", ui.stickyKeysAutoOff->isChecked());
    keyboardGroup.writeEntry("StickyKeysBeep", ui.stickyKeysBeep->isChecked());
    keyboardGroup.writeEntry("ToggleKeysBeep", ui.toggleKeysBeep->isChecked());
    keyboardGroup.writeEntry("kNotifyModifiers", ui.kNotifyModifiers->isChecked());

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

    if (ui.systemBell->isChecked() ||
        ui.customBell->isChecked() ||
        ui.visibleBell->isChecked()) {
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
    ui.systemBell->setChecked(true);
    ui.customBell->setChecked(false);
    ui.soundEdit->setText(QString());

    ui.visibleBell->setChecked(false);
    ui.invertScreen->setChecked(true);
    ui.flashScreen->setChecked(false);
    ui.colorButton->setColor(QColor(Qt::red));

    ui.duration->setValue(500);

    slowKeys->setChecked(false);
    slowKeysDelay->setValue(500);
    slowKeysPressBeep->setChecked(true);
    slowKeysAcceptBeep->setChecked(true);
    slowKeysRejectBeep->setChecked(true);

    bounceKeys->setChecked(false);
    bounceKeysDelay->setValue(500);
    bounceKeysRejectBeep->setChecked(true);

    ui.stickyKeys->setChecked(false);
    ui.stickyKeysLock->setChecked(true);
    ui.stickyKeysAutoOff->setChecked(false);
    ui.stickyKeysBeep->setChecked(true);
    ui.toggleKeysBeep->setChecked(false);
    ui.kNotifyModifiers->setChecked(false);

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
    ui.flashScreen->setChecked(false);
}


void KAccessConfig::flashClicked()
{
    ui.invertScreen->setChecked(false);
}


void KAccessConfig::checkAccess()
{
    bool custom = ui.customBell->isChecked();
    ui.soundEdit->setEnabled(custom);
    ui.soundButton->setEnabled(custom);
    ui.soundLabel->setEnabled(custom);

    bool visible = ui.visibleBell->isChecked();
    ui.invertScreen->setEnabled(visible);
    ui.flashScreen->setEnabled(visible);
    ui.colorButton->setEnabled(visible);
    ui.duration->setEnabled(visible);

    bool sticky = ui.stickyKeys->isChecked();
    ui.stickyKeysLock->setEnabled(sticky);
    ui.stickyKeysAutoOff->setEnabled(sticky);
    ui.stickyKeysBeep->setEnabled(sticky);

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

