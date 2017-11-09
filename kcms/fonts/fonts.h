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

#ifndef FONTS_H
#define FONTS_H

#include <config-X11.h>
#include <QAbstractItemModel>
#include <QStandardItemModel>

#include <KQuickAddons/ConfigModule>

#include "kxftconfig.h"

class FontAASettings : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QAbstractItemModel *subPixelOptionsModel READ subPixelOptionsModel CONSTANT)
    Q_PROPERTY(QAbstractItemModel *hintingOptionsModel READ hintingOptionsModel CONSTANT)
    Q_PROPERTY(QString subPixel READ subPixel WRITE setSubPixel NOTIFY subPixelChanged)
    Q_PROPERTY(QString hinting READ hinting WRITE setHinting NOTIFY hintingChanged)
    Q_PROPERTY(bool exclude READ exclude WRITE setExclude NOTIFY excludeChanged)
    Q_PROPERTY(int excludeTo READ excludeTo WRITE setExcludeTo NOTIFY excludeToChanged)
    Q_PROPERTY(int excludeFrom READ excludeFrom WRITE setExcludeFrom NOTIFY excludeFromChanged)
    Q_PROPERTY(int antiAliasing READ antiAliasing WRITE setAntiAliasing NOTIFY aliasingChanged)
    Q_PROPERTY(int dpi READ dpi WRITE setDpi NOTIFY dpiChanged)

public:
    enum AASetting { AAEnabled, AASystem, AADisabled };
#if defined(HAVE_FONTCONFIG) && defined (HAVE_X11)
    FontAASettings(QObject *parent);

    bool save(KXftConfig::AntiAliasing::State aaState);
    void load();
    void defaults();
    int getIndexSubPixel(KXftConfig::SubPixel::Type spType) const;
    KXftConfig::SubPixel::Type getSubPixelType();
    int getIndexHint(KXftConfig::Hint::Style hStyle) const;
    KXftConfig::Hint::Style getHintStyle();
    void setAntiAliasingState(KXftConfig::AntiAliasing::State aaState);
    QAbstractItemModel* subPixelOptionsModel() { return m_subPixelOptionsModel; }
    QAbstractItemModel* hintingOptionsModel() { return m_hintingOptionsModel; }

    void setSubPixel(const QString &subPixel);
    QString subPixel() const;

    void setHinting(const QString &hinting);
    QString hinting() const;

    void setExclude(bool exclude);
    bool exclude() const;

    void setExcludeTo(const int &excludeTo);
    int excludeTo() const;

    void setExcludeFrom(const int &excludeTo);
    int excludeFrom() const;

    void setAntiAliasing(const int& antiAliasing);
    int antiAliasing() const;

    void setDpi(const int &dpi);
    int dpi() const;

    Q_INVOKABLE int subPixelCurrentIndex();
    Q_INVOKABLE int hintingCurrentIndex();
    
#endif

Q_SIGNALS:
    void subPixelChanged();
    void hintingChanged();
    void excludeChanged();
    void excludeToChanged();
    void excludeFromChanged();
    void antiAliasingChanged();
    void aliasingChanged();
    void dpiChanged();

#if defined(HAVE_FONTCONFIG) && defined (HAVE_X11)
private:
    QString m_subPixel;
    QString m_hinting;
    int m_excludeTo;
    int m_excludeFrom;
    int m_antiAliasing;
    int m_antiAliasingOriginal;
    int m_dpi;
    int m_dpiOriginal;
    QStandardItemModel *m_subPixelOptionsModel;
    QStandardItemModel *m_hintingOptionsModel;
    bool m_exclude = false;
#endif
};

/**
 * The Desktop/fonts tab in kcontrol.
 */
class KFonts : public KQuickAddons::ConfigModule
{
    Q_OBJECT
    Q_PROPERTY(QFont generalFont READ generalFont WRITE setGeneralFont NOTIFY generalFontChanged)
    Q_PROPERTY(QFont fixedWidthFont READ fixedWidthFont WRITE setFixedWidthFont NOTIFY fixedWidthFontChanged)
    Q_PROPERTY(QFont smallFont READ smallFont WRITE setSmallFont NOTIFY smallFontChanged)
    Q_PROPERTY(QFont toolbarFont READ toolbarFont WRITE setToolbarFont NOTIFY toolbarFontChanged)
    Q_PROPERTY(QFont menuFont READ menuFont WRITE setMenuFont NOTIFY menuFontChanged)
    Q_PROPERTY(QFont windowTitleFont READ windowTitleFont WRITE setWindowTitleFont NOTIFY windowTitleFontChanged)
    Q_PROPERTY(QObject *fontAASettings READ fontAASettings CONSTANT)

public:
    KFonts(QObject *parent, const QVariantList &);
    ~KFonts();

    void setGeneralFont(const QFont &font);
    QFont generalFont() const;

    void setFixedWidthFont(const QFont &font);
    QFont fixedWidthFont() const;

    void setSmallFont(const QFont &font);
    QFont smallFont() const;

    void setToolbarFont(const QFont &font);
    QFont toolbarFont() const;

    void setMenuFont(const QFont &font);
    QFont menuFont() const;

    void setWindowTitleFont(const QFont &font);
    QFont windowTitleFont() const;

    QObject* fontAASettings() { return m_fontAASettings; }

public Q_SLOTS:
    void load();
    void save();
    void defaults();
    Q_INVOKABLE void adjustAllFonts();
    //TODO: replace this with the new labs platform dialogs?
    Q_INVOKABLE QFont chooseFont(const QFont &font);

Q_SIGNALS:
    void fontsHaveChanged();

    void generalFontChanged();
    void fixedWidthFontChanged();
    void smallFontChanged();
    void toolbarFontChanged();
    void menuFontChanged();
    void windowTitleFontChanged();

private:
    QFont applyFontDiff(const QFont &fnt, const QFont &newFont, int fontDiffFlags);

    QFont m_generalFont;
    QFont m_fixedWidthFont;
    QFont m_smallFont;
    QFont m_toolbarFont;
    QFont m_menuFont;
    QFont m_windowTitleFont;

    FontAASettings *m_fontAASettings;
};

#endif

