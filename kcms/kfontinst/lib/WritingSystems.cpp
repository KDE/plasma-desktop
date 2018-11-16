/*
 * KFontInst - KDE Font Installer
 *
 * Copyright 2003-2009 Craig Drummond <craig@kde.org>
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

#include <QGlobalStatic>
#include <QFontDatabase>
#include <fontconfig/fontconfig.h>
#include "WritingSystems.h"

namespace KFI
{

Q_GLOBAL_STATIC(WritingSystems, theInstance)

WritingSystems * WritingSystems::instance()
{
    return theInstance;
}

static const struct
{
    QFontDatabase::WritingSystem ws;
    const FcChar8                *lang;
} constLanguageForWritingSystem[]=
{
    { QFontDatabase::Latin, (const FcChar8 *)"en" },
    { QFontDatabase::Greek, (const FcChar8 *)"el" },
    { QFontDatabase::Cyrillic, (const FcChar8 *)"ru" },
    { QFontDatabase::Armenian, (const FcChar8 *)"hy" },
    { QFontDatabase::Hebrew, (const FcChar8 *)"he" },
    { QFontDatabase::Arabic, (const FcChar8 *)"ar" },
    { QFontDatabase::Syriac, (const FcChar8 *)"syr" },
    { QFontDatabase::Thaana, (const FcChar8 *)"div" },
    { QFontDatabase::Devanagari, (const FcChar8 *)"hi" },
    { QFontDatabase::Bengali, (const FcChar8 *)"bn" },
    { QFontDatabase::Gurmukhi, (const FcChar8 *)"pa" },
    { QFontDatabase::Gujarati, (const FcChar8 *)"gu" },
    { QFontDatabase::Oriya, (const FcChar8 *)"or" },
    { QFontDatabase::Tamil, (const FcChar8 *)"ta" },
    { QFontDatabase::Telugu, (const FcChar8 *)"te" },
    { QFontDatabase::Kannada, (const FcChar8 *)"kn" },
    { QFontDatabase::Malayalam, (const FcChar8 *)"ml" },
    { QFontDatabase::Sinhala, (const FcChar8 *)"si" },
    { QFontDatabase::Thai, (const FcChar8 *)"th" },
    { QFontDatabase::Lao, (const FcChar8 *)"lo" },
    { QFontDatabase::Tibetan, (const FcChar8 *)"bo" },
    { QFontDatabase::Myanmar, (const FcChar8 *)"my" },
    { QFontDatabase::Georgian, (const FcChar8 *)"ka" },
    { QFontDatabase::Khmer, (const FcChar8 *)"km" },
    { QFontDatabase::SimplifiedChinese, (const FcChar8 *)"zh-cn" },
    { QFontDatabase::TraditionalChinese, (const FcChar8 *)"zh-tw" },
    { QFontDatabase::Japanese, (const FcChar8 *)"ja" },
    { QFontDatabase::Korean, (const FcChar8 *)"ko" },
    { QFontDatabase::Vietnamese, (const FcChar8 *)"vi" },
    { QFontDatabase::Other, nullptr },

    // The following is only used to save writing system data for disabled fonts...
    { QFontDatabase::Telugu, (const FcChar8 *)"Qt-Telugu" },
    { QFontDatabase::Kannada, (const FcChar8 *)"Qt-Kannada" },
    { QFontDatabase::Malayalam, (const FcChar8 *)"Qt-Malayalam" },
    { QFontDatabase::Sinhala, (const FcChar8 *)"Qt-Sinhala" },
    { QFontDatabase::Myanmar, (const FcChar8 *)"Qt-Myanmar" },
    { QFontDatabase::Ogham, (const FcChar8 *)"Qt-Ogham" },
    { QFontDatabase::Runic, (const FcChar8 *)"Qt-Runic" },

    { QFontDatabase::Any, nullptr }
};


inline qulonglong toBit(QFontDatabase::WritingSystem ws)
{
    return ((qulonglong)1) << (int)ws;
}

//.........the following section is inspired by qfontdatabase_x11.cpp / loadFontConfig

// Unfortunately FontConfig doesn't know about some languages. We have to test these through the
// charset. The lists below contain the systems where we need to do this.
static const struct
{
    QFontDatabase::WritingSystem ws;
    ushort                       ch;
} sampleCharForWritingSystem[] =
{
    { QFontDatabase::Telugu, 0xc15 },
    { QFontDatabase::Kannada, 0xc95 },
    { QFontDatabase::Malayalam, 0xd15 },
    { QFontDatabase::Sinhala, 0xd9a },
    { QFontDatabase::Myanmar, 0x1000 },
    { QFontDatabase::Ogham, 0x1681 },
    { QFontDatabase::Runic, 0x16a0 },
    { QFontDatabase::Any, 0x0 }
};

qulonglong WritingSystems::get(FcPattern *pat) const
{
    qulonglong ws(0);
    FcLangSet  *langset(nullptr);

    if (FcResultMatch==FcPatternGetLangSet(pat, FC_LANG, 0, &langset))
    {
        for (int i = 0; constLanguageForWritingSystem[i].lang; ++i)
            if (FcLangDifferentLang!=FcLangSetHasLang(langset, constLanguageForWritingSystem[i].lang))
                ws|=toBit(constLanguageForWritingSystem[i].ws);
    }
    else
        ws|=toBit(QFontDatabase::Other);

    FcCharSet *cs(nullptr);

    if (FcResultMatch == FcPatternGetCharSet(pat, FC_CHARSET, 0, &cs))
    {
        // some languages are not supported by FontConfig, we rather check the
        // charset to detect these
        for (int i = 0; QFontDatabase::Any!=sampleCharForWritingSystem[i].ws; ++i)
            if (FcCharSetHasChar(cs, sampleCharForWritingSystem[i].ch))
                ws|=toBit(sampleCharForWritingSystem[i].ws);
    }

    return ws;
}
//.........

qulonglong WritingSystems::get(const QStringList &langs) const
{
    qulonglong ws(0);

    QStringList::ConstIterator it(langs.begin()),
                               end(langs.end());

    for(; it!=end; ++it)
        ws|=itsMap[*it];

    return ws;
}

QStringList WritingSystems::getLangs(qulonglong ws) const
{
    QMap<QString, qulonglong>::ConstIterator wit(itsMap.begin()),
                                             wend(itsMap.end());
    QStringList                              systems;

    for(; wit!=wend; ++wit)
        if(ws&wit.value())
            systems+=wit.key();
    return systems;
}

WritingSystems::WritingSystems()
{
    for(int i=0; QFontDatabase::Any!=constLanguageForWritingSystem[i].ws; ++i)
        if(constLanguageForWritingSystem[i].lang)
            itsMap[(const char *)constLanguageForWritingSystem[i].lang]=
                ((qulonglong)1)<<constLanguageForWritingSystem[i].ws;
}

}
