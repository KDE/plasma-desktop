/*
    Copyright 1997 Mark Donohoe
    Copyright 1999 Lars Knoll
    Copyright 2000 Rik Hemsley

    Ported to kcontrol2 by Geert Jansen.

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <config-workspace.h>

#include "fonts.h"

#include <stdlib.h>

#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <qplatformdefs.h>

//Added by qt3to4:
#include <QPixmap>
#include <QByteArray>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFormLayout>

#include <KFontDialog>
#include <KAcceleratorManager>
#include <KApplication>
#include <KGlobalSettings>
#include <KMessageBox>
#include <KProcess>
#include <KConfig>
#include <KStandardDirs>
#include <KDebug>
#include <KLocalizedString>
#include <KPluginFactory>
#include <KDoubleNumInput>

#include "../krdb/krdb.h"

#ifdef HAVE_FREETYPE
#include <ft2build.h>
#ifdef FT_LCD_FILTER_H
#include FT_FREETYPE_H
#include FT_LCD_FILTER_H
#endif
#endif

#if HAVE_X11
#include <X11/Xlib.h>
#endif

#if HAVE_X11
// X11 headers
#undef Bool
#undef Unsorted
#undef None
#endif

static const char *const aa_rgb_xpm[] = {
    "12 12 3 1",
    "a c #0000ff",
    "# c #00ff00",
    ". c #ff0000",
    "....####aaaa",
    "....####aaaa",
    "....####aaaa",
    "....####aaaa",
    "....####aaaa",
    "....####aaaa",
    "....####aaaa",
    "....####aaaa",
    "....####aaaa",
    "....####aaaa",
    "....####aaaa",
    "....####aaaa"
};
static const char *const aa_bgr_xpm[] = {
    "12 12 3 1",
    ". c #0000ff",
    "# c #00ff00",
    "a c #ff0000",
    "....####aaaa",
    "....####aaaa",
    "....####aaaa",
    "....####aaaa",
    "....####aaaa",
    "....####aaaa",
    "....####aaaa",
    "....####aaaa",
    "....####aaaa",
    "....####aaaa",
    "....####aaaa",
    "....####aaaa"
};
static const char *const aa_vrgb_xpm[] = {
    "12 12 3 1",
    "a c #0000ff",
    "# c #00ff00",
    ". c #ff0000",
    "............",
    "............",
    "............",
    "............",
    "############",
    "############",
    "############",
    "############",
    "aaaaaaaaaaaa",
    "aaaaaaaaaaaa",
    "aaaaaaaaaaaa",
    "aaaaaaaaaaaa"
};
static const char *const aa_vbgr_xpm[] = {
    "12 12 3 1",
    ". c #0000ff",
    "# c #00ff00",
    "a c #ff0000",
    "............",
    "............",
    "............",
    "............",
    "############",
    "############",
    "############",
    "############",
    "aaaaaaaaaaaa",
    "aaaaaaaaaaaa",
    "aaaaaaaaaaaa",
    "aaaaaaaaaaaa"
};

static const char *const *const aaPixmaps[] = { 0, 0, aa_rgb_xpm, aa_bgr_xpm, aa_vrgb_xpm, aa_vbgr_xpm };

/**** DLL Interface ****/
K_PLUGIN_FACTORY(FontFactory, registerPlugin<KFonts>();)

/**** FontUseItem ****/

FontUseItem::FontUseItem(
    QWidget *parent,
    const QString &name,
    const QString &grp,
    const QString &key,
    const QString &rc,
    const QFont &default_fnt,
    bool f
)
    : KFontRequester(parent, f),
      _rcfile(rc),
      _rcgroup(grp),
      _rckey(key),
      _default(default_fnt)
{
    KAcceleratorManager::setNoAccel(this);
    setTitle(name);
    readFont();
}

void FontUseItem::setDefault()
{
    setFont(_default, isFixedOnly());
}

void FontUseItem::readFont()
{
    KConfig *config;

    bool deleteme = false;
    if (_rcfile.isEmpty()) {
        config = KSharedConfig::openConfig().data();
    } else {
        config = new KConfig(_rcfile);
        deleteme = true;
    }

    KConfigGroup group(config, _rcgroup);
    QFont tmpFnt(_default);
    setFont(group.readEntry(_rckey, tmpFnt), isFixedOnly());
    if (deleteme) {
        delete config;
    }
}

