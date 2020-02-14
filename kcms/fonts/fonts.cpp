/*
    Copyright 1997 Mark Donohoe
    Copyright 1999 Lars Knoll
    Copyright 2000 Rik Hemsley
    Copyright 2015 Antonis Tsiapaliokas <antonis.tsiapaliokas@kde.org>
    Copyright 2017 Marco Martin <mart@kde.org>
    Copyright 2019 Benjamin Port <benjamin.port@enioka.com>

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


#include "fonts.h"

#include <QQuickItem>
#include <QWindow>
#include <QQmlEngine>
#include <QQuickView>
#include <QFontDialog>
#include <QApplication>
#include <QFontDatabase>

#include <KAcceleratorManager>
#include <KGlobalSettings>
#include <KConfigGroup>
#include <KConfig>
#include <KAboutData>
#include <KLocalizedString>
#include <KPluginFactory>
#include <KFontDialog>
#include <KWindowSystem>

#include "../krdb/krdb.h"
#include "previewimageprovider.h"

#include "fontssettings.h"

/**** DLL Interface ****/
K_PLUGIN_FACTORY_WITH_JSON(KFontsFactory, "kcm_fonts.json", registerPlugin<KFonts>();)

//from KFontRequester
// Determine if the font with given properties is available on the system,
// otherwise find and return the best fitting combination.
static QFont nearestExistingFont(const QFont &font)
{
    QFontDatabase dbase;

    // Initialize font data according to given font object.
    QString family = font.family();
    QString style = dbase.styleString(font);
    qreal size = font.pointSizeF();

    // Check if the family exists.
    const QStringList families = dbase.families();
    if (!families.contains(family)) {
        // Chose another family.
        family = QFontInfo(font).family(); // the nearest match
        if (!families.contains(family)) {
            family = families.count() ? families.at(0) : QStringLiteral("fixed");
        }
    }

    // Check if the family has the requested style.
    // Easiest by piping it through font selection in the database.
    QString retStyle = dbase.styleString(dbase.font(family, style, 10));
    style = retStyle;

    // Check if the family has the requested size.
    // Only for bitmap fonts.
    if (!dbase.isSmoothlyScalable(family, style)) {
        QList<int> sizes = dbase.smoothSizes(family, style);
        if (!sizes.contains(size)) {
            // Find nearest available size.
            int mindiff = 1000;
            int refsize = size;
            Q_FOREACH (int lsize, sizes) {
                int diff = qAbs(refsize - lsize);
                if (mindiff > diff) {
                    mindiff = diff;
                    size = lsize;
                }
            }
        }
    }

    // Select the font with confirmed properties.
    QFont result = dbase.font(family, style, int(size));
    if (dbase.isSmoothlyScalable(family, style) && result.pointSize() == floor(size)) {
        result.setPointSizeF(size);
    }
    return result;
}

/**** FontAASettings ****/
#if defined(HAVE_FONTCONFIG) && HAVE_X11
FontAASettings::FontAASettings(QObject *parent)
    : QObject(parent)
    , m_state()
    , m_originalState()
    , m_subPixelOptionsModel(new QStandardItemModel(this))
    , m_hintingOptionsModel(new QStandardItemModel(this))
{
    m_state.subPixel = KXftConfig::SubPixel::None;
    for (int t = KXftConfig::SubPixel::None; t <= KXftConfig::SubPixel::Vbgr; ++t) {
        QStandardItem *item = new QStandardItem(KXftConfig::description((KXftConfig::SubPixel::Type)t));
        m_subPixelOptionsModel->appendRow(item);
    }

    m_state.hinting = KXftConfig::Hint::None;
    for (int s = KXftConfig::Hint::None; s <= KXftConfig::Hint::Full; ++s) {
        QStandardItem * item = new QStandardItem(KXftConfig::description((KXftConfig::Hint::Style)s));
        m_hintingOptionsModel->appendRow(item);
    }
}

