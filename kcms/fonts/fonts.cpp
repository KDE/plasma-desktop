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
#include "kxftconfig.h"

#include "fontssettings.h"
#include "fontsaasettings.h"

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

/**** KFonts ****/

KFonts::KFonts(QObject *parent, const QVariantList &args)
    : KQuickAddons::ManagedConfigModule(parent, args)
    , m_settings(new FontsSettings(this))
    , m_settingsAA(new FontsAASettings(this))
    , m_subPixelOptionsModel(new QStandardItemModel(this))
    , m_hintingOptionsModel(new QStandardItemModel(this))
{
    KAboutData* about = new KAboutData("kcm_fonts", i18n("Fonts"),
                                       "0.1", QString(), KAboutLicense::LGPL);
    about->addAuthor(i18n("Antonis Tsiapaliokas"), QString(), "antonis.tsiapaliokas@kde.org");
    setAboutData(about);
    qmlRegisterType<QStandardItemModel>();
    qmlRegisterType<FontsSettings>();
    qmlRegisterType<FontsAASettings>();

    setButtons(Apply | Default | Help);

    for (KXftConfig::SubPixel::Type t : {KXftConfig::SubPixel::None, KXftConfig::SubPixel::Rgb, KXftConfig::SubPixel::Bgr, KXftConfig::SubPixel::Vrgb, KXftConfig::SubPixel::Vbgr}) {
        auto item = new QStandardItem(KXftConfig::description(t));
        m_subPixelOptionsModel->appendRow(item);
    }

    for (KXftConfig::Hint::Style s : {KXftConfig::Hint::None, KXftConfig::Hint::Slight, KXftConfig::Hint::Medium, KXftConfig::Hint::Full}) {
        auto item = new QStandardItem(KXftConfig::description(s));
        m_hintingOptionsModel->appendRow(item);
    }
    connect(m_settingsAA, &FontsAASettings::hintingChanged, this, &KFonts::hintingCurrentIndexChanged);
    connect(m_settingsAA, &FontsAASettings::subPixelChanged, this, &KFonts::subPixelCurrentIndexChanged);
}

KFonts::~KFonts()
{
}

FontsSettings *KFonts::fontsSettings() const
{
    return m_settings;
}

FontsAASettings *KFonts::fontsAASettings() const
{
    return m_settingsAA;
}

QAbstractItemModel *KFonts::subPixelOptionsModel() const
{
    return m_subPixelOptionsModel;
}

QAbstractItemModel *KFonts::hintingOptionsModel() const
{
    return m_hintingOptionsModel;
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

    // Then set the existing fonts based on those settings
    setNearestExistingFonts();

    // Load preview
    // NOTE: This needs to be done AFTER AA settings is loaded
    // otherwise AA settings will be reset in process of loading
    // previews
    engine()->addImageProvider("preview", new PreviewImageProvider(m_settings->font()));

    // KCM expect save state to be false at this point (can be true because of setNearestExistingFonts
    setNeedsSave(false);
}

void KFonts::save()
{
    auto dpiItem = m_settingsAA->findItem("forceFontDPI");
    auto dpiWaylandItem = m_settingsAA->findItem("forceFontDPIWayland");
    auto antiAliasingItem = m_settingsAA->findItem("antiAliasing");
    Q_ASSERT(dpiItem && dpiWaylandItem && antiAliasingItem);
    if (dpiItem->isSaveNeeded() || dpiWaylandItem->isSaveNeeded() || antiAliasingItem->isSaveNeeded()) {
        emit aliasingChangeApplied();
    }

    auto forceFontDPIChanged = dpiItem->isSaveNeeded();

    ManagedConfigModule::save();

#if HAVE_X11
    // if the setting is reset in the module, remove the dpi value,
    // otherwise don't explicitly remove it and leave any possible system-wide value
    if (m_settingsAA->forceFontDPI() == 0 && forceFontDPIChanged && !KWindowSystem::isPlatformWayland()) {
        QProcess proc;
        proc.setProcessChannelMode(QProcess::ForwardedChannels);
        proc.start("xrdb", QStringList() << "-quiet" << "-remove" << "-nocpp");
        if (proc.waitForStarted()) {
            proc.write(QByteArray("Xft.dpi\n"));
            proc.closeWriteChannel();
            proc.waitForFinished();
        }
    }
    QApplication::processEvents();
#endif


    KGlobalSettings::self()->emitChange(KGlobalSettings::FontChanged);

    runRdb(KRdbExportXftSettings | KRdbExportGtkTheme);

    emit fontsHaveChanged();
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

int KFonts::subPixelCurrentIndex() const
{
    return m_settingsAA->subPixel() - KXftConfig::SubPixel::None;
}

void KFonts::setSubPixelCurrentIndex(int idx)
{
    m_settingsAA->setSubPixel(static_cast<KXftConfig::SubPixel::Type>(KXftConfig::SubPixel::None + idx));
}

int KFonts::hintingCurrentIndex() const
{
    return m_settingsAA->hinting() - KXftConfig::Hint::None;
}

void KFonts::setHintingCurrentIndex(int idx)
{
    m_settingsAA->setHinting(static_cast<KXftConfig::Hint::Style>(KXftConfig::Hint::None + idx));
}

#include "fonts.moc"

