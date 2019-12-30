/*
 * KFontInst - KDE Font Installer
 *
 * Copyright 2003-2007 Craig Drummond <craig@kde.org>
 *
 * ----
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "Fc.h"
#include <QTextStream>
#include <QHash>
#include <QUrlQuery>
//
// KDE font chooser always seems to use Italic - for both Oblique, and Italic. So I guess
// the fonts:/ should do too - so as to appear more unified.
//
// ditto with respect to Medium/Regular
#define KFI_HAVE_OBLIQUE         // Do we differentiate between Italic and Oblique when comparing slants?
#define KFI_DISPLAY_OBLIQUE      // Do we want to list "Oblique"? Or always use Italic?
#define KFI_HAVE_MEDIUM_WEIGHT   // Do we differentiate between Medium and Regular weights when comparing weights?
#define KFI_DISPLAY_MEDIUM      // Do we want to list "Medium"? Or always use Regular?

namespace KFI
{

namespace FC
{

#define FC_PROTOCOL    QString::fromLatin1("fontconfig")
#define FC_STYLE_QUERY QString::fromLatin1("style")
#define FC_FILE_QUERY  QString::fromLatin1("file")
#define FC_INDEX_QUERY QString::fromLatin1("index")

QUrl encode(const QString &name, quint32 style, const QString &file, int index)
{
    QUrl url(QUrl::fromLocalFile(name));

    url.setScheme(FC_PROTOCOL);
    url.addQueryItem(FC_STYLE_QUERY, QString::number(style));
    if(!file.isEmpty())
        url.addQueryItem(FC_FILE_QUERY, file);
    if(index>0)
        url.addQueryItem(FC_INDEX_QUERY, QString::number(index));
    return url;
}

Misc::TFont decode(const QUrl &url)
{
    QUrlQuery query(url);
    return FC_PROTOCOL==url.scheme()
                ? Misc::TFont(url.path(), query.queryItemValue(FC_STYLE_QUERY).toUInt())
                : Misc::TFont(QString(), 0);
}

QString getFile(const QUrl &url)
{
    QUrlQuery query(url);
    return FC_PROTOCOL==url.scheme() ? query.queryItemValue(FC_FILE_QUERY) : QString();
}

int getIndex(const QUrl &url)
{
    QUrlQuery query(url);
    return FC_PROTOCOL==url.scheme() ? query.queryItemValue(FC_INDEX_QUERY).toInt() : 0;
}

int weight(int w)
{
    if(KFI_NULL_SETTING==w)
#ifdef KFI_HAVE_MEDIUM_WEIGHT
        return FC_WEIGHT_MEDIUM;
#else
        return FC_WEIGHT_REGULAR;
#endif

    if(w<FC_WEIGHT_EXTRALIGHT)
        return FC_WEIGHT_THIN;

    if(w<(FC_WEIGHT_EXTRALIGHT+FC_WEIGHT_LIGHT)/2)
        return FC_WEIGHT_EXTRALIGHT;

    if(w<(FC_WEIGHT_LIGHT+FC_WEIGHT_REGULAR)/2)
        return FC_WEIGHT_LIGHT;

#ifdef KFI_HAVE_MEDIUM_WEIGHT
    if(w<(FC_WEIGHT_REGULAR+FC_WEIGHT_MEDIUM)/2)
        return FC_WEIGHT_REGULAR;

    if(w<(FC_WEIGHT_MEDIUM+FC_WEIGHT_DEMIBOLD)/2)
        return FC_WEIGHT_MEDIUM;
#else
    if(w<(FC_WEIGHT_REGULAR+FC_WEIGHT_DEMIBOLD)/2)
        return FC_WEIGHT_REGULAR;
#endif

    if(w<(FC_WEIGHT_DEMIBOLD+FC_WEIGHT_BOLD)/2)
        return FC_WEIGHT_DEMIBOLD;

    if(w<(FC_WEIGHT_BOLD+FC_WEIGHT_EXTRABOLD)/2)
        return FC_WEIGHT_BOLD;

    if(w<(FC_WEIGHT_EXTRABOLD+FC_WEIGHT_BLACK)/2)
        return FC_WEIGHT_EXTRABOLD;

    return FC_WEIGHT_BLACK;
}

int width(int w)
{
    if(KFI_NULL_SETTING==w)
        return KFI_FC_WIDTH_NORMAL;

    if(w<KFI_FC_WIDTH_EXTRACONDENSED)
        return KFI_FC_WIDTH_EXTRACONDENSED;

    if(w<(KFI_FC_WIDTH_EXTRACONDENSED+KFI_FC_WIDTH_CONDENSED)/2)
        return KFI_FC_WIDTH_EXTRACONDENSED;

    if(w<(KFI_FC_WIDTH_CONDENSED+KFI_FC_WIDTH_SEMICONDENSED)/2)
        return KFI_FC_WIDTH_CONDENSED;

    if(w<(KFI_FC_WIDTH_SEMICONDENSED+KFI_FC_WIDTH_NORMAL)/2)
        return KFI_FC_WIDTH_SEMICONDENSED;

    if(w<(KFI_FC_WIDTH_NORMAL+KFI_FC_WIDTH_SEMIEXPANDED)/2)
        return KFI_FC_WIDTH_NORMAL;

    if(w<(KFI_FC_WIDTH_SEMIEXPANDED+KFI_FC_WIDTH_EXPANDED)/2)
        return KFI_FC_WIDTH_SEMIEXPANDED;

    if(w<(KFI_FC_WIDTH_EXPANDED+KFI_FC_WIDTH_EXTRAEXPANDED)/2)
        return KFI_FC_WIDTH_EXPANDED;

    if(w<(KFI_FC_WIDTH_EXTRAEXPANDED+KFI_FC_WIDTH_ULTRAEXPANDED)/2)
        return KFI_FC_WIDTH_EXTRAEXPANDED;

    return KFI_FC_WIDTH_ULTRAEXPANDED;
}

int slant(int s)
{
    if(KFI_NULL_SETTING==s || s<FC_SLANT_ITALIC)
        return FC_SLANT_ROMAN;

#ifdef KFI_HAVE_OBLIQUE
    if(s<(FC_SLANT_ITALIC+FC_SLANT_OBLIQUE)/2)
        return FC_SLANT_ITALIC;

    return FC_SLANT_OBLIQUE;
#else
    return FC_SLANT_ITALIC;
#endif
}

int spacing(int s)
{
    if(s<FC_MONO)
        return FC_PROPORTIONAL;

    if(s<(FC_MONO+FC_CHARCELL)/2)
        return FC_MONO;

    return FC_CHARCELL;
}

int strToWeight(const QString &str, QString &newStr)
{
    if(0==str.indexOf(i18n(KFI_WEIGHT_THIN), 0, Qt::CaseInsensitive))
    {
        newStr=str.mid(i18n(KFI_WEIGHT_THIN).length());
        return FC_WEIGHT_THIN;
    }
    if(0==str.indexOf(i18n(KFI_WEIGHT_EXTRALIGHT), 0, Qt::CaseInsensitive))
    {
        newStr=str.mid(i18n(KFI_WEIGHT_EXTRALIGHT).length());
        return FC_WEIGHT_EXTRALIGHT;
    }
    //if(0==str.indexOf(i18n(KFI_WEIGHT_ULTRALIGHT), 0, Qt::CaseInsensitive))
    //{
    //    newStr=str.mid(i18n(KFI_WEIGHT_ULTRALIGHT).length());
    //    return FC_WEIGHT_ULTRALIGHT;
    //}
    if(0==str.indexOf(i18n(KFI_WEIGHT_LIGHT), 0, Qt::CaseInsensitive))
    {
        newStr=str.mid(i18n(KFI_WEIGHT_LIGHT).length());
        return FC_WEIGHT_LIGHT;
    }
    if(0==str.indexOf(i18n(KFI_WEIGHT_REGULAR), 0, Qt::CaseInsensitive))
    {
        newStr=str.mid(i18n(KFI_WEIGHT_REGULAR).length());
        return FC_WEIGHT_REGULAR;
    }
    //if(0==str.indexOf(i18n(KFI_WEIGHT_NORMAL), 0, Qt::CaseInsensitive))
    //{
    //    newStr=str.mid(i18n(KFI_WEIGHT_NORMAL).length());
    //    return FC_WEIGHT_NORMAL;
    //}
    if(0==str.indexOf(i18n(KFI_WEIGHT_MEDIUM), 0, Qt::CaseInsensitive))
    {
        newStr=str.mid(i18n(KFI_WEIGHT_MEDIUM).length());
        return FC_WEIGHT_MEDIUM;
    }
    if(0==str.indexOf(i18n(KFI_WEIGHT_DEMIBOLD), 0, Qt::CaseInsensitive))
    {
        newStr=str.mid(i18n(KFI_WEIGHT_DEMIBOLD).length());
        return FC_WEIGHT_DEMIBOLD;
    }
    //if(0==str.indexOf(i18n(KFI_WEIGHT_SEMIBOLD), 0, Qt::CaseInsensitive))
    //{
    //    newStr=str.mid(i18n(KFI_WEIGHT_SEMIBOLD).length());
    //    return FC_WEIGHT_SEMIBOLD;
    //}
    if(0==str.indexOf(i18n(KFI_WEIGHT_BOLD), 0, Qt::CaseInsensitive))
    {
        newStr=str.mid(i18n(KFI_WEIGHT_BOLD).length());
        return FC_WEIGHT_BOLD;
    }
    if(0==str.indexOf(i18n(KFI_WEIGHT_EXTRABOLD), 0, Qt::CaseInsensitive))
    {
        newStr=str.mid(i18n(KFI_WEIGHT_EXTRABOLD).length());
        return FC_WEIGHT_EXTRABOLD;
    }
    //if(0==str.indexOf(i18n(KFI_WEIGHT_ULTRABOLD), 0, Qt::CaseInsensitive))
    //{
    //    newStr=str.mid(i18n(KFI_WEIGHT_ULTRABOLD).length());
    //    return FC_WEIGHT_ULTRABOLD;
    //}
    if(0==str.indexOf(i18n(KFI_WEIGHT_BLACK), 0, Qt::CaseInsensitive))
    {
        newStr=str.mid(i18n(KFI_WEIGHT_BLACK).length());
        return FC_WEIGHT_BLACK;
    }
    if(0==str.indexOf(i18n(KFI_WEIGHT_BLACK), 0, Qt::CaseInsensitive))
    {
        newStr=str.mid(i18n(KFI_WEIGHT_BLACK).length());
        return FC_WEIGHT_BLACK;
    }

    newStr=str;
    return FC_WEIGHT_REGULAR;
}

int strToWidth(const QString &str, QString &newStr)
{
    if(0==str.indexOf(i18n(KFI_WIDTH_ULTRACONDENSED), 0, Qt::CaseInsensitive))
    {
        newStr=str.mid(i18n(KFI_WIDTH_ULTRACONDENSED).length());
        return KFI_FC_WIDTH_ULTRACONDENSED;
    }
    if(0==str.indexOf(i18n(KFI_WIDTH_EXTRACONDENSED), 0, Qt::CaseInsensitive))
    {
        newStr=str.mid(i18n(KFI_WIDTH_EXTRACONDENSED).length());
        return KFI_FC_WIDTH_EXTRACONDENSED;
    }
    if(0==str.indexOf(i18n(KFI_WIDTH_CONDENSED), 0, Qt::CaseInsensitive))
    {
        newStr=str.mid(i18n(KFI_WIDTH_CONDENSED).length());
        return KFI_FC_WIDTH_CONDENSED;
    }
    if(0==str.indexOf(i18n(KFI_WIDTH_SEMICONDENSED), 0, Qt::CaseInsensitive))
    {
        newStr=str.mid(i18n(KFI_WIDTH_SEMICONDENSED).length());
        return KFI_FC_WIDTH_SEMICONDENSED;
    }
    if(0==str.indexOf(i18n(KFI_WIDTH_NORMAL), 0, Qt::CaseInsensitive))
    {
        newStr=str.mid(i18n(KFI_WIDTH_NORMAL).length());
        return KFI_FC_WIDTH_NORMAL;
    }
    if(0==str.indexOf(i18n(KFI_WIDTH_SEMIEXPANDED), 0, Qt::CaseInsensitive))
    {
        newStr=str.mid(i18n(KFI_WIDTH_SEMIEXPANDED).length());
        return KFI_FC_WIDTH_SEMIEXPANDED;
    }
    if(0==str.indexOf(i18n(KFI_WIDTH_EXPANDED), 0, Qt::CaseInsensitive))
    {
        newStr=str.mid(i18n(KFI_WIDTH_EXPANDED).length());
        return KFI_FC_WIDTH_EXPANDED;
    }
    if(0==str.indexOf(i18n(KFI_WIDTH_EXTRAEXPANDED), 0, Qt::CaseInsensitive))
    {
        newStr=str.mid(i18n(KFI_WIDTH_EXTRAEXPANDED).length());
        return KFI_FC_WIDTH_EXTRAEXPANDED;
    }
    if(0==str.indexOf(i18n(KFI_WIDTH_ULTRAEXPANDED), 0, Qt::CaseInsensitive))
    {
        newStr=str.mid(i18n(KFI_WIDTH_ULTRAEXPANDED).length());
        return KFI_FC_WIDTH_ULTRAEXPANDED;
    }

    newStr=str;
    return KFI_FC_WIDTH_NORMAL;
}

int strToSlant(const QString &str)
{
    if(-1!=str.indexOf(i18n(KFI_SLANT_ITALIC)))
        return FC_SLANT_ITALIC;
    if(-1!=str.indexOf(i18n(KFI_SLANT_OBLIQUE)))
        return FC_SLANT_OBLIQUE;
    return FC_SLANT_ROMAN;
}

quint32 createStyleVal(const QString &name)
{
    int pos;

    if(-1==(pos=name.indexOf(", ")))   // No style information...
        return createStyleVal(FC_WEIGHT_REGULAR,
#ifdef KFI_FC_NO_WIDTHS
                              KFI_NULL_SETTING
#else
                              KFI_FC_WIDTH_NORMAL
#endif
                              , FC_SLANT_ROMAN);
    else
    {
        QString style(name.mid(pos+2));

        return createStyleVal(strToWeight(style, style),
#ifdef KFI_FC_NO_WIDTHS
                              KFI_NULL_SETTING
#else
                              strToWidth(style, style)
#endif
                              , strToSlant(style));
    }
}

QString styleValToStr(quint32 style)
{
    QString str;
    int     weight, width, slant;

    decomposeStyleVal(style, weight, width, slant);
    str.sprintf("0X%02X%02X%02X\n", weight, width, slant);
    return str;
}

void decomposeStyleVal(quint32 styleInfo, int &weight, int &width, int &slant)
{
    weight=(styleInfo&0xFF0000)>>16;
    width=(styleInfo&0x00FF00)>>8;
    slant=(styleInfo&0x0000FF);
}

quint32 styleValFromStr(const QString &style)
{
    if(style.isEmpty())
        return KFI_NO_STYLE_INFO;
    else
    {
        quint32 val;

        QTextStream(const_cast<QString *>(&style), QIODevice::ReadOnly) >> val;
        return val;
    }
}

QString getFcString(FcPattern *pat, const char *val, int index)
{
    QString rv;
    FcChar8 *fcStr;

    if(FcResultMatch==FcPatternGetString(pat, val, index, &fcStr))
        rv=QString::fromUtf8((char *)fcStr);

    return rv;
}

//
// TODO: How correct is this?
// Qt/Gtk seem to prefer font family names with FAMILY_LANG=="en"
// ...so if possible, use that. Else, use the first non "xx" lang.
QString getFcLangString(FcPattern *pat, const char *val, const char *valLang)
{
    QString rv;
    int     langIndex=-1;

    for(int i=0; true; ++i)
    {
        QString lang=getFcString(pat, valLang, i);

        if(lang.isEmpty())
            break;
        else if(QString::fromLatin1("en")==lang)
            return getFcString(pat, val, i);
        else if(QString::fromLatin1("xx")!=lang && -1==langIndex)
            langIndex=i;
    }

    return getFcString(pat, val, langIndex>0 ? langIndex : 0);
}

int getFcInt(FcPattern *pat, const char *val, int index, int def)
{
    int rv;

    if (FcResultMatch==FcPatternGetInteger(pat, val, index, &rv))
        return rv;
    return def;
}

QString getName(const QString &file)
{
    int       count=0;
    FcPattern *pat=FcFreeTypeQuery((const FcChar8 *)(QFile::encodeName(file).constData()), 0, nullptr,
                                    &count);
    QString   name(i18n("Unknown"));

    if(pat)
    {
        name=FC::createName(pat);
        FcPatternDestroy(pat);
    }

    return name;
}

void getDetails(FcPattern *pat, QString &family, quint32 &styleVal, int &index, QString &foundry)
{
    int weight=getFcInt(pat,  FC_WEIGHT, 0, KFI_NULL_SETTING),
        width=
#ifdef KFI_FC_NO_WIDTHS
               KFI_NULL_SETTING,
#else
               getFcInt(pat,  FC_WIDTH, 0, KFI_NULL_SETTING),
#endif
        slant=getFcInt(pat,  FC_SLANT, 0, KFI_NULL_SETTING);

    index=getFcInt(pat,  FC_INDEX, 0, 0);
// #ifdef KFI_USE_TRANSLATED_FAMILY_NAME
    family=getFcLangString(pat, FC_FAMILY, FC_FAMILYLANG);
// #else
//     family=getFcString(pat, FC_FAMILY, 0);
// #endif
    styleVal=createStyleVal(weight, width, slant);
    foundry=getFcString(pat, FC_FOUNDRY, 0);
}

QString createName(FcPattern *pat)
{
    QString family,
            foundry;
    quint32 styleVal;
    int     index;
           
    getDetails(pat, family, styleVal, index, foundry);
    return createName(family, styleVal);
}

QString createName(const QString &family, quint32 styleInfo)
{
    int weight, width, slant;

    decomposeStyleVal(styleInfo, weight, width, slant);
    return createName(family, weight, width, slant);
}

QString createName(const QString &family, int weight, int width, int slant)
{
    return family+QString::fromLatin1(", ")+createStyleName(weight, width, slant);
}

QString createStyleName(quint32 styleInfo)
{
    int weight, width, slant;

    decomposeStyleVal(styleInfo, weight, width, slant);
    return createStyleName(weight, width, slant);
}

QString createStyleName(int weight, int width, int slant)
{
//
//CPD: TODO: the names *need* to match up with kfontchooser's...
//         : Removing KFI_DISPLAY_OBLIQUE and KFI_DISPLAY_MEDIUM help this.
//           However, I have at least one bad font:
//               Rockwell Extra Bold. Both fontconfig, and kcmshell fonts list family
//               as "Rockwell Extra Bold" -- good (well at least they match). *But* fontconfig
//               is returning the weight "Extra Bold", and kcmshell fonts is using "Bold" :-(
//
    QString name,
            weightString,
            widthString,
            slantString;

#ifndef KFI_FC_NO_WIDTHS
    if(KFI_NULL_SETTING!=width)
        widthString=widthStr(width);
#endif

    if(KFI_NULL_SETTING!=slant)
        slantString=slantStr(slant);

    //
    // If weight is "Regular", we only want to display it if slant and width are empty.
    if(KFI_NULL_SETTING!=weight)
    {
        weightString=weightStr(weight, !slantString.isEmpty() || !widthString.isEmpty());

        if(!weightString.isEmpty())
            name=weightString;
    }

#ifndef KFI_FC_NO_WIDTHS
    if(!widthString.isEmpty())
    {
        if(!name.isEmpty())
            name+=QChar(' ');
        name+=widthString;
    }
#endif

    if(!slantString.isEmpty())
    {
        if(!name.isEmpty())
            name+=QChar(' ');
        name+=slantString;
    }
    return name;
}

QString weightStr(int w, bool emptyNormal)
{
    switch(weight(w))
    {
        case FC_WEIGHT_THIN:
            return i18n(KFI_WEIGHT_THIN);
        case FC_WEIGHT_EXTRALIGHT:
            return i18n(KFI_WEIGHT_EXTRALIGHT);
        case FC_WEIGHT_LIGHT:
            return i18n(KFI_WEIGHT_LIGHT);
        case FC_WEIGHT_MEDIUM:
#ifdef KFI_DISPLAY_MEDIUM
            return i18n(KFI_WEIGHT_MEDIUM);
#endif
        case FC_WEIGHT_REGULAR:
            return emptyNormal ? QString() : i18n(KFI_WEIGHT_REGULAR);
        case FC_WEIGHT_DEMIBOLD:
            return i18n(KFI_WEIGHT_DEMIBOLD);
        case FC_WEIGHT_BOLD:
            return i18n(KFI_WEIGHT_BOLD);
        case FC_WEIGHT_EXTRABOLD:
            return i18n(KFI_WEIGHT_EXTRABOLD);
        default:
            return i18n(KFI_WEIGHT_BLACK);
    }
}

QString widthStr(int w, bool emptyNormal)
{
    switch(width(w))
    {
        case KFI_FC_WIDTH_ULTRACONDENSED:
            return i18n(KFI_WIDTH_ULTRACONDENSED);
        case KFI_FC_WIDTH_EXTRACONDENSED:
            return i18n(KFI_WIDTH_EXTRACONDENSED);
        case KFI_FC_WIDTH_CONDENSED:
            return i18n(KFI_WIDTH_CONDENSED);
        case KFI_FC_WIDTH_SEMICONDENSED:
            return i18n(KFI_WIDTH_SEMICONDENSED);
        case KFI_FC_WIDTH_NORMAL:
            return emptyNormal ? QString() : i18n(KFI_WIDTH_NORMAL);
        case KFI_FC_WIDTH_SEMIEXPANDED:
            return i18n(KFI_WIDTH_SEMIEXPANDED);
        case KFI_FC_WIDTH_EXPANDED:
            return i18n(KFI_WIDTH_EXPANDED);
        case KFI_FC_WIDTH_EXTRAEXPANDED:
            return i18n(KFI_WIDTH_EXTRAEXPANDED);
        default:
            return i18n(KFI_WIDTH_ULTRAEXPANDED);
    }
}

QString slantStr(int s, bool emptyNormal)
{
    switch(slant(s))
    {
        case FC_SLANT_OBLIQUE:
#ifdef KFI_DISPLAY_OBLIQUE
            return i18n(KFI_SLANT_OBLIQUE);
#endif
        case FC_SLANT_ITALIC:
            return i18n(KFI_SLANT_ITALIC);
        default:
            return emptyNormal ? QString() : i18n(KFI_SLANT_ROMAN);
    }
}

QString spacingStr(int s)
{
    switch(spacing(s))
    {
        case FC_MONO:
            return i18n(KFI_SPACING_MONO);
        case FC_CHARCELL:
            return i18n(KFI_SPACING_CHARCELL);
        default:
            return i18n(KFI_SPACING_PROPORTIONAL);
    }
}

bool bitmapsEnabled()
{
    //
    // On some systems, such as KUbuntu, fontconfig is configured to ignore all bitmap fonts.
    // The following check tries to get a list of installed bitmaps, if it an empty list is returned
    // it is assumed that bitmaps are disabled.

    static bool enabled(false);
    static bool checked(false); // Do not keep on checking!

    if(!checked)
    {
        FcObjectSet *os  = FcObjectSetBuild(FC_FAMILY, (void *)nullptr);
        FcPattern   *pat = FcPatternBuild(nullptr, FC_SCALABLE, FcTypeBool, FcFalse, NULL);
        FcFontSet   *set = FcFontList(nullptr, pat, os);

        FcPatternDestroy(pat);
        FcObjectSetDestroy(os);

        if (set)
        {
            if(set->nfont)
                enabled=true;

            FcFontSetDestroy(set);
        }
        checked=true;
    }

    return enabled;
}

}

}