void FontAASettings::load()
{
    double     from, to;
    KXftConfig xft;

    if (xft.getExcludeRange(from, to)) {
        m_state.excludeFrom = from;
        m_state.excludeTo = to;
        setExclude(true);
    } else {
        m_state.excludeFrom = 8;
        m_state.excludeTo = 15;
        setExclude(false);
    }
    m_originalState.exclude = m_state.exclude;
    m_originalState.excludeFrom = m_state.excludeFrom;
    m_originalState.excludeTo = m_state.excludeTo;
    excludeToChanged();
    excludeFromChanged();

    // start with empty subpixel type
    KXftConfig::SubPixel::Type spType = KXftConfig::SubPixel::NotSet;
    // get subpixel type from config
    xft.getSubPixelType(spType);
    m_originalState.subPixel = spType;
    // if it is not set, we set it to rgb
    if (spType == KXftConfig::SubPixel::NotSet) {
        spType = KXftConfig::SubPixel::Rgb;
    }
    setSubPixel(spType);
    m_state.subPixelHasLocalConfig = xft.subPixelTypeHasLocalConfig();

    // start with empty hint style
    KXftConfig::Hint::Style hStyle = KXftConfig::Hint::NotSet;
    // get value from config;
    xft.getHintStyle(hStyle);
    m_originalState.hinting = hStyle;
    // if it is not set, we set it to slight hinting
    if (hStyle == KXftConfig::Hint::NotSet) {
        hStyle = KXftConfig::Hint::Slight;
    }
    setHinting(hStyle);
    m_state.hintingHasLocalConfig = xft.hintStyleHasLocalConfig();

    KConfig _cfgfonts("kcmfonts");
    KConfigGroup cfgfonts(&_cfgfonts, "General");

    int dpicfg;
    if (KWindowSystem::isPlatformWayland()) {
        dpicfg = cfgfonts.readEntry("forceFontDPIWayland", 0);
    } else {
        dpicfg = cfgfonts.readEntry("forceFontDPI", 0);
    }

    if (dpicfg <= 0) {
        m_originalState.dpi = 0;
    } else {
        m_originalState.dpi = dpicfg;
    };

    setDpi(dpicfg);

    KSharedConfig::Ptr config = KSharedConfig::openConfig("kdeglobals");
    KConfigGroup cg(config, "General");

    if (cfgfonts.readEntry("dontChangeAASettings", true)) {
        setAntiAliasing(true); //AASystem
        m_state.antiAliasingHasLocalConfig = false;
    } else if (cg.readEntry("XftAntialias", true)) {
        setAntiAliasing(true); //AAEnabled
        m_state.antiAliasingHasLocalConfig = xft.antiAliasingHasLocalConfig();
    } else {
        setAntiAliasing(false); //AADisabled
        m_state.antiAliasingHasLocalConfig = xft.antiAliasingHasLocalConfig();
    }
    m_originalState.antiAliasing = m_state.antiAliasing;
    m_originalState.antiAliasingHasLocalConfig = m_state.antiAliasingHasLocalConfig;
}

