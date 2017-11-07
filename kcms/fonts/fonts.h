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
    Q_PROPERTY(int excludeTo READ excludeTo WRITE setExcludeTo NOTIFY excludeToChanged)
    Q_PROPERTY(int excludeFrom READ excludeFrom WRITE setExcludeFrom NOTIFY excludeFromChanged)
    Q_PROPERTY(int antiAliasing READ antiAliasing WRITE setAntiAliasing NOTIFY aliasingChanged)
    Q_PROPERTY(int dpi READ dpi WRITE setDpi NOTIFY dpiChanged)

public:

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
    int m_dpi;
    QStandardItemModel *m_subPixelOptionsModel;
    QStandardItemModel *m_hintingOptionsModel;
#endif
};

/**
 * The Desktop/fonts tab in kcontrol.
 */


struct FontsType {
    QString name;
    QString category;
    QString status;
    QFont font;
    QString fontName;
};

class KFonts : public KQuickAddons::ConfigModule
{
    Q_OBJECT
    Q_PROPERTY(QAbstractItemModel *fontsModel READ fontsModel CONSTANT)
    Q_PROPERTY(QObject *fontAASettings READ fontAASettings CONSTANT)

public:

    enum Roles {
        CategoryRole = Qt::UserRole + 1,
        StatusRole,
        FontRole
    };

    KFonts(QObject *parent, const QVariantList &);
    ~KFonts();


    QStandardItemModel* fontsModel() { return m_fontsModel; }
    QObject* fontAASettings() { return m_fontAASettings; }

public Q_SLOTS:
    void load();
    void save();
    void defaults();
    Q_INVOKABLE void updateFont(int currentIndex, QFont font);
    Q_INVOKABLE void adjustAllFonts(QFont);

Q_SIGNALS:
    void fontsHaveChanged();

private:
    enum AASetting { AAEnabled, AASystem, AADisabled };
    QStandardItemModel *m_fontsModel;
    FontAASettings *m_fontAASettings;
};

#endif

