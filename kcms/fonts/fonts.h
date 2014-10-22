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

#include <QLabel>
#include <QList>
#include <config-X11.h>

#include <KCModule>
#include <KDialog>
#include <KFontRequester>

#include "kxftconfig.h"

class QCheckBox;
class QComboBox;
class QSpinBox;
class KDoubleNumInput;
class FontAASettings;

class FontUseItem : public KFontRequester
{
    Q_OBJECT

public:
    FontUseItem(QWidget *parent, const QString &name, const QString &grp,
                const QString &key, const QString &rc, const QFont &default_fnt,
                bool fixed = false);

    void readFont();
    void writeFont();
    void setDefault();
    void applyFontDiff(const QFont &fnt, int fontDiffFlags);

    const QString &rcFile()
    {
        return _rcfile;
    }
    const QString &rcGroup()
    {
        return _rcgroup;
    }
    const QString &rcKey()
    {
        return _rckey;
    }

private:
    QString _rcfile;
    QString _rcgroup;
    QString _rckey;
    QFont _default;
};

class FontAASettings : public KDialog
{
    Q_OBJECT

public:

#if defined(HAVE_FONTCONFIG) && defined (HAVE_X11)
    FontAASettings(QWidget *parent);

    bool save(KXftConfig::AntiAliasing::State aaState);
    bool load();
    void defaults();
    int getIndex(KXftConfig::SubPixel::Type spType);
    KXftConfig::SubPixel::Type getSubPixelType();
    int getIndex(KXftConfig::Hint::Style hStyle);
    KXftConfig::Hint::Style getHintStyle();
    void setAntiAliasingState(KXftConfig::AntiAliasing::State aaState);
    void enableWidgets();
    int exec();
#endif

protected Q_SLOTS:

    void changed();

#if defined(HAVE_FONTCONFIG) && defined (HAVE_X11)
private:

    QCheckBox *excludeRange;
    KDoubleNumInput *excludeFrom;
    KDoubleNumInput *excludeTo;
    QComboBox *subPixelType;
    QComboBox *hintingStyle;
    QLabel    *subPixelLabel;
    QLabel    *excludeToLabel;
    bool      changesMade;
#endif
};

/**
 * The Desktop/fonts tab in kcontrol.
 */
class KFonts : public KCModule
{
    Q_OBJECT

public:
    KFonts(QWidget *parent, const QVariantList &);
    ~KFonts();

    virtual void load();
    virtual void save();
    virtual void defaults();

protected Q_SLOTS:
    void fontSelected();
    void slotApplyFontDiff();
    void slotUseAntiAliasing();
    void slotCfgAa();

private:
#if defined(HAVE_FONTCONFIG) && defined (HAVE_X11)
    enum AASetting { AAEnabled, AASystem, AADisabled };
    AASetting useAA, useAA_original;
    QComboBox *cbAA;
    QPushButton *aaSettingsButton;
    FontAASettings *aaSettings;
#endif
#if HAVE_X11
    int dpi_original;
    QCheckBox *checkboxForceDpi;
    QSpinBox *spinboxDpi;
#endif
    QList<FontUseItem *> fontUseList;
};

#endif