void FontUseItem::writeFont()
{
    KConfig *config;

    if (_rcfile.isEmpty()) {
        config = KSharedConfig::openConfig().data();
        KConfigGroup(config, _rcgroup).writeEntry(_rckey, font(), KConfig::Normal | KConfig::Global);
    } else {
        config = new KConfig(KStandardDirs::locateLocal("config", _rcfile));
        KConfigGroup(config, _rcgroup).writeEntry(_rckey, font());
        config->sync();
        delete config;
    }
}

void FontUseItem::applyFontDiff(const QFont &fnt, int fontDiffFlags)
{
    QFont _font(font());

    if (fontDiffFlags & KFontChooser::FontDiffSize) {
        _font.setPointSizeF(fnt.pointSizeF());
    }
    if (fontDiffFlags & KFontChooser::FontDiffFamily) {
        if (!isFixedOnly() || QFontInfo(fnt).fixedPitch()) {
            _font.setFamily(fnt.family());
        }
    }
    if (fontDiffFlags & KFontChooser::FontDiffStyle) {
        _font.setWeight(fnt.weight());
        _font.setStyle(fnt.style());
        _font.setUnderline(fnt.underline());
#if QT_VERSION >= 0x040800
        _font.setStyleName(fnt.styleName());
#endif
    }

    setFont(_font, isFixedOnly());
}

/**** FontAASettings ****/
#if defined(HAVE_FONTCONFIG) && defined (HAVE_X11)
FontAASettings::FontAASettings(QWidget *parent)
    : KDialog(parent),
      changesMade(false)
{
    setObjectName("FontAASettings");
    setModal(true);
    setCaption(i18n("Configure Anti-Alias Settings"));
    setButtons(Ok | Cancel);

    QWidget     *mw = new QWidget(this);
    QFormLayout *layout = new QFormLayout(mw);
    layout->setVerticalSpacing(0);
    layout->setMargin(0);

    excludeRange = new QCheckBox(i18n("E&xclude range:"), mw);
    QHBoxLayout *rangeLayout = new QHBoxLayout();
    excludeFrom = new KDoubleNumInput(0, 72, 8.0, mw, 1, 1);
    excludeFrom->setSuffix(i18n(" pt"));
    rangeLayout->addWidget(excludeFrom);
    excludeToLabel = new QLabel(i18n(" to "), mw);
    rangeLayout->addWidget(excludeToLabel);
    excludeTo = new KDoubleNumInput(0, 72, 15.0, mw, 1, 1);
    excludeTo->setSuffix(i18n(" pt"));
    rangeLayout->addWidget(excludeTo);
    layout->addRow(excludeRange, rangeLayout);

    QString subPixelWhatsThis = i18n("<p>If you have a TFT or LCD screen you"
                                     " can further improve the quality of displayed fonts by selecting"
                                     " this option.<br />Sub-pixel rendering is also known as ClearType(tm).<br />"
                                     " In order for sub-pixel rendering to"
                                     " work correctly you need to know how the sub-pixels of your display"
                                     " are aligned.</p>"
                                     " <p>On TFT or LCD displays a single pixel is actually composed of"
                                     " three sub-pixels, red, green and blue. Most displays"
                                     " have a linear ordering of RGB sub-pixel, some have BGR.<br />"
                                     " This feature does not work with CRT monitors.</p>");

    subPixelLabel = new QLabel(i18n("Sub-pixel rendering type: "), mw);
    subPixelLabel->setWhatsThis(subPixelWhatsThis);

    subPixelType = new QComboBox(mw);
    layout->addRow(subPixelLabel, subPixelType);

    subPixelType->setEditable(false);
    subPixelType->setWhatsThis(subPixelWhatsThis);

    for (int t = KXftConfig::SubPixel::NotSet; t <= KXftConfig::SubPixel::Vbgr; ++t) {
        subPixelType->addItem(QPixmap(aaPixmaps[t]), i18n(KXftConfig::description((KXftConfig::SubPixel::Type)t).toUtf8()));
    }

    QLabel *hintingLabel = new QLabel(i18n("Hinting style: "), mw);
    hintingStyle = new QComboBox(mw);
    hintingStyle->setEditable(false);
    layout->addRow(hintingLabel, hintingStyle);
    for (int s = KXftConfig::Hint::NotSet; s <= KXftConfig::Hint::Full; ++s) {
        hintingStyle->addItem(i18n(KXftConfig::description((KXftConfig::Hint::Style)s).toUtf8()));
    }

    QString hintingText(i18n("Hinting is a process used to enhance the quality of fonts at small sizes."));
    hintingStyle->setWhatsThis(hintingText);
    hintingLabel->setWhatsThis(hintingText);
    load();
    enableWidgets();
    setMainWidget(mw);

    connect(excludeRange, SIGNAL(toggled(bool)), SLOT(changed()));
    connect(excludeFrom, SIGNAL(valueChanged(double)), SLOT(changed()));
    connect(excludeTo, SIGNAL(valueChanged(double)), SLOT(changed()));
    connect(subPixelType, SIGNAL(activated(QString)), SLOT(changed()));
    connect(hintingStyle, SIGNAL(activated(QString)), SLOT(changed()));
}

