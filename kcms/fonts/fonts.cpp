/*
    Copyright 1997 Mark Donohoe
    Copyright 1999 Lars Knoll
    Copyright 2000 Rik Hemsley
    Copyright 2015 Antonis Tsiapaliokas <antonis.tsiapaliokas@kde.org>
    Copyright 2017 Marco Martin <mart@kde.org>

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

#include <KAcceleratorManager>
#include <KApplication>
#include <KGlobalSettings>
#include <KConfigGroup>
#include <KConfig>
#include <KLocalizedString>
#include <KPluginFactory>
#include <QDebug>

#include "../krdb/krdb.h"

/**** DLL Interface ****/
K_PLUGIN_FACTORY_WITH_JSON(KFontsFactory, "kcm_fonts.json", registerPlugin<KFonts>();)

/**** FontAASettings ****/
#if defined(HAVE_FONTCONFIG) && defined (HAVE_X11)
FontAASettings::FontAASettings(QObject *parent)
    : QObject(parent)
    , m_subPixelOptionsModel(new QStandardItemModel(this))
    , m_hintingOptionsModel(new QStandardItemModel(this))
    , m_subPixel(QString("System default"))
    , m_hinting(QString("System default"))
{
    for (int t = KXftConfig::SubPixel::NotSet; t <= KXftConfig::SubPixel::Vbgr; ++t) {
        QStandardItem *item = new QStandardItem(i18n(KXftConfig::description((KXftConfig::SubPixel::Type)t).toUtf8()));
        m_subPixelOptionsModel->appendRow(item);
    }

    for (int s = KXftConfig::Hint::NotSet; s <= KXftConfig::Hint::Full; ++s) {
        QStandardItem * item = new QStandardItem(i18n(KXftConfig::description((KXftConfig::Hint::Style)s).toUtf8()));
        m_hintingOptionsModel->appendRow(item);
    }
}

void FontAASettings::load()
{
    double     from, to;
    KXftConfig xft;

    if (xft.getExcludeRange(from, to)) {
        m_excludeFrom = from;
        m_excludeTo = to;
    } else {
        m_excludeFrom = 8;
        m_excludeTo = 15;
    }

    KXftConfig::SubPixel::Type spType;

    xft.getSubPixelType(spType);
    m_subPixel = KXftConfig::description(spType);

    KXftConfig::Hint::Style hStyle;

    if (!xft.getHintStyle(hStyle) || KXftConfig::Hint::NotSet == hStyle) {
        KConfig kglobals("kdeglobals", KConfig::NoGlobals);

        hStyle = KXftConfig::Hint::NotSet;
        xft.setHintStyle(hStyle);
        KConfigGroup(&kglobals, "General").writeEntry("XftHintStyle", KXftConfig::toStr(hStyle));
        kglobals.sync();
        runRdb(KRdbExportXftSettings | KRdbExportGtkTheme);
    } else {
        m_hinting = KXftConfig::description(hStyle);
    }

    KConfig _cfgfonts("kcmfonts");
    KConfigGroup cfgfonts(&_cfgfonts, "General");
    setDpi(cfgfonts.readEntry("forceFontDPI", 0));

    if (cfgfonts.readEntry("dontChangeAASettings", true)) {
        setAntiAliasing(1); //AASystem
    } else if (xft.aliasingEnabled()) {
        setAntiAliasing(0); //AAEnabled
    } else {
        setAntiAliasing(2); //AADisabled
    }
}

bool FontAASettings::save(KXftConfig::AntiAliasing::State aaState)
{
    KXftConfig   xft;
    KConfig      kglobals("kdeglobals", KConfig::NoGlobals);
    KConfigGroup grp(&kglobals, "General");

    xft.setAntiAliasing(aaState);
    xft.setExcludeRange(m_excludeFrom, m_excludeTo);

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

    KConfig _cfgfonts("kcmfonts");
    KConfigGroup cfgfonts(&_cfgfonts, "General");
    cfgfonts.writeEntry("forceFontDPI", m_dpi);
    cfgfonts.sync();

    return mod;
}

void FontAASettings::defaults()
{
    setExcludeTo(15);
    setExcludeFrom(8);
    setAntiAliasing(1);
    setDpi(96);
    setSubPixel(KXftConfig::description(KXftConfig::SubPixel::NotSet));
    setHinting(KXftConfig::description(KXftConfig::Hint::NotSet));
}

int FontAASettings::getIndexSubPixel(KXftConfig::SubPixel::Type spType) const
{
    int pos = -1;
    int index;

    for (index = 0; index < m_subPixelOptionsModel->rowCount(); ++index) {
        QStandardItem *item = m_subPixelOptionsModel->item(index);
        if (item->text() == i18n(KXftConfig::description(spType).toUtf8())) {
            pos = index;
            break;
        }
    }

    return pos;
}