bool FontAASettings::save(KXftConfig::AntiAliasing::State aaState)
{
    KXftConfig   xft;

    KSharedConfig::Ptr config = KSharedConfig::openConfig("kdeglobals");
    KConfigGroup grp(config, "General");

    xft.setAntiAliasing(aaState);
    if (m_state.exclude) {
        xft.setExcludeRange(m_state.excludeFrom, m_state.excludeTo);
    } else {
        xft.setExcludeRange(0, 0);
    }

    KXftConfig::SubPixel::Type spType = (KXftConfig::SubPixel::Type)m_state.subPixel;

    if (subPixelNeedsSave()) {
        xft.setSubPixelType(spType);
    } else {
        xft.setSubPixelType(KXftConfig::SubPixel::NotSet);
    }
    grp.writeEntry("XftSubPixel", KXftConfig::toStr(spType));
    if (aaState == KXftConfig::AntiAliasing::NotSet) {
        grp.revertToDefault("XftAntialias");
    } else {
        grp.writeEntry("XftAntialias", aaState == KXftConfig::AntiAliasing::Enabled);
    }

    bool mod = false;
    KXftConfig::Hint::Style hStyle = (KXftConfig::Hint::Style)m_state.hinting;

    if (hintingNeedsSave()) {
        xft.setHintStyle(hStyle);
    } else {
        xft.setHintStyle(KXftConfig::Hint::NotSet);
    }

    QString hs(KXftConfig::toStr(hStyle));

    if (hs != grp.readEntry("XftHintStyle")) {
        if (KXftConfig::Hint::NotSet == hStyle) {
            grp.revertToDefault("XftHintStyle");
        } else {
            grp.writeEntry("XftHintStyle", hs);
        }
    }
    mod = true;
    config->sync();

    if (!mod) {
        mod = xft.changed();
    }

    xft.apply();

    KConfig _cfgfonts("kcmfonts");
    KConfigGroup cfgfonts(&_cfgfonts, "General");

    if (KWindowSystem::isPlatformWayland()) {
        cfgfonts.writeEntry("forceFontDPIWayland", m_state.dpi);
    } else {
        cfgfonts.writeEntry("forceFontDPI", m_state.dpi);
    }

    cfgfonts.sync();

#if HAVE_X11
    // if the setting is reset in the module, remove the dpi value,
    // otherwise don't explicitly remove it and leave any possible system-wide value
    if (m_state.dpi == 0 && m_originalState.dpi != 0 && !KWindowSystem::isPlatformWayland()) {
        QProcess proc;
        proc.setProcessChannelMode(QProcess::ForwardedChannels);
        proc.start("xrdb", QStringList() << "-quiet" << "-remove" << "-nocpp");
        if (proc.waitForStarted()) {
            proc.write(QByteArray("Xft.dpi\n"));
            proc.closeWriteChannel();
            proc.waitForFinished();
        }
    }
#endif

    QApplication::processEvents();			// Process font change ourselves

    // Don't overwrite global settings unless explicitly asked for - e.g. the system
    // fontconfig setup may be much more complex than this module can provide.
    // TODO: With AASystem the changes already made by this module should be reverted somehow.
#if defined(HAVE_FONTCONFIG) && defined (HAVE_X11)
    if (mod || (m_state.antiAliasing != m_originalState.antiAliasing) || m_state.dpi != m_originalState.dpi) {
        m_originalState.antiAliasing = m_state.antiAliasing;
        m_originalState.dpi = m_state.dpi;
        emit aliasingChangeApplied();
    }
#else
#if HAVE_X11
    if (m_state.dpi != m_originalState.dpi) {
        m_originalState.dpi = m_state.dpi;
    }
#endif
#endif

    m_originalState.exclude = m_state.exclude;
    m_originalState.excludeTo = m_state.excludeTo;
    m_originalState.excludeFrom = m_state.excludeFrom;

    m_originalState.subPixel = m_state.subPixel;
    m_originalState.hinting = m_state.hinting;

    return mod;
}

void FontAASettings::defaults()
{
    setExclude(false);
    setExcludeTo(15);
    setExcludeFrom(8);
    setAntiAliasing(true);
    m_originalState.antiAliasing = m_state.antiAliasing;
    m_state.antiAliasingHasLocalConfig = false;
    setDpi(0);
    setSubPixel(KXftConfig::SubPixel::Rgb);
    m_state.subPixelHasLocalConfig = false;
    setHinting(KXftConfig::Hint::Slight);
    m_state.hintingHasLocalConfig = false;
}

#endif

void FontAASettings::setExclude(bool exclude)
{
    if (exclude == m_state.exclude) {
        return;
    }

    m_state.exclude = exclude;
    emit excludeChanged();
}

bool FontAASettings::exclude() const
{
    return m_state.exclude;
}

void FontAASettings::setExcludeTo(int excludeTo)
{
    if (m_state.excludeTo == excludeTo) {
        return;
    }

    m_state.excludeTo = excludeTo;
    emit excludeToChanged();
}

int FontAASettings::excludeTo() const
{
    return m_state.excludeTo;
}

void FontAASettings::setExcludeFrom(int excludeFrom)
{
    if (m_state.excludeFrom == excludeFrom) {
        return;
    }

    m_state.excludeFrom = excludeFrom;
    emit excludeFromChanged();
}

int FontAASettings::excludeFrom() const
{
    return m_state.excludeFrom;
}

void FontAASettings::setAntiAliasing(bool antiAliasing)
{
    if (m_state.antiAliasing == antiAliasing) {
        return;
    }

    m_state.antiAliasing = antiAliasing;
    emit aliasingChanged();
}

bool FontAASettings::antiAliasing() const
{
    return m_state.antiAliasing;
}

bool FontAASettings::antiAliasingNeedsSave() const
{
    return m_state.antiAliasingHasLocalConfig || (m_state.antiAliasing != m_originalState.antiAliasing);
}

bool FontAASettings::subPixelNeedsSave() const
{
    return m_state.subPixelHasLocalConfig || (m_state.subPixel != m_originalState.subPixel);
}

bool FontAASettings::hintingNeedsSave() const
{
    return m_state.hintingHasLocalConfig || (m_state.hinting != m_originalState.hinting);
}