bool FontAASettings::load()
{
    double     from, to;
    KXftConfig xft;

    if (xft.getExcludeRange(from, to)) {
        excludeRange->setChecked(true);
    } else {
        excludeRange->setChecked(false);
        from = 8.0;
        to = 15.0;
    }

    excludeFrom->setValue(from);
    excludeTo->setValue(to);

    KXftConfig::SubPixel::Type spType;

    xft.getSubPixelType(spType);
    int idx = getIndex(spType);

    subPixelType->setCurrentIndex(idx);

    KXftConfig::Hint::Style hStyle;

    if (!xft.getHintStyle(hStyle) || KXftConfig::Hint::NotSet == hStyle) {
        KConfig kglobals("kdeglobals", KConfig::NoGlobals);

        hStyle = KXftConfig::Hint::NotSet;
        xft.setHintStyle(hStyle);
        KConfigGroup(&kglobals, "General").writeEntry("XftHintStyle", KXftConfig::toStr(hStyle));
        kglobals.sync();
        runRdb(KRdbExportXftSettings | KRdbExportGtkTheme);
    }

    hintingStyle->setCurrentIndex(getIndex(hStyle));

    enableWidgets();

    return xft.aliasingEnabled();
}

bool FontAASettings::save(KXftConfig::AntiAliasing::State aaState)
{
    KXftConfig   xft;
    KConfig      kglobals("kdeglobals", KConfig::NoGlobals);
    KConfigGroup grp(&kglobals, "General");

    xft.setAntiAliasing(aaState);

    if (excludeRange->isChecked()) {
        xft.setExcludeRange(excludeFrom->value(), excludeTo->value());
    } else {
        xft.setExcludeRange(0, 0);
    }

    KXftConfig::SubPixel::Type spType(getSubPixelType());

    xft.setSubPixelType(spType);
    grp.writeEntry("XftSubPixel", KXftConfig::toStr(spType));
    if (KXftConfig::AntiAliasing::NotSet == aaState) {
        grp.revertToDefault("XftAntialias");
    } else {
        grp.writeEntry("XftAntialias", aaState == KXftConfig::AntiAliasing::Enabled);
    }

    bool mod = false;
    KXftConfig::Hint::Style hStyle(getHintStyle());

    xft.setHintStyle(hStyle);

    QString hs(KXftConfig::toStr(hStyle));
    if (hs != grp.readEntry("XftHintStyle")) {
        if (KXftConfig::Hint::NotSet == hStyle) {
            grp.revertToDefault("XftHintStyle");
        } else {
            grp.writeEntry("XftHintStyle", hs);
        }
    }
    mod = true;
    kglobals.sync();

    if (!mod) {
        mod = xft.changed();
    }

    xft.apply();

    return mod;
}

void FontAASettings::defaults()
{
    excludeRange->setChecked(false);
    excludeFrom->setValue(8.0);
    excludeTo->setValue(15.0);
    subPixelType->setCurrentIndex(getIndex(KXftConfig::SubPixel::NotSet));
    hintingStyle->setCurrentIndex(getIndex(KXftConfig::Hint::NotSet));
    enableWidgets();
}

