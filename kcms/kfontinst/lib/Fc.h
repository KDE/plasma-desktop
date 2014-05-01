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

#ifndef __FC_H__
#define __FC_H__

#include <QUrl>
#include <fontconfig/fontconfig.h>
#include "kfontinst_export.h"
#include "KfiConstants.h"
#include "Misc.h"

#if (FC_VERSION<20200)

#define KFI_FC_NO_WIDTHS
#define KFI_FC_LIMITED_WEIGHTS

#endif

#ifdef KFI_FC_LIMITED_WEIGHTS

#undef FC_WEIGHT_LIGHT
#define FC_WEIGHT_THIN              0
#define FC_WEIGHT_EXTRALIGHT        40
#define FC_WEIGHT_ULTRALIGHT        FC_WEIGHT_EXTRALIGHT
#define FC_WEIGHT_LIGHT             50
#define FC_WEIGHT_BOOK              75
#define FC_WEIGHT_REGULAR           80
#define FC_WEIGHT_NORMAL            FC_WEIGHT_REGULAR
#define FC_WEIGHT_SEMIBOLD          FC_WEIGHT_DEMIBOLD
#define FC_WEIGHT_EXTRABOLD         205
#define FC_WEIGHT_ULTRABOLD         FC_WEIGHT_EXTRABOLD
#define FC_WEIGHT_HEAVY             FC_WEIGHT_BLACK

#endif

class QString;

namespace KFI
{
namespace FC
{
//
// Ideally only want this class to contain KFI_FC_NO_WIDTHS
#ifdef KFI_FC_NO_WIDTHS
#define KFI_FC_WIDTH_ULTRACONDENSED     50
#define KFI_FC_WIDTH_EXTRACONDENSED     63
#define KFI_FC_WIDTH_CONDENSED          75
#define KFI_FC_WIDTH_SEMICONDENSED      87
#define KFI_FC_WIDTH_NORMAL             100
#define KFI_FC_WIDTH_SEMIEXPANDED       113
#define KFI_FC_WIDTH_EXPANDED           125
#define KFI_FC_WIDTH_EXTRAEXPANDED      150
#define KFI_FC_WIDTH_ULTRAEXPANDED      200
#else
#define KFI_FC_WIDTH_ULTRACONDENSED     FC_WIDTH_ULTRACONDENSED
#define KFI_FC_WIDTH_EXTRACONDENSED     FC_WIDTH_EXTRACONDENSED
#define KFI_FC_WIDTH_CONDENSED          FC_WIDTH_CONDENSED
#define KFI_FC_WIDTH_SEMICONDENSED      FC_WIDTH_SEMICONDENSED
#define KFI_FC_WIDTH_NORMAL             FC_WIDTH_NORMAL
#define KFI_FC_WIDTH_SEMIEXPANDED       FC_WIDTH_SEMIEXPANDED
#define KFI_FC_WIDTH_EXPANDED           FC_WIDTH_EXPANDED
#define KFI_FC_WIDTH_EXTRAEXPANDED      FC_WIDTH_EXTRAEXPANDED
#define KFI_FC_WIDTH_ULTRAEXPANDED      FC_WIDTH_ULTRAEXPANDED
#endif

    extern Q_DECL_EXPORT QUrl        encode(const QString &name, quint32 style, const QString &file=QString(), int index=0);
    extern Q_DECL_EXPORT Misc::TFont decode(const QUrl &url);
    extern Q_DECL_EXPORT QString     getFile(const QUrl &url);
    extern Q_DECL_EXPORT int         getIndex(const QUrl &url);
    extern Q_DECL_EXPORT int     weight(int w); // round w to nearest fc weight
    extern Q_DECL_EXPORT int     width(int w); // round w to nearest fc width
    extern Q_DECL_EXPORT int     slant(int s); // round s to nearest fc slant
    extern Q_DECL_EXPORT int     spacing(int s); // round s to nearest fc spacing
    extern Q_DECL_EXPORT int     strToWeight(const QString &str, QString &newStr);
    extern Q_DECL_EXPORT int     strToWidth(const QString &str, QString &newStr);
    extern Q_DECL_EXPORT int     strToSlant(const QString &str);
    extern Q_DECL_EXPORT quint32 createStyleVal(const QString &name);
    inline Q_DECL_EXPORT quint32 createStyleVal(int weight, int width, int slant)
                            { return ((weight&0xFF)<<16)+((width&0xFF)<<8)+(slant&0xFF); }
    extern Q_DECL_EXPORT QString styleValToStr(quint32 style);
    extern Q_DECL_EXPORT void    decomposeStyleVal(quint32 styleInfo, int &weight, int &width, int &slant);
    extern Q_DECL_EXPORT quint32 styleValFromStr(const QString &style);

    extern Q_DECL_EXPORT QString getFcString(FcPattern *pat, const char *val, int index=0);
#ifdef KFI_USE_TRANSLATED_FAMILY_NAME
    extern Q_DECL_EXPORT QString getFcLangString(FcPattern *pat, const char *val, const char *valLang);
#endif
    extern Q_DECL_EXPORT int     getFcInt(FcPattern *pat, const char *val, int index=0, int def=-1);
    extern Q_DECL_EXPORT QString getName(const QString &file);
    extern Q_DECL_EXPORT void    getDetails(FcPattern *pat, QString &family, quint32 &styleVal, int &index, QString &foundry);
    extern Q_DECL_EXPORT QString createName(FcPattern *pat);
    extern Q_DECL_EXPORT QString createName(const QString &family, quint32 styleInfo);
    extern Q_DECL_EXPORT QString createName(const QString &family, int weight, int width, int slant);
    inline Q_DECL_EXPORT QString createName(const Misc::TFont &font) { return createName(font.family, font.styleInfo); }
    extern Q_DECL_EXPORT QString createStyleName(quint32 styleInfo);
    extern Q_DECL_EXPORT QString createStyleName(int weight, int width, int slant);
    extern Q_DECL_EXPORT QString weightStr(int w, bool emptyNormal=true);
    extern Q_DECL_EXPORT QString widthStr(int w, bool emptyNormal=true);
    extern Q_DECL_EXPORT QString slantStr(int s, bool emptyNormal=true);
    extern Q_DECL_EXPORT QString spacingStr(int s);
    extern Q_DECL_EXPORT bool    bitmapsEnabled();
}
}

#endif
