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

#ifndef FONTS_H
#define FONTS_H

#include <config-X11.h>
#include <QAbstractItemModel>
#include <QStandardItemModel>

#include <KQuickAddons/ManagedConfigModule>

#include "kxftconfig.h"

class FontsSettings;

class FontAASettings : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QAbstractItemModel *subPixelOptionsModel READ subPixelOptionsModel CONSTANT)
    Q_PROPERTY(int subPixelCurrentIndex READ subPixelCurrentIndex WRITE setSubPixelCurrentIndex NOTIFY subPixelCurrentIndexChanged)
    Q_PROPERTY(QAbstractItemModel *hintingOptionsModel READ hintingOptionsModel CONSTANT)
    Q_PROPERTY(int hintingCurrentIndex READ hintingCurrentIndex WRITE setHintingCurrentIndex NOTIFY hintingCurrentIndexChanged)

    Q_PROPERTY(bool exclude READ exclude WRITE setExclude NOTIFY excludeChanged)
    Q_PROPERTY(int excludeTo READ excludeTo WRITE setExcludeTo NOTIFY excludeToChanged)
    Q_PROPERTY(int excludeFrom READ excludeFrom WRITE setExcludeFrom NOTIFY excludeFromChanged)
    Q_PROPERTY(bool antiAliasing READ antiAliasing WRITE setAntiAliasing NOTIFY aliasingChanged)
    Q_PROPERTY(int dpi READ dpi WRITE setDpi NOTIFY dpiChanged)

    struct State
    {
        bool exclude;
        int excludeFrom;
        int excludeTo;
        int antiAliasing;
        bool antiAliasingHasLocalConfig;
        bool subPixelHasLocalConfig;
        bool hintingHasLocalConfig;
        int dpi;
        int subPixel;
        int hinting;

        bool operator==(const State& other) const;
        bool operator!=(const State& other) const;
    };

public:
    enum AASetting { AAEnabled, AASystem, AADisabled };
#if defined(HAVE_FONTCONFIG) && HAVE_X11
    FontAASettings(QObject *parent);

    bool save(KXftConfig::AntiAliasing::State aaState);
    void load();
    void defaults();
    QAbstractItemModel* subPixelOptionsModel() { return m_subPixelOptionsModel; }
    QAbstractItemModel* hintingOptionsModel() { return m_hintingOptionsModel; }

    void setExclude(bool exclude);
    bool exclude() const;

    void setExcludeTo(int excludeTo);
    int excludeTo() const;

    void setExcludeFrom(int excludeFrom);
    int excludeFrom() const;

    void setAntiAliasing(bool antiAliasing);
    bool antiAliasing() const;

    bool antiAliasingNeedsSave() const;
    bool subPixelNeedsSave() const;
    bool hintingNeedsSave() const;

    void setDpi(const int &dpi);
    int dpi() const;

    int subPixelCurrentIndex();
    void setSubPixelCurrentIndex(int idx);
    void setSubPixel(int idx);
    int hintingCurrentIndex();
    void setHintingCurrentIndex(int idx);
    void setHinting(int idx);

    bool needsSave() const;
    bool isDefaults() const;

#endif

Q_SIGNALS:
    void excludeChanged();
    void excludeToChanged();
    void excludeFromChanged();
    void antiAliasingChanged();
    void aliasingChangeApplied();
    void aliasingChanged();
    void dpiChanged();
    void subPixelCurrentIndexChanged();
    void hintingCurrentIndexChanged();

#if defined(HAVE_FONTCONFIG) && HAVE_X11
private:
    State m_state;
    State m_originalState;
    QStandardItemModel *m_subPixelOptionsModel;
    QStandardItemModel *m_hintingOptionsModel;
#endif
};

/**
 * The Desktop/fonts tab in kcontrol.
 */
class KFonts : public KQuickAddons::ManagedConfigModule
{
    Q_OBJECT
    Q_PROPERTY(FontsSettings *fontsSettings READ fontsSettings CONSTANT)
    Q_PROPERTY(QObject *fontAASettings READ fontAASettings CONSTANT)

public:
    KFonts(QObject *parent, const QVariantList &);
    ~KFonts() override;

    FontsSettings *fontsSettings() const;

    QObject* fontAASettings() { return m_fontAASettings; }

public Q_SLOTS:
    void load() override;
    void save() override;
    void defaults() override;
    Q_INVOKABLE void adjustAllFonts();

Q_SIGNALS:
    void fontsHaveChanged();

private:
    bool isSaveNeeded() const override;
    bool isDefaults() const override;
    QFont applyFontDiff(const QFont &fnt, const QFont &newFont, int fontDiffFlags);
    void setNearestExistingFonts();

    FontsSettings *m_settings;

    FontAASettings *m_fontAASettings;
};

#endif