int FontAASettings::getIndex(KXftConfig::SubPixel::Type spType)
{
    int pos = -1;
    int index;

    for (index = 0; index < subPixelType->count(); ++index)
        if (subPixelType->itemText(index) == i18n(KXftConfig::description(spType).toUtf8())) {
            pos = index;
            break;
        }

    return pos;
}

KXftConfig::SubPixel::Type FontAASettings::getSubPixelType()
{
    int t;

    for (t = KXftConfig::SubPixel::NotSet; t <= KXftConfig::SubPixel::Vbgr; ++t)
        if (subPixelType->currentText() == i18n(KXftConfig::description((KXftConfig::SubPixel::Type)t).toUtf8())) {
            return (KXftConfig::SubPixel::Type)t;
        }

    return KXftConfig::SubPixel::NotSet;
}

int FontAASettings::getIndex(KXftConfig::Hint::Style hStyle)
{
    int pos = -1;
    int index;

    for (index = 0; index < hintingStyle->count(); ++index)
        if (hintingStyle->itemText(index) == i18n(KXftConfig::description(hStyle).toUtf8())) {
            pos = index;
            break;
        }

    return pos;
}

KXftConfig::Hint::Style FontAASettings::getHintStyle()
{
    int s;

    for (s = KXftConfig::Hint::NotSet; s <= KXftConfig::Hint::Full; ++s)
        if (hintingStyle->currentText() == i18n(KXftConfig::description((KXftConfig::Hint::Style)s).toUtf8())) {
            return (KXftConfig::Hint::Style)s;
        }

    return KXftConfig::Hint::Medium;
}

void FontAASettings::enableWidgets()
{
    excludeFrom->setEnabled(excludeRange->isChecked());
    excludeTo->setEnabled(excludeRange->isChecked());
    excludeToLabel->setEnabled(excludeRange->isChecked());
#ifdef FT_LCD_FILTER_H
    static int ft_has_subpixel = -1;
    if (ft_has_subpixel == -1) {
        FT_Library            ftLibrary;
        if (FT_Init_FreeType(&ftLibrary) == 0) {
            ft_has_subpixel = (FT_Library_SetLcdFilter(ftLibrary, FT_LCD_FILTER_DEFAULT)
                               == FT_Err_Unimplemented_Feature) ? 0 : 1;
            FT_Done_FreeType(ftLibrary);
        }
    }
    subPixelType->setEnabled(ft_has_subpixel);
#endif
}
#endif

void FontAASettings::changed()
{
#if defined(HAVE_FONTCONFIG) && defined (HAVE_X11)
    changesMade = true;
    enableWidgets();
#endif
}

#if defined(HAVE_FONTCONFIG) && defined (HAVE_X11)
int FontAASettings::exec()
{
    int i = KDialog::exec();

    if (!i) {
        load();    // Reset settings...
    }

    return i && changesMade;
}
#endif

/**** KFonts ****/

