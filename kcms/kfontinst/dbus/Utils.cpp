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

#include <QFile>
#include <QByteArray>
#include <QTextStream>
#include <KShell>
#include <fontconfig/fontconfig.h>
#include "Utils.h"
#include "FontInst.h"
#include "Misc.h"
#include "Fc.h"
#include "WritingSystems.h"

namespace KFI
{

namespace Utils
{

bool isAAfm(const QString &fname)
{
    if(Misc::checkExt(QFile::encodeName(fname), "afm"))   // CPD? Is this a necessary check?
    {
        QFile file(fname);

        if(file.open(QIODevice::ReadOnly))
        {
            QTextStream stream(&file);
            QString     line;

            for(int lc=0; lc<30 && !stream.atEnd(); ++lc)
            {
                line=stream.readLine();

                if(line.contains("StartFontMetrics"))
                {
                    file.close();
                    return true;
                }
            }

            file.close();
        }
    }

    return false;
}

bool isAPfm(const QString &fname)
{
    bool ok=false;

    // I know extension checking is bad, but Ghostscript's pf2afm requires the pfm file to
    // have the .pfm extension...
    QByteArray name(QFile::encodeName(fname));

    if(Misc::checkExt(name, "pfm"))
    {
        //
        // OK, the extension matches, so perform a little contents checking...
        FILE *f=fopen(name.constData(), "r");

        if(f)
        {
            static const unsigned long constCopyrightLen =  60;
            static const unsigned long constTypeToExt    =  49;
            static const unsigned long constExtToFname   =  20;
            static const unsigned long constExtLen       =  30;
            static const unsigned long constFontnameMin  =  75;
            static const unsigned long constFontnameMax  = 512;

            unsigned short version=0,
                           type=0,
                           extlen=0;
            unsigned long  length=0,
                           fontname=0,
                           fLength=0;

            fseek(f, 0, SEEK_END);
            fLength=ftell(f);
            fseek(f, 0, SEEK_SET);

            if(2==fread(&version, 1, 2, f) && // Read version
               4==fread(&length, 1, 4, f) &&  // length...
               length==fLength &&
               0==fseek(f, constCopyrightLen, SEEK_CUR) &&   // Skip copyright notice...
               2==fread(&type, 1, 2, f) &&
               0==fseek(f, constTypeToExt, SEEK_CUR) &&
               2==fread(&extlen, 1, 2, f) &&
               extlen==constExtLen &&
               0==fseek(f, constExtToFname, SEEK_CUR) &&
               4==fread(&fontname, 1, 4, f) &&
               fontname>constFontnameMin && fontname<constFontnameMax)
                ok=true;
            fclose(f);
        }
    }

    return ok;
}

// This function is *only* used for the generation of AFMs from PFMs.
bool isAType1(const QString &fname)
{
    static const char         constStr[]="%!PS-AdobeFont-";
    static const unsigned int constStrLen=15;
    static const unsigned int constPfbOffset=6;
    static const unsigned int constPfbLen=constStrLen+constPfbOffset;

    QByteArray name(QFile::encodeName(fname));
    char       buffer[constPfbLen];
    bool       match=false;

    if(Misc::checkExt(name, "pfa"))
    {
        FILE *f=fopen(name.constData(), "r");

        if(f)
        {
            if(constStrLen==fread(buffer, 1, constStrLen, f))
                match=0==memcmp(buffer, constStr, constStrLen);
            fclose(f);
        }
    }
    else if(Misc::checkExt(name, "pfb"))
    {
        static const char constPfbMarker = static_cast<char>(0x80);

        FILE *f=fopen(name.constData(), "r");

        if(f)
        {
            if(constPfbLen==fread(buffer, 1, constPfbLen, f))
                match=buffer[0]==constPfbMarker && 0==memcmp(&buffer[constPfbOffset], constStr,
                                                             constStrLen);
            fclose(f);
        }
    }

    return match;
}

static QString getMatch(const QString &file, const char *extension)
{
    QString f(Misc::changeExt(file, extension));

    return Misc::fExists(f) ? f : QString();
}

void createAfm(const QString &file, EFileType type)
{
    bool pfm=FILE_PFM==type,
         type1=FILE_SCALABLE==type && isAType1(file);

    if(type1 || pfm)
    {
        // pf2afm wants files with lowercase extension, so just check for lowercase!
        // -- when a font is installed, the extension is converted to lowercase anyway...
        QString afm=getMatch(file, "afm");

        if(afm.isEmpty())  // No point creating if AFM already exists!
        {
            QString pfm,
                    t1;

            if(type1)      // Its a Type1, so look for existing PFM
            {
                pfm=getMatch(file, "pfm");
                t1=file;
            }
            else           // Its a PFM, so look for existing Type1
            {
                t1=getMatch(file, "pfa");
                if(t1.isEmpty())
                    t1=getMatch(file, "pfb");
                pfm=file;
            }

            if(!t1.isEmpty() && !pfm.isEmpty())         // Do we have both Type1 and PFM?
            {
                QString rootName(t1.left(t1.length()-4));
                Misc::doCmd("pf2afm", KShell::quoteArg(rootName)); // pf2afm wants name without extension...
                Misc::setFilePerms(QFile::encodeName(rootName+".afm"));
            }
        }
    }
}

EFileType check(const QString &file, Family &fam)
{
    if(isAAfm(file))
        return FILE_AFM;
    else if(isAPfm(file))
        return FILE_PFM;
    else
    {
        // Check that file is a font via FreeType...
        int       count=0;
        FcPattern *pat=FcFreeTypeQuery((const FcChar8 *)(QFile::encodeName(file).constData()), 0, nullptr,
                                       &count);

        if(pat)
        {
            FcBool     scalable;
            QString    family,
                       foundry;
            quint32    style;
            int        index;
            qulonglong ws;
            EFileType  type=(FcResultMatch!=FcPatternGetBool(pat, FC_SCALABLE, 0, &scalable) || !scalable)
                                ? FILE_BITMAP : FILE_SCALABLE;

            FC::getDetails(pat, family, style, index, foundry);
            ws=WritingSystems::instance()->get(pat);
            FcPatternDestroy(pat);
            Style st(style, scalable, ws);
            st.add(File(file, foundry, index));
            fam=Family(family);
            fam.add(st);
            return type;
        }
    }
    return FILE_INVALID;
}

}

}