void FontAASettings::setDpi(const int &dpi)
{
    if (m_state.dpi == dpi) {
        return;
    }

    m_state.dpi = dpi;
    emit dpiChanged();
}

int FontAASettings::dpi() const
{
    return m_state.dpi;
}

void FontAASettings::setSubPixel(int idx)
{
    if (m_state.subPixel == idx) {
        return;
    }

    m_state.subPixel = idx;
    emit subPixelCurrentIndexChanged();
}

void FontAASettings::setSubPixelCurrentIndex(int idx)
{
    setSubPixel(KXftConfig::SubPixel::None + idx);
}

int FontAASettings::subPixelCurrentIndex()
{
    return m_state.subPixel - KXftConfig::SubPixel::None;
}

void FontAASettings::setHinting(int idx)
{
    if (m_state.hinting == idx) {
        return;
    }

    m_state.hinting = idx;
    emit hintingCurrentIndexChanged();
}

void FontAASettings::setHintingCurrentIndex(int idx)
{
    setHinting(KXftConfig::Hint::None + idx);
}

int FontAASettings::hintingCurrentIndex()
{
    return m_state.hinting - KXftConfig::Hint::None;
}

bool FontAASettings::needsSave() const
{
    return m_state != m_originalState;
}

bool FontAASettings::isDefaults() const
{
    State defaultState{};
    defaultState.exclude = false;
    defaultState.excludeTo = 15;
    defaultState.excludeFrom = 8;
    defaultState.antiAliasing = true;
    defaultState.dpi = 0;
    defaultState.subPixel = KXftConfig::SubPixel::Rgb;
    defaultState.hinting = KXftConfig::Hint::Slight;
    return m_state == defaultState;
}

bool FontAASettings::State::operator==(const State& other) const
{
    if (
        exclude != other.exclude
        || antiAliasing != other.antiAliasing
        || dpi != other.dpi
        || subPixel != other.subPixel
        || hinting != other.hinting
    ) {
        return false;
    }

    if (exclude && (excludeFrom != other.excludeFrom || excludeTo != other.excludeTo)) {
        return false;
    }

    return true;
}

bool FontAASettings::State::operator!=(const State& other) const
{
    return !(*this == other);
}

/**** KFonts ****/

KFonts::KFonts(QObject *parent, const QVariantList &args)
    : KQuickAddons::ManagedConfigModule(parent, args)
    , m_settings(new FontsSettings(this))
    , m_fontAASettings(new FontAASettings(this))
{
    KAboutData* about = new KAboutData("kcm_fonts", i18n("Fonts"),
                                       "0.1", QString(), KAboutLicense::LGPL);
    about->addAuthor(i18n("Antonis Tsiapaliokas"), QString(), "antonis.tsiapaliokas@kde.org");
    setAboutData(about);
    qmlRegisterType<QStandardItemModel>();
    qmlRegisterType<FontsSettings>();

    setButtons(Apply | Default | Help);

    connect(m_fontAASettings, &FontAASettings::subPixelCurrentIndexChanged, this, &KFonts::settingsChanged);
    connect(m_fontAASettings, &FontAASettings::hintingCurrentIndexChanged, this, &KFonts::settingsChanged);
    connect(m_fontAASettings, &FontAASettings::excludeChanged, this, &KFonts::settingsChanged);
    connect(m_fontAASettings, &FontAASettings::excludeFromChanged, this, &KFonts::settingsChanged);
    connect(m_fontAASettings, &FontAASettings::excludeToChanged, this, &KFonts::settingsChanged);
    connect(m_fontAASettings, &FontAASettings::antiAliasingChanged, this, &KFonts::settingsChanged);
    connect(m_fontAASettings, &FontAASettings::aliasingChanged, this, &KFonts::settingsChanged);
    connect(m_fontAASettings, &FontAASettings::dpiChanged, this, &KFonts::settingsChanged);
}

KFonts::~KFonts()
{
}

FontsSettings *KFonts::fontsSettings() const
{
    return m_settings;
}

void KFonts::defaults()
{
    ManagedConfigModule::defaults();
    m_fontAASettings->defaults();
}

void KFonts::setNearestExistingFonts()
{
    m_settings->setFont(nearestExistingFont(m_settings->font()));
    m_settings->setFixed(nearestExistingFont(m_settings->fixed()));
    m_settings->setSmallestReadableFont(nearestExistingFont(m_settings->smallestReadableFont()));
    m_settings->setToolBarFont(nearestExistingFont(m_settings->toolBarFont()));
    m_settings->setMenuFont(nearestExistingFont(m_settings->menuFont()));
    m_settings->setActiveFont(nearestExistingFont(m_settings->activeFont()));
}