KXftConfig::SubPixel::Type FontAASettings::getSubPixelType()
{
    int t;

    for (t = KXftConfig::SubPixel::NotSet; t <= KXftConfig::SubPixel::Vbgr; ++t){
        if (m_subPixel == i18n(KXftConfig::description((KXftConfig::SubPixel::Type)t).toUtf8())) {
            return (KXftConfig::SubPixel::Type)t;
        }
    }
    return KXftConfig::SubPixel::NotSet;
}

int FontAASettings::getIndexHint(KXftConfig::Hint::Style hStyle) const
{
    int pos = -1;
    int index;

    for (index = 0; index < m_hintingOptionsModel->rowCount(); ++index) {
        QStandardItem *item = m_hintingOptionsModel->item(index);
        if (item->text() == i18n(KXftConfig::description(hStyle).toUtf8())) {
            pos = index;
            break;
        }
    }

    return pos;
}

KXftConfig::Hint::Style FontAASettings::getHintStyle()
{
    int s;

    for (s = KXftConfig::Hint::NotSet; s <= KXftConfig::Hint::Full; ++s){
        if (m_hinting == i18n(KXftConfig::description((KXftConfig::Hint::Style)s).toUtf8())) {
            return (KXftConfig::Hint::Style)s;
        }
    }

    return KXftConfig::Hint::Medium;
}

#endif

void FontAASettings::setSubPixel(const QString &subPixel)
{

    if (m_subPixel == subPixel) {
        return;
    }

    m_subPixel = subPixel;
    emit subPixelChanged();
}

QString FontAASettings::subPixel() const
{
    return m_subPixel;
}

void FontAASettings::setHinting(const QString &hinting)
{
    if (m_hinting == hinting) {
        return;
    }

    m_hinting = hinting;
    emit hintingChanged();
}

QString FontAASettings::hinting() const
{
    return m_hinting;
}

void FontAASettings::setExcludeTo(const int &excludeTo)
{
    if (m_excludeTo == excludeTo) {
        return;
    }

    m_excludeTo = excludeTo;
    emit excludeToChanged();
}

int FontAASettings::excludeTo() const
{
    return m_excludeTo;
}

void FontAASettings::setExcludeFrom(const int &excludeTo)
{
    if (m_excludeFrom == excludeTo) {
        return;
    }

    m_excludeFrom = excludeTo;
    emit excludeToChanged();
}

int FontAASettings::excludeFrom() const
{
    return m_excludeFrom;
}

void FontAASettings::setAntiAliasing(const int &antiAliasing)
{
    if (m_antiAliasing == antiAliasing) {
        return;
    }

    m_antiAliasing = antiAliasing;
    emit aliasingChanged();
}

int FontAASettings::antiAliasing() const
{
    return m_antiAliasing;
}

void FontAASettings::setDpi(const int &dpi)
{
    if (m_dpi == dpi) {
        return;
    }

    m_dpi = dpi;
    emit dpiChanged();
}

int FontAASettings::dpi() const
{
    return m_dpi;
}

int FontAASettings::subPixelCurrentIndex()
{
    return getSubPixelType();
}

int FontAASettings::hintingCurrentIndex()
{
    return getHintStyle();
}


/**** KFonts ****/

KFonts::KFonts(QObject *parent, const QVariantList &args)
    : KQuickAddons::ConfigModule(parent, args)
    , m_fontAASettings(new FontAASettings(this))
{
    qApp->setAttribute(Qt::AA_DontCreateNativeWidgetSiblings);
    KAboutData* about = new KAboutData("kcm_fonts", i18n("Configure Fonts"),
                                       "0.1", QString(), KAboutLicense::LGPL);
    about->addAuthor(i18n("Antonis Tsiapaliokas"), QString(), "antonis.tsiapaliokas@kde.org");
    setAboutData(about);
    qmlRegisterType<QStandardItemModel>();
    setButtons(Apply | Default);

    auto updateState = [this]() {
        setNeedsSave(true);
    };

    connect(m_fontAASettings, &FontAASettings::subPixelChanged, this, updateState);
    connect(m_fontAASettings, &FontAASettings::hintingChanged, this, updateState);
    connect(m_fontAASettings, &FontAASettings::excludeToChanged, this, updateState);
    connect(m_fontAASettings, &FontAASettings::antiAliasingChanged, this, updateState);
    connect(m_fontAASettings, &FontAASettings::aliasingChanged, this, updateState);
    connect(m_fontAASettings, &FontAASettings::dpiChanged, this, updateState);
}

KFonts::~KFonts()
{
}

