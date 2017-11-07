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
    , m_fontsModel(new QStandardItemModel(this))
    , m_fontAASettings(new FontAASettings(this))
{
    qApp->setAttribute(Qt::AA_DontCreateNativeWidgetSiblings);
    KAboutData* about = new KAboutData("kcm_fonts", i18n("Configure Fonts"),
                                       "0.1", QString(), KAboutLicense::LGPL);
    about->addAuthor(i18n("Antonis Tsiapaliokas"), QString(), "antonis.tsiapaliokas@kde.org");
    setAboutData(about);
    qmlRegisterType<QStandardItemModel>();
    setButtons(Apply | Default);

    QHash<int, QByteArray> roles = m_fontsModel->roleNames();
    roles[CategoryRole] = "categoryName";
    roles[StatusRole] = "statusName";
    roles[FontRole] = "font";
    m_fontsModel->setItemRoleNames(roles);
}

KFonts::~KFonts()
{
}

void KFonts::defaults()
{
    QList<QFont> defaultFonts;
#ifdef Q_OS_MAC
    defaultFonts << QFont("Lucida Grande", 13) // general/menu/desktop
        << QFont("Monaco", 10)
        << QFont("Lucida Grande", 11); // toolbar
#elif defined(Q_WS_MAEMO_5) || defined(MEEGO_EDITION_HARMATTAN)
    defaultFonts << QFont("Sans Serif", 16) // general/menu/desktop
        << QFont("Monospace", 16)
        << QFont("Sans Serif", 16); // toolbar
#else
    defaultFonts << QFont("Oxygen-Sans", 10) // general/menu/desktop
        <<QFont("Oxygen Mono", 9) // fixed font
        << QFont("Oxygen-Sans", 8) //small
        << QFont("Oxygen-Sans", 9); // toolbar
#endif
        defaultFonts
            << QFont("Oxygen-Sans", 10) // smallestReadableFont
            << QFont("Oxygen-Sans", 10); // window title

    int count = 0;
    for (const auto font : defaultFonts) {
        updateFont(count, font);
        count++;
    }

    m_fontAASettings->defaults();
}

void KFonts::load()
{
    QList<FontsType> fonts;
    fonts << FontsType { "General:", "General", "font" }
        << FontsType { "Fixed width:", "General", "fixed" }
        << FontsType { "Small:", "General", "smallestReadableFont" }
        << FontsType { "Toolbar:", "General", "toolBarFont" }
        << FontsType { "Menu:", "General", "menuFont" }
        << FontsType { "Window Title:", "WM", "activeFont" };

    KSharedConfig::Ptr config = KSharedConfig::openConfig("kdeglobals");
    for (auto it : fonts) {

        KConfigGroup cg(config, it.category);
        QStandardItem *item = new QStandardItem(it.name);
        item->setData(it.category, CategoryRole);
        item->setData(it.status, StatusRole);

        QFont font;
        font.fromString(cg.readEntry(it.status));
        item->setData(font, FontRole);

        m_fontsModel->appendRow(item);
    }

    m_fontAASettings->load();
}

void KFonts::save()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig("kdeglobals");
    for (int i = 0; i < m_fontsModel->rowCount(); i++) {
        QStandardItem *item = m_fontsModel->item(i);

        KConfigGroup cg(config, item->data(CategoryRole).toString());
        cg.writeEntry(item->data(StatusRole).toString(), item->data(FontRole).toString());
        cg.sync();
    }

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

void KFonts::updateFont(int currentIndex, QFont font)
{
    QStandardItem *item = m_fontsModel->item(currentIndex);
    item->setData(font, FontRole);

    setNeedsSave(true);
}


void KFonts::adjustAllFonts(QFont font)
{
    for (int i = 0; i < m_fontsModel->rowCount(); i++) {
        updateFont(i, font);
    }
}

#include "fonts.moc"