KFonts::KFonts(QWidget *parent, const QVariantList &args)
    :   KCModule(parent, args)
{
    QStringList nameGroupKeyRc;

    nameGroupKeyRc
            << i18nc("font usage", "General")       << "General"    << "font"         << ""
            << i18nc("font usage", "Fixed width")   << "General"    << "fixed"        << ""
            << i18nc("font usage", "Small")         << "General"    << "smallestReadableFont" << ""
            << i18nc("font usage", "Toolbar")       << "General"    << "toolBarFont"  << ""
            << i18nc("font usage", "Menu")          << "General"    << "menuFont"     << ""
            << i18nc("font usage", "Window title")  << "WM"         << "activeFont"   << "";

    QList<QFont> defaultFontList;

    // NOTE: keep in sync with kdelibs/kdeui/kernel/kglobalsettings.cpp

#ifdef Q_WS_MAC
    QFont f0("Lucida Grande", 13); // general/menu/desktop
    QFont f1("Monaco", 10);
    QFont f2("Lucida Grande", 11); // toolbar
#elif defined(Q_WS_MAEMO_5) || defined(MEEGO_EDITION_HARMATTAN)
    QFont f0("Sans Serif", 16); // general/menu/desktop
    QFont f1("Monospace", 16;
    QFont f2("Sans Serif", 16); // toolbar
#else
    QFont f0("Oxygen-Sans", 10); // general/menu/desktop
    QFont f1("Oxygen Mono", 9); // fixed font
    QFont f2("Oxygen-Sans", 9); // toolbar
#endif
    QFont f3("Oxygen-Sans", 10); // window title
    QFont f5("Oxygen-Sans", 8); // smallestReadableFont

    defaultFontList << f0 << f1 << f5 << f2 << f0 << f3 << f0;

    QList<bool> fixedList;

    fixedList
            <<  false
            <<  true
            <<  false
            <<  false
            <<  false
            <<  false;

    QStringList quickHelpList;

    quickHelpList
            << i18n("Used for normal text (e.g. button labels, list items).")
            << i18n("A non-proportional font (i.e. typewriter font).")
            << i18n("Smallest font that is still readable well.")
            << i18n("Used to display text beside toolbar icons.")
            << i18n("Used by menu bars and popup menus.")
            << i18n("Used by the window titlebar.");

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setMargin(0);

    QGridLayout *fontUseLayout = new QGridLayout();
    layout->addLayout(fontUseLayout);
    fontUseLayout->setColumnStretch(0, 0);
    fontUseLayout->setColumnStretch(1, 1);
    fontUseLayout->setColumnStretch(2, 0);

    QList<QFont>::ConstIterator defaultFontIt(defaultFontList.begin());
    QList<bool>::ConstIterator fixedListIt(fixedList.begin());
    QStringList::ConstIterator quickHelpIt(quickHelpList.begin());
    QStringList::ConstIterator it(nameGroupKeyRc.begin());

    unsigned int count = 0;

    while (it != nameGroupKeyRc.constEnd()) {

        QString name = *it; it++;
        QString group = *it; it++;
        QString key = *it; it++;
        QString file = *it; it++;

        FontUseItem *i =
            new FontUseItem(
            this,
            name,
            group,
            key,
            file,
            *defaultFontIt++,
            *fixedListIt++
        );

        fontUseList.append(i);
        connect(i, SIGNAL(fontSelected(QFont)), SLOT(fontSelected()));

        QLabel *fontUse = new QLabel(i18nc("Font role", "%1: ", name), this);
        fontUse->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        fontUse->setWhatsThis(*quickHelpIt++);

        fontUseLayout->addWidget(fontUse, count, 0);
        fontUseLayout->addWidget(i, count, 1);

        ++count;
    }

    QHBoxLayout *hblay = new QHBoxLayout();
    layout->addLayout(hblay);
    hblay->addStretch();
    QPushButton *fontAdjustButton = new QPushButton(i18n("Ad&just All Fonts..."), this);
    fontAdjustButton->setWhatsThis(i18n("Click to change all fonts"));
    hblay->addWidget(fontAdjustButton);
    connect(fontAdjustButton, SIGNAL(clicked()), SLOT(slotApplyFontDiff()));

    layout->addSpacing(KDialog::spacingHint());

    QGridLayout *lay = new QGridLayout();
    layout->addLayout(lay);
    lay->setColumnStretch(3, 10);
#if defined(HAVE_FONTCONFIG) && defined (HAVE_X11)
    QLabel *label = 0L;
    label = new QLabel(i18n("Use a&nti-aliasing:"), this);
    label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    lay->addWidget(label, 0, 0);
    cbAA = new QComboBox(this);
    cbAA->insertItem(AAEnabled, i18nc("Use anti-aliasing", "Enabled"));    // change AASetting type if order changes
    cbAA->insertItem(AASystem, i18nc("Use anti-aliasing", "System Settings"));
    cbAA->insertItem(AADisabled, i18nc("Use anti-aliasing", "Disabled"));
    cbAA->setWhatsThis(i18n("If this option is selected, KDE will smooth the edges of curves in fonts."));
    aaSettingsButton = new QPushButton(i18n("Configure..."), this);
    connect(aaSettingsButton, SIGNAL(clicked()), SLOT(slotCfgAa()));
    label->setBuddy(cbAA);
    lay->addWidget(cbAA, 0, 1);
    lay->addWidget(aaSettingsButton, 0, 2);
    // Initialize aaSettingsButton state based on the current cbAA->currentIndex value, will be eventually updated at load()
    slotUseAntiAliasing();

    connect(cbAA, SIGNAL(currentIndexChanged(int)), SLOT(slotUseAntiAliasing()));
#endif
#if HAVE_X11
    checkboxForceDpi = new QCheckBox(i18n("Force fonts DPI:"), this);
    lay->addWidget(checkboxForceDpi, 1, 0);
    spinboxDpi = new QSpinBox(this);
    spinboxDpi->setRange(1, 1000);
    spinboxDpi->setSingleStep(24); // The common DPI values 72, 96 and 120 are multiples of 24
    QString whatsthis = i18n(
                            "<p>This option forces a specific DPI value for fonts. It may be useful"
                            " when the real DPI of the hardware is not detected properly and it"
                            " is also often misused when poor quality fonts are used that do not"
                            " look well with DPI values other than 96 or 120 DPI.</p>"
                            "<p>The use of this option is generally discouraged. For selecting proper DPI"
                            " value a better option is explicitly configuring it for the whole X server if"
                            " possible (e.g. DisplaySize in xorg.conf or adding <i>-dpi value</i> to"
                            " ServerLocalArgs= in $KDEDIR/share/config/kdm/kdmrc). When fonts do not render"
                            " properly with real DPI value better fonts should be used or configuration"
                            " of font hinting should be checked.</p>");
    spinboxDpi->setWhatsThis(whatsthis);
    checkboxForceDpi->setChecked(false);
    spinboxDpi->setEnabled(false);
    connect(spinboxDpi, SIGNAL(valueChanged(int)), SLOT(changed()));
    connect(checkboxForceDpi, SIGNAL(toggled(bool)), SLOT(changed()));
    connect(checkboxForceDpi, SIGNAL(toggled(bool)), spinboxDpi, SLOT(setEnabled(bool)));
    lay->addWidget(spinboxDpi, 1, 1);
#endif
    layout->addStretch(1);

#if defined(HAVE_FONTCONFIG) && defined (HAVE_X11)
    aaSettings = new FontAASettings(this);
#endif

}

KFonts::~KFonts()
{
    QList<FontUseItem *>::Iterator it(fontUseList.begin()),
          end(fontUseList.end());

    for (; it != end; ++it) {
        delete(*it);
    }
    fontUseList.clear();
}

void KFonts::fontSelected()
{
    emit changed(true);
}

void KFonts::defaults()
{
    for (int i = 0; i < (int) fontUseList.count(); i++) {
        fontUseList.at(i)->setDefault();
    }

#if defined(HAVE_FONTCONFIG) && defined (HAVE_X11)
    useAA = AASystem;
    cbAA->setCurrentIndex(useAA);
    aaSettings->defaults();
#endif
#if HAVE_X11
    checkboxForceDpi->setChecked(false);
    spinboxDpi->setValue(96);
#endif
    emit changed(true);
}

void KFonts::load()
{
    QList<FontUseItem *>::Iterator it(fontUseList.begin()),
          end(fontUseList.end());

    for (; it != end; ++it) {
        (*it)->readFont();
    }

#if defined(HAVE_FONTCONFIG) && defined (HAVE_X11)
    useAA_original = useAA = aaSettings->load() ? AAEnabled : AADisabled;
    cbAA->setCurrentIndex(useAA);
#endif

    KConfig _cfgfonts("kcmfonts");
    KConfigGroup cfgfonts(&_cfgfonts, "General");
#if HAVE_X11
    int dpicfg = cfgfonts.readEntry("forceFontDPI", 0);
    if (dpicfg <= 0) {
        checkboxForceDpi->setChecked(false);
        spinboxDpi->setValue(96);
        dpi_original = 0;
    } else {
        checkboxForceDpi->setChecked(true);
        spinboxDpi->setValue(dpicfg);
        dpi_original = dpicfg;
    };
#endif
#if defined(HAVE_FONTCONFIG) && defined (HAVE_X11)
    if (cfgfonts.readEntry("dontChangeAASettings", true)) {
        useAA_original = useAA = AASystem;
        cbAA->setCurrentIndex(useAA);
    }
#endif

    emit changed(false);
}

void KFonts::save()
{
    QList<FontUseItem *>::Iterator it(fontUseList.begin()),
          end(fontUseList.end());

    for (; it != end; ++it) {
        (*it)->writeFont();
    }

    KSharedConfig::openConfig()->sync();

    KConfig _cfgfonts("kcmfonts");
    KConfigGroup cfgfonts(&_cfgfonts, "General");
#if HAVE_X11
    int dpi = (checkboxForceDpi->isChecked() ? spinboxDpi->value() : 0);
    cfgfonts.writeEntry("forceFontDPI", dpi);
#endif
#if defined(HAVE_FONTCONFIG) && defined (HAVE_X11)
    cfgfonts.writeEntry("dontChangeAASettings", cbAA->currentIndex() == AASystem);
#endif
    cfgfonts.sync();
#if HAVE_X11
    // if the setting is reset in the module, remove the dpi value,
    // otherwise don't explicitly remove it and leave any possible system-wide value
    if (dpi == 0 && dpi_original != 0) {
        KProcess proc;
        proc << "xrdb" << "-quiet" << "-remove" << "-nocpp";
        proc.start();
        if (proc.waitForStarted()) {
            proc.write(QByteArray("Xft.dpi\n"));
            proc.closeWriteChannel();
            proc.waitForFinished();
        }
    }
#endif

    KGlobalSettings::self()->emitChange(KGlobalSettings::FontChanged);

    kapp->processEvents(); // Process font change ourselves

    // Don't overwrite global settings unless explicitly asked for - e.g. the system
    // fontconfig setup may be much more complex than this module can provide.
    // TODO: With AASystem the changes already made by this module should be reverted somehow.
#if defined(HAVE_FONTCONFIG) && defined (HAVE_X11)
    bool aaSave = false;
    if (cbAA->currentIndex() == AAEnabled ) {
        aaSave = aaSettings->save(KXftConfig::AntiAliasing::Enabled);
    } else if (cbAA->currentIndex() == AADisabled) {
        aaSave = aaSettings->save(KXftConfig::AntiAliasing::Disabled);
    } else {
        // If AASystem is selected, this removes all fontconfig settings made by
        // this module.
        aaSettings->defaults();
        aaSave = aaSettings->save(KXftConfig::AntiAliasing::NotSet);
    }

    if (aaSave || (useAA != useAA_original) || dpi != dpi_original) {
        KMessageBox::information(this,
                                 i18n(
                                     "<p>Some changes such as anti-aliasing or DPI will only affect newly started applications.</p>"
                                 ), i18n("Font Settings Changed"), "FontSettingsChanged");
        useAA_original = useAA;
        dpi_original = dpi;
    }
#else
#if HAVE_X11
    if (dpi != dpi_original) {
        KMessageBox::information(this,
                                 i18n(
                                     "<p>Some changes such as DPI will only affect newly started applications.</p>"
                                 ), i18n("Font Settings Changed"), "FontSettingsChanged");
        dpi_original = dpi;
    }
#endif
#endif
    runRdb(KRdbExportXftSettings | KRdbExportGtkTheme);

    emit changed(false);
}

void KFonts::slotApplyFontDiff()
{
    QFont font = QFont(fontUseList.first()->font());
    KFontChooser::FontDiffFlags fontDiffFlags = 0;
    int ret = KFontDialog::getFontDiff(font, fontDiffFlags, KFontChooser::NoDisplayFlags, this);

    if (ret == KDialog::Accepted && fontDiffFlags) {
        for (int i = 0; i < (int) fontUseList.count(); i++) {
            fontUseList.at(i)->applyFontDiff(font, fontDiffFlags);
        }
        emit changed(true);
    }
}

void KFonts::slotUseAntiAliasing()
{
#if defined(HAVE_FONTCONFIG) && defined (HAVE_X11)
    useAA = static_cast< AASetting >(cbAA->currentIndex());
    aaSettingsButton->setEnabled(useAA == AAEnabled);
    emit changed(true);
#endif
}

void KFonts::slotCfgAa()
{
#if defined(HAVE_FONTCONFIG) && defined (HAVE_X11)
    if (aaSettings->exec()) {
        emit changed(true);
    }
#endif
}

#include "fonts.moc"