void KFonts::load()
{
    // first load all the settings
    ManagedConfigModule::load();
    m_fontAASettings->load();

    // Then set the existing fonts based on those settings
    setNearestExistingFonts();

    // Load preview
    // NOTE: This needs to be done AFTER AA settings is loaded
    // otherwise AA settings will be resetted in process of loading
    // previews
    engine()->addImageProvider("preview", new PreviewImageProvider(m_settings->font()));

    // KCM expect save state to be false at this point (can be true because of setNearestExistingFonts
    setNeedsSave(false);
}

void KFonts::save()
{
    ManagedConfigModule::save();

    KConfig _cfgfonts("kcmfonts");
    KConfigGroup cfgfonts(&_cfgfonts, "General");

    FontAASettings::AASetting aaSetting = FontAASettings::AASystem;
    if (m_fontAASettings->antiAliasingNeedsSave()) {
        aaSetting = m_fontAASettings->antiAliasing() ? FontAASettings::AAEnabled : FontAASettings::AADisabled;
    }
    cfgfonts.writeEntry("dontChangeAASettings", aaSetting == FontAASettings::AASystem);

    if (aaSetting == FontAASettings::AAEnabled) {
        m_fontAASettings->save(KXftConfig::AntiAliasing::Enabled);
    } else if (aaSetting == FontAASettings::AADisabled) {
        m_fontAASettings->save(KXftConfig::AntiAliasing::Disabled);
    } else {
        m_fontAASettings->save(KXftConfig::AntiAliasing::NotSet);
    }

    KGlobalSettings::self()->emitChange(KGlobalSettings::FontChanged);

    runRdb(KRdbExportXftSettings | KRdbExportGtkTheme);

    emit fontsHaveChanged();
}

bool KFonts::isSaveNeeded() const
{
    return m_fontAASettings->needsSave();
}

bool KFonts::isDefaults() const
{
    return m_fontAASettings->isDefaults();
}

void KFonts::adjustAllFonts()
{
    QFont font = m_settings->font();
    KFontChooser::FontDiffFlags fontDiffFlags;
    int ret = KFontDialog::getFontDiff(font, fontDiffFlags, KFontChooser::NoDisplayFlags);

    if (ret == KDialog::Accepted && fontDiffFlags) {
        if (!m_settings->isImmutable("font")) {
            m_settings->setFont(applyFontDiff(m_settings->font(), font, fontDiffFlags));
        }
        if (!m_settings->isImmutable("menuFont")) {
            m_settings->setMenuFont(applyFontDiff(m_settings->menuFont(), font, fontDiffFlags));
        }
        if (!m_settings->isImmutable("toolBarFont")) {
            m_settings->setToolBarFont(applyFontDiff(m_settings->toolBarFont(), font, fontDiffFlags));
        }
        if (!m_settings->isImmutable("activeFont")) {
            m_settings->setActiveFont(applyFontDiff(m_settings->activeFont(), font, fontDiffFlags));
        }
        if (!m_settings->isImmutable("smallestReadableFont")) {
            m_settings->setSmallestReadableFont(applyFontDiff(m_settings->smallestReadableFont(), font, fontDiffFlags));
        }
        const QFont adjustedFont = applyFontDiff(m_settings->fixed(), font, fontDiffFlags);
        if (QFontInfo(adjustedFont).fixedPitch() && !m_settings->isImmutable("fixed")) {
            m_settings->setFixed(adjustedFont);
        }
    }
}

QFont KFonts::applyFontDiff(const QFont &fnt, const QFont &newFont, int fontDiffFlags)
{
    QFont font(fnt);

    if (fontDiffFlags & KFontChooser::FontDiffSize) {
        font.setPointSizeF(newFont.pointSizeF());
    }
    if ((fontDiffFlags & KFontChooser::FontDiffFamily)) {
        font.setFamily(newFont.family());
    }
    if (fontDiffFlags & KFontChooser::FontDiffStyle) {
        font.setWeight(newFont.weight());
        font.setStyle(newFont.style());
        font.setUnderline(newFont.underline());
        font.setStyleName(newFont.styleName());
    }

    return font;
}

#include "fonts.moc"

