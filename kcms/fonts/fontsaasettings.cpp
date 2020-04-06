/**
 *  Copyright 2020 Benjamin Port <benjamin.port@enioka.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "fontsaasettings.h"

#include <QDebug>
#include <KWindowSystem>

#include "kxftconfig.h"

namespace {
    bool defaultExclude()
    {
        return false;
    }

    int defaultExcludeFrom()
    {
        return 8;
    }

    int defaultExcludeTo()
    {
        return 15;
    }

    bool defaultAntiAliasing()
    {
        return true;
    }

    int defaultSubPixel()
    {
        return KXftConfig::SubPixel::Rgb;
    }

    int defaultHinting()
    {
        return KXftConfig::Hint::Slight;
    }
}


class FontAASettingsStore : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool exclude READ exclude WRITE setExclude)
    Q_PROPERTY(int excludeFrom READ excludeFrom WRITE setExcludeFrom)
    Q_PROPERTY(int excludeTo READ excludeTo WRITE setExcludeTo)
    Q_PROPERTY(bool antiAliasing READ antiAliasing WRITE setAntiAliasing)
    Q_PROPERTY(KXftConfig::SubPixel::Type subPixel READ subPixel WRITE setSubPixel)
    Q_PROPERTY(KXftConfig::Hint::Style hinting READ hinting WRITE setHinting)
public:
    FontAASettingsStore(FontsAASettings *parent = nullptr)
        : QObject(parent)
        , m_settings(parent)
    {
        load();
    }

    bool exclude() const
    {
        return m_exclude;
    }

    void setExclude(bool exclude)
    {
        m_exclude = exclude;
    }

    int excludeFrom() const
    {
        return m_excludeFrom;
    }

    void setExcludeFrom(int excludeFrom)
    {
        m_excludeFrom = excludeFrom;
    }

    int excludeTo() const
    {
        return m_excludeTo;
    }

    void setExcludeTo(int excludeTo)
    {
        m_excludeTo = excludeTo;
    }

    bool isImmutable() const
    {
        return m_isImmutable;
    }

    bool antiAliasing() const
    {
        return m_antiAliasing;
    }

    void setAntiAliasing(bool antiAliasing)
    {
        if (antiAliasing != m_antiAliasing) {
            m_antiAliasingChanged = true;
        }
        m_antiAliasing = antiAliasing;
    }

    KXftConfig::SubPixel::Type subPixel() const
    {
        return m_subPixel;
    }

    void setSubPixel(KXftConfig::SubPixel::Type subPixel)
    {
        m_subPixel = subPixel;
    }

    KXftConfig::Hint::Style hinting() const
    {
        return m_hinting;
    }

    void setHinting(KXftConfig::Hint::Style hinting)
    {
        m_hinting = hinting;
    }

    void save()
    {
        KXftConfig xft;
        KXftConfig::AntiAliasing::State aaState = KXftConfig::AntiAliasing::NotSet;
        if (m_antiAliasingChanged || xft.antiAliasingHasLocalConfig()) {
            aaState = m_antiAliasing ? KXftConfig::AntiAliasing::Enabled : KXftConfig::AntiAliasing::Disabled;
        }
        xft.setAntiAliasing(aaState);

        if (m_exclude) {
            xft.setExcludeRange(m_excludeFrom, m_excludeTo);
        } else {
            xft.setExcludeRange(0, 0);
        }

        KXftConfig::SubPixel::Type spType = static_cast<KXftConfig::SubPixel::Type>(m_subPixel);
        if (m_subPixelChanged || xft.subPixelTypeHasLocalConfig()) {
            xft.setSubPixelType(spType);
        } else {
            xft.setSubPixelType(KXftConfig::SubPixel::NotSet);
        }

        KXftConfig::Hint::Style hStyle = static_cast<KXftConfig::Hint::Style>(m_hinting);
        if (m_hintingChanged || xft.hintStyleHasLocalConfig()) {
            xft.setHintStyle(hStyle);
        } else {
            xft.setHintStyle(KXftConfig::Hint::NotSet);
        }

        // Write to KConfig to sync with krdb
        KSharedConfig::Ptr config = KSharedConfig::openConfig("kdeglobals");
        KConfigGroup grp(config, "General");

        grp.writeEntry("XftSubPixel", KXftConfig::toStr(spType));

        if (aaState == KXftConfig::AntiAliasing::NotSet) {
           grp.revertToDefault("XftAntialias");
        } else {
            grp.writeEntry("XftAntialias", aaState == KXftConfig::AntiAliasing::Enabled);
        }

        QString hs(KXftConfig::toStr(hStyle));
        if (hs != grp.readEntry("XftHintStyle")) {
            if (KXftConfig::Hint::NotSet == hStyle) {
                grp.revertToDefault("XftHintStyle");
            } else {
                grp.writeEntry("XftHintStyle", hs);
            }
        }

        xft.apply();

        m_subPixelChanged = false;
        m_hintingChanged = false;
        m_antiAliasingChanged = false;
    }

    void load()
    {
        double from, to;
        KXftConfig xft;

        if (xft.getExcludeRange(from, to)) {
            setExclude(true);
            setExcludeFrom(from);
            setExcludeTo(to);
        } else {
            setExclude(defaultExclude());
            setExcludeFrom(defaultExcludeFrom());
            setExcludeTo(defaultExcludeTo());
        }

        // sub pixel
        KXftConfig::SubPixel::Type spType = KXftConfig::SubPixel::NotSet;
        xft.getSubPixelType(spType);
        // if it is not set, we set it to rgb
        if (spType == KXftConfig::SubPixel::NotSet) {
            spType = KXftConfig::SubPixel::Rgb;
        }
        setSubPixel(spType);

        // hinting
        KXftConfig::Hint::Style hStyle = KXftConfig::Hint::NotSet;
        xft.getHintStyle(hStyle);
        // if it is not set, we set it to slight hinting
        if (hStyle == KXftConfig::Hint::NotSet) {
            hStyle = KXftConfig::Hint::Slight;
        }
        setHinting(hStyle);

        KSharedConfig::Ptr config = KSharedConfig::openConfig("kdeglobals");
        KConfigGroup cg(config, "General");
        m_isImmutable = cg.isEntryImmutable("XftAntialias");

        const auto aaState = xft.getAntiAliasing();
        setAntiAliasing(aaState != KXftConfig::AntiAliasing::Disabled);

        m_subPixelChanged = false;
        m_hintingChanged = false;
        m_antiAliasingChanged = false;
    }

private:
    FontsAASettings *m_settings;
    bool m_isImmutable;
    bool m_antiAliasing;
    bool m_antiAliasingChanged;
    KXftConfig::SubPixel::Type m_subPixel;
    bool m_subPixelChanged;
    KXftConfig::Hint::Style m_hinting;
    bool m_hintingChanged;
    bool m_exclude;
    int m_excludeFrom;
    int m_excludeTo;
};

FontsAASettings::FontsAASettings(QObject *parent)
    : FontsAASettingsBase(parent)
    , m_fontAASettingsStore(new FontAASettingsStore(this))
{
    addItemInternal("exclude", defaultExclude(), &FontsAASettings::excludeChanged);
    addItemInternal("excludeFrom", defaultExcludeFrom(), &FontsAASettings::excludeFromChanged);
    addItemInternal("excludeTo", defaultExcludeTo(), &FontsAASettings::excludeToChanged);
    addItemInternal("antiAliasing", defaultAntiAliasing(), &FontsAASettings::antiAliasingChanged);
    addItemInternal("subPixel", defaultSubPixel(), &FontsAASettings::subPixelChanged);
    addItemInternal("hinting", defaultHinting(), &FontsAASettings::hintingChanged);

    connect(this, &FontsAASettings::forceFontDPIWaylandChanged, this, &FontsAASettings::dpiChanged);
    connect(this, &FontsAASettings::forceFontDPIChanged, this, &FontsAASettings::dpiChanged);
}

void FontsAASettings::addItemInternal(const QByteArray &propertyName, const QVariant &defaultValue, NotifySignalType notifySignal)
{
    auto item = new KPropertySkeletonItem(m_fontAASettingsStore, propertyName, defaultValue);
    addItem(item, propertyName);
    item->setNotifyFunction([this, notifySignal] { emit (this->*notifySignal)(); });
}

bool FontsAASettings::exclude() const
{
    return findItem("exclude")->property().toBool();
}

void FontsAASettings::setExclude(bool exclude)
{
    findItem("exclude")->setProperty(exclude);
}

int FontsAASettings::excludeFrom() const
{
    return findItem("excludeFrom")->property().toInt();
}

void FontsAASettings::setExcludeFrom(int excludeFrom)
{
    findItem("excludeFrom")->setProperty(excludeFrom);
}

int FontsAASettings::excludeTo() const
{
    return findItem("excludeTo")->property().toInt();
}

void FontsAASettings::setExcludeTo(int excludeTo)
{
    findItem("excludeTo")->setProperty(excludeTo);
}

bool FontsAASettings::antiAliasing() const
{
    return findItem("antiAliasing")->property().toBool();
}

void FontsAASettings::setAntiAliasing(bool antiAliasing)
{
    findItem("antiAliasing")->setProperty(antiAliasing);
}

int FontsAASettings::dpi() const
{
    if (KWindowSystem::isPlatformWayland()) {
        return forceFontDPIWayland();
    } else {
        return forceFontDPI();
    }
}

void FontsAASettings::setDpi(int newDPI)
{
    if (dpi() == newDPI) {
        return;
    }
    if (KWindowSystem::isPlatformWayland()) {
        setForceFontDPIWayland(newDPI);
    } else {
        setForceFontDPI(newDPI);
    }
    emit dpiChanged();
}

KXftConfig::SubPixel::Type FontsAASettings::subPixel() const
{
    return findItem("subPixel")->property().value<KXftConfig::SubPixel::Type>();
}

void FontsAASettings::setSubPixel(KXftConfig::SubPixel::Type subPixel)
{
    findItem("subPixel")->setProperty(subPixel);
}

KXftConfig::Hint::Style FontsAASettings::hinting() const
{
    return findItem("hinting")->property().value<KXftConfig::Hint::Style>();
}

bool FontsAASettings::isAaImmutable() const
{
    return m_fontAASettingsStore->isImmutable();
}

bool FontsAASettings::excludeStateProxy() const
{
    return false;
}

void FontsAASettings::setHinting(KXftConfig::Hint::Style hinting)
{
    findItem("hinting")->setProperty(hinting);
}

bool FontsAASettings::usrSave()
{
    m_fontAASettingsStore->save();
    return FontsAASettingsBase::usrSave();
}

#include "fontsaasettings.moc"
