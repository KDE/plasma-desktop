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

#include "Misc.h"
#include "config-paths.h"
#include <QSet>
#include <QMap>
#include <QVector>
#include <QDir>
#include <QFile>
#include <QByteArray>
#include <QTextCodec>
#include <QTextStream>
#include <QProcess>
#include <QTemporaryFile>
#include <QStandardPaths>
#include <QUrlQuery>
#include <unistd.h>
#include <ctype.h>

namespace KFI
{

namespace Misc
{

QString prettyUrl(const QUrl &url)
{
    QString u(url.url());

    u.replace("%20", " ");
    return u;
}

QString dirSyntax(const QString &d)
{
    if(!d.isEmpty())
    {
        QString ds(d);

        ds.replace("//", "/");

        int slashPos(ds.lastIndexOf('/'));

        if(slashPos!=(((int)ds.length())-1))
            ds.append('/');

        return ds;
    }

    return d;
}

QString fileSyntax(const QString &d)
{
    if(!d.isEmpty())
    {
        QString ds(d);

        ds.replace("//", "/");

        int slashPos(ds.lastIndexOf('/'));

        if(slashPos==(((int)ds.length())-1))
            ds.remove(slashPos, 1);
        return ds;
    }

    return d;
}

QString getDir(const QString &f)
{
    QString d(f);

    int slashPos(d.lastIndexOf('/'));
 
    if(slashPos!=-1)
        d.remove(slashPos+1, d.length());

    return dirSyntax(d);
}

QString getFile(const QString &f)
{
    QString d(f);

    int slashPos=d.lastIndexOf('/');
 
    if(slashPos!=-1)
        d.remove(0, slashPos+1);

    return d;
}

bool createDir(const QString &dir)
{
    if (!QDir().mkpath(dir))
        return false;
    //
    // Clear any umask before setting dir perms
    mode_t oldMask(umask(0000));
    const QByteArray d = QFile::encodeName(dir);
    ::chmod(d.constData(), DIR_PERMS);
    // Reset umask
    ::umask(oldMask);
    return true;
}

void setFilePerms(const QByteArray &f)
{
    //
    // Clear any umask before setting file perms
    mode_t oldMask(umask(0000));
    ::chmod(f.constData(), FILE_PERMS);
    // Reset umask
    ::umask(oldMask);
}

bool doCmd(const QString &cmd, const QString &p1, const QString &p2, const QString &p3)
{
    QStringList args;

    if(!p1.isEmpty())
        args << p1;
    if(!p2.isEmpty())
        args << p2;
    if(!p3.isEmpty())
        args << p3;

    return 0==QProcess::execute(cmd, args);
}

QString changeExt(const QString &f, const QString &newExt)
{
    QString newStr(f);
    int     dotPos(newStr.lastIndexOf('.'));

    if(-1==dotPos)
        newStr+=QChar('.')+newExt;
    else
    {
        newStr.remove(dotPos+1, newStr.length());
        newStr+=newExt;
    }
    return newStr;
}

//
// Get a list of files associated with a file, e.g.:
//
//    File: /home/a/courier.pfa
//
//    Associated: /home/a/courier.afm /home/a/courier.pfm
//
void getAssociatedFiles(const QString &path, QStringList &files, bool afmAndPfm)
{
    QString ext(path);
    int     dotPos(ext.lastIndexOf('.'));
    bool    check(false);

    if(-1==dotPos) // Hmm, no extension - check anyway...
        check=true;
    else           // Cool, got an extension - see if it is a Type1 font...
    {
        ext=ext.mid(dotPos+1);
        check=0==ext.compare("pfa", Qt::CaseInsensitive) ||
              0==ext.compare("pfb", Qt::CaseInsensitive);
    }

    if(check)
    {
        const char *afm[]={"afm", "AFM", "Afm", nullptr},
                   *pfm[]={"pfm", "PFM", "Pfm", nullptr};
        bool       gotAfm(false);
        int        e;

        for(e=0; afm[e]; ++e)
        {
            QString statFile(changeExt(path, afm[e]));

            if(fExists(statFile))
            {
                files.append(statFile);
                gotAfm=true;
                break;
            }
        }

        if(afmAndPfm || !gotAfm)
            for(e=0; pfm[e]; ++e)
            {
                QString statFile(changeExt(path, pfm[e]));

                if(fExists(statFile))
                {
                    files.append(statFile);
                    break;
                }
            }
    }
}

time_t getTimeStamp(const QString &item)
{
    QT_STATBUF info;

    return !item.isEmpty() && 0==QT_LSTAT(QFile::encodeName(item), &info) ? info.st_mtime : 0;
}


bool check(const QString &path, bool file, bool checkW)
{ 
    QT_STATBUF info;
    QByteArray      pathC(QFile::encodeName(path));

    return 0==QT_LSTAT(pathC, &info) &&
           (file ? (S_ISREG(info.st_mode) || S_ISLNK(info.st_mode))
                 : S_ISDIR(info.st_mode)) &&
           (!checkW || 0==::access(pathC, W_OK));
}

QString getFolder(const QString &defaultDir, const QString &root, QStringList &dirs)
{
    if(dirs.contains(defaultDir))
        return defaultDir;
    else
    {
        QStringList::const_iterator it,
                              end=dirs.constEnd();
        bool                  found=false;

        for(it=dirs.constBegin(); it!=end && !found; ++it)
            if(0==(*it).indexOf(root))
                return *it;
    }

    return defaultDir;
}

bool checkExt(const QString &fname, const QString &ext)
{
    QString extension('.'+ext);

    return fname.length()>extension.length()
            ? 0==fname.mid(fname.length()-extension.length()).compare(extension, Qt::CaseInsensitive)
            : false;
}

bool isBitmap(const QString &str)
{
    return checkExt(str, "pcf") || checkExt(str, "bdf") || checkExt(str, "pcf.gz") || checkExt(str, "bdf.gz");
}

bool isMetrics(const QString &str)
{
    return checkExt(str, "afm") || checkExt(str, "pfm");
}

int getIntQueryVal(const QUrl &url, const char *key, int defVal)
{
    QUrlQuery query(url);
    QString item(query.queryItemValue(key));
    int     val(defVal);

    if(!item.isNull())
        val=item.toInt();

    return val;
}

bool printable(const QString &mime)
{
    return "font/otf"==mime || "font/ttf"==mime ||
           "application/x-font-type1"==mime || "application/x-font-ttf"==mime ||
           "application/x-font-otf"==mime || "application/x-font-type1"==mime;
}

uint qHash(const KFI::Misc::TFont &key)
{
    //return qHash(QString(key.family+'%'+QString().setNum(key.styleInfo, 16)));
    const QChar *p = key.family.unicode();
    int         n = key.family.size();
    uint        h = 0,
                g;

    h = (h << 4) + key.styleInfo;
    if ((g = (h & 0xf0000000)) != 0)
        h ^= g >> 23;
    h &= ~g;

    while (n--)
    {
        h = (h << 4) + (*p++).unicode();
        if ((g = (h & 0xf0000000)) != 0)
            h ^= g >> 23;
        h &= ~g;
    }
    return h;
}

// Taken from qdom.cpp
QString encodeText(const QString &str, QTextStream &s)
{
    const QTextCodec *const codec = s.codec();

    Q_ASSERT(codec);

    QString retval(str);
    int     len = retval.length(),
            i = 0;

    while (i < len)
    {
        const QChar ati(retval.at(i));

        if (ati == QLatin1Char('<'))
        {
            retval.replace(i, 1, QLatin1String("&lt;"));
            len += 3;
            i += 4;
        }
        else if (ati == QLatin1Char('"'))
        {
            retval.replace(i, 1, QLatin1String("&quot;"));
            len += 5;
            i += 6;
        }
        else if (ati == QLatin1Char('&'))
        {
            retval.replace(i, 1, QLatin1String("&amp;"));
            len += 4;
            i += 5;
        }
        else if (ati == QLatin1Char('>') && i >= 2 && retval[i - 1] == QLatin1Char(']') && retval[i - 2] == QLatin1Char(']'))
        {
            retval.replace(i, 1, QLatin1String("&gt;"));
            len += 3;
            i += 4;
        }
        else
        {
            if(codec->canEncode(ati))
                ++i;
            else
            {
                // We have to use a character reference to get it through.
                const ushort codepoint(ati.unicode());
                const QString replacement(QLatin1String("&#x") + QString::number(codepoint, 16) + QLatin1Char(';'));
                retval.replace(i, 1, replacement);
                i += replacement.length();
                len += replacement.length() - 1;
            }
        }
    }

    return retval;
}

QString contractHome(QString path)
{
    if (!path.isEmpty() && '/'==path[0])
    {
        QString home(QDir::homePath());

        if(path.startsWith(home))
        {
            int len = home.length();

            if(len>1 && (path.length() == len || path[len] == '/'))
                return path.replace(0, len, QString::fromLatin1("~"));
        }
    }

    return path;
}

QString expandHome(QString path)
{
    if(!path.isEmpty() && '~'==path[0])
        return 1==path.length() ? QDir::homePath() : path.replace(0, 1, QDir::homePath());

    return path;
}

QMap<QString, QString> getFontFileMap(const QSet<QString> &files)
{
    QMap<QString, QString>        map;
    QSet<QString>::ConstIterator  it=files.constBegin(),
                                  end=files.constEnd();
    QMap<QString, QSet<QString> > fontsFiles;

    for(;it!=end; ++it)
        fontsFiles[unhide(getFile(*it))].insert(getDir(*it));

    QMap<QString, QSet<QString> >::ConstIterator fIt(fontsFiles.constBegin()),
                                                 fEnd(fontsFiles.constEnd());

    for(; fIt!=fEnd; ++fIt)
        if(fIt.value().count()>1)
        {
            QVector<QString>             orig(fIt.value().count()),
                                         modified(fIt.value().count());
            QSet<QString>::ConstIterator oIt(fIt.value().constBegin()),
                                         oEnd(fIt.value().constEnd());
            bool                         good=true;
            int                          count=fIt.value().count();

            for(int i=0;  i<count && good; ++i, ++oIt)
                orig[i]=modified[i]=*oIt;
            
            while(good)
            {
                int end=modified[0].indexOf('/', 1);
                
                if(-1!=end)
                {
                    QString dir=modified[0].left(end);
                
                    for(int i=1;  i<count && good; ++i)
                        if(0!=modified[i].indexOf(dir))
                            good=false;
                    if(good)
                        for(int i=0;  i<count && good; ++i)
                            modified[i].remove(0, dir.length());
                }
                else
                    good=false;
            }
            for(int i=0;  i<count; ++i)
                map[getDir(modified[i]).mid(1)+fIt.key()]=fExists(orig[i]+fIt.key())
                                                                    ? orig[i]+fIt.key()
                                                                    : orig[i]+hide(fIt.key());
        }
        else // Only 1 entry! :-)
            map[unhide(fIt.key())]=fExists((*fIt.value().begin())+fIt.key())
                                    ? (*fIt.value().begin())+fIt.key()
                                    : (*fIt.value().begin())+hide(fIt.key());

    return map;
}

QString modifyName(const QString &fname)
{
    static const char constSymbols[]={ '-', ' ', ':', ';', '/', '~', 0 };

    QString rv(fname);

    for(int s=0; constSymbols[s]; ++s)
        rv.replace(constSymbols[s], '_');

    int dotPos(rv.lastIndexOf('.'));

    return -1==dotPos
            ? rv
            : rv.left(dotPos+1)+rv.mid(dotPos+1).toLower();
}

QString app(const QString &name, const char *path)
{
    static QMap<QString, QString> apps;
    
    if(!apps.contains(name)) {
        QStringList installPaths;
        if (qstrcmp(path, "libexec") == 0)
            installPaths.append(KFONTINST_LIBEXEC_DIR);
        apps[name] = QStandardPaths::findExecutable(name, installPaths);
    }
    return apps[name];
}

} // Misc::

} // KFI::