void KFonts::defaults()
{
#ifdef Q_OS_MAC
    setGeneralFont(QFont("Lucida Grande", 13));
    setMenuFont(QFont("Lucida Grande", 13));
    setFixedWidthFont(QFont("Monaco", 10));
    setToolbarFont(QFont("Lucida Grande", 11));
    setSmallFont(QFont("Lucida Grande", 9));
    setWindowTitleFont(QFont("Lucida Grande", 14));
#else
    setGeneralFont(QFont("Noto Sans", 10));
    setMenuFont(QFont("Noto Sans", 10));
    setFixedWidthFont(QFont("Hack", 9));
    setToolbarFont(QFont("Noto Sans", 10));
    setSmallFont(QFont("Noto Sans", 8));
    setWindowTitleFont(QFont("Noto Sans", 10));
#endif

    m_fontAASettings->defaults();
}

void KFonts::load()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig("kdeglobals");

    KConfigGroup cg(config, "General");
    QFont font;
    font.fromString(cg.readEntry("font"));
    setGeneralFont(font);

    font.fromString(cg.readEntry("fixed"));
    setFixedWidthFont(font);

    font.fromString(cg.readEntry("smallestReadableFont"));
    setFixedWidthFont(font);

    font.fromString(cg.readEntry("toolBarFont"));
    setFixedWidthFont(font);

    font.fromString(cg.readEntry("menuFont"));
    setFixedWidthFont(font);

    cg = KConfigGroup(config, "WM");
    font.fromString(cg.readEntry("activeFont"));
    setFixedWidthFont(font);

    m_fontAASettings->load();
    setNeedsSave(false);
}

void KFonts::save()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig("kdeglobals");

    KConfigGroup cg(config, "General");
    cg.writeEntry("font", m_generalFont.toString());
    cg.writeEntry("fixed", m_generalFont.toString());
    cg.writeEntry("smallestReadableFont", m_generalFont.toString());
    cg.writeEntry("toolBarFont", m_generalFont.toString());
    cg.writeEntry("menuFont", m_generalFont.toString());
    cg.sync();
    cg = KConfigGroup(config, "WM");
    cg.writeEntry("activeFont", m_generalFont.toString());
    cg.sync();

    KConfig _cfgfonts("kcmfonts");
    KConfigGroup cfgfonts(&_cfgfonts, "General");

    AASetting aaSetting = (AASetting)m_fontAASettings->antiAliasing();
    cfgfonts.writeEntry("dontChangeAASettings", aaSetting == AASystem);
    if (aaSetting == AAEnabled) {
        m_fontAASettings->save(KXftConfig::AntiAliasing::Enabled);
    } else if (aaSetting == AADisabled) {
        m_fontAASettings->save(KXftConfig::AntiAliasing::Disabled);
    } else {
        m_fontAASettings->save(KXftConfig::AntiAliasing::NotSet);
    }

    runRdb(KRdbExportXftSettings | KRdbExportGtkTheme);
    KGlobalSettings::self()->emitChange(KGlobalSettings::FontChanged);
    emit fontsHaveChanged();
    setNeedsSave(false);
}

void KFonts::setGeneralFont(const QFont &font)
{
    if (m_generalFont == font) {
        return;
    }

    m_generalFont = font;
    emit generalFontChanged();
    setNeedsSave(true);
}

QFont KFonts::generalFont() const
{
    return m_generalFont;
}

void KFonts::setFixedWidthFont(const QFont &font)
{
    if (m_fixedWidthFont == font) {
        return;
    }

    m_fixedWidthFont = font;
    emit fixedWidthFontChanged();
    setNeedsSave(true);
}

QFont KFonts::fixedWidthFont() const
{
    return m_fixedWidthFont;
}

void KFonts::setSmallFont(const QFont &font)
{
    if (m_smallFont == font) {
        return;
    }

    m_smallFont = font;
    emit smallFontChanged();
    setNeedsSave(true);
}

QFont KFonts::smallFont() const
{
    return m_smallFont;
}

void KFonts::setToolbarFont(const QFont &font)
{
    if (m_toolbarFont == font) {
        return;
    }

    m_toolbarFont = font;
    emit toolbarFontChanged();
    setNeedsSave(true);
}

QFont KFonts::toolbarFont() const
{
    return m_toolbarFont;
}

void KFonts::setMenuFont(const QFont &font)
{
    if (m_menuFont == font) {
        return;
    }

    m_menuFont = font;
    emit menuFontChanged();
    setNeedsSave(true);
}

QFont KFonts::menuFont() const
{
    return m_menuFont;
}

void KFonts::setWindowTitleFont(const QFont &font)
{
    if (m_windowTitleFont == font) {
        return;
    }

    m_windowTitleFont = font;
    emit windowTitleFontChanged();
    setNeedsSave(true);
}

QFont KFonts::windowTitleFont() const
{
    return m_windowTitleFont;
}

void KFonts::adjustAllFonts(QFont font)
{
    setGeneralFont(font);
    setMenuFont(font);
    setFixedWidthFont(font);
    setToolbarFont(font);
    setSmallFont(font);
    setWindowTitleFont(font);
}

#include "fonts.moc"

