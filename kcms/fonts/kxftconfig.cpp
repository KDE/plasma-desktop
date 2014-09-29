/*
   Copyright (c) 2002 Craig Drummond <craig@kde.org>

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

#include "kxftconfig.h"
#ifdef HAVE_FONTCONFIG

#include <stdarg.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <sys/stat.h>

#include <QRegExp>
#include <QFile>
#include <QDir>
#include <QX11Info>
#include <QByteArray>

#include <KLocalizedString>
#include <kde_file.h>
#include <KGlobal>
#include <KStandardDirs>
#include <KDebug>

#include <fontconfig/fontconfig.h>

using namespace std;

static int point2Pixel(double point)
{
    return (int)(((point * QX11Info::appDpiY()) / 72.0) + 0.5);
}

static int pixel2Point(double pixel)
{
    return (int)(((pixel * 72.0) / (double)QX11Info::appDpiY()) + 0.5);
}

static bool equal(double d1, double d2)
{
    return (fabs(d1 - d2) < 0.0001);
}

static QString dirSyntax(const QString &d)
{
    if (!d.isNull()) {
        QString ds(d);

        ds.replace("//", "/");

        int slashPos = ds.lastIndexOf('/');

        if (slashPos != (((int)ds.length()) - 1)) {
            ds.append('/');
        }

        return ds;
    }

    return d;
}

static bool check(const QString &path, unsigned int fmt, bool checkW = false)
{
    KDE_struct_stat info;
    QByteArray        pathC(QFile::encodeName(path));

    return 0 == KDE_lstat(pathC, &info) && (info.st_mode & S_IFMT) == fmt &&
           (!checkW || 0 ==::access(pathC, W_OK));
}

inline bool fExists(const QString &p)
{
    return check(p, S_IFREG, false);
}

inline bool dWritable(const QString &p)
{
    return check(p, S_IFDIR, true);
}

static QString getDir(const QString &f)
{
    QString d(f);

    int slashPos = d.lastIndexOf('/');

    if (-1 != slashPos) {
        d.remove(slashPos + 1, d.length());
    }

    return dirSyntax(d);
}

static time_t getTimeStamp(const QString &item)
{
    KDE_struct_stat info;

    return !item.isNull() && 0 == KDE_lstat(QFile::encodeName(item), &info) ? info.st_mtime : 0;
}

//
// Obtain location of config file to use.
QString getConfigFile()
{
    FcStrList   *list = FcConfigGetConfigFiles(FcConfigGetCurrent());
    QStringList files;
    FcChar8     *file;
    QString     home(dirSyntax(QDir::homePath()));

    while ((file = FcStrListNext(list))) {
        QString f((const char *)file);

        if (fExists(f) && 0 == f.indexOf(home)) {
            files.append(f);
        }
    }
    FcStrListDone(list);

    //
    // Go through list of files, looking for the preferred one...
    if (files.count()) {
        QStringList::const_iterator it(files.begin()),
                    end(files.end());

        for (; it != end; ++it)
            if (-1 != (*it).indexOf(QRegExp("/\\.?fonts\\.conf$"))) {
                return *it;
            }
        return files.front();  // Just return the 1st one...
    } else { // Hmmm... no known files?
        if (FcGetVersion() >= 21000) {
            QString targetPath(KGlobal::dirs()->localxdgconfdir() + "fontconfig");
            QDir target(targetPath);
            if (!target.exists()) {
                target.mkpath(targetPath);
            }
            return targetPath + "/fonts.conf";
        } else {
            return home + "/.fonts.conf";
        }
    }
}

static QString getEntry(QDomElement element, const char *type, unsigned int numAttributes, ...)
{
    if (numAttributes == element.attributes().length()) {
        va_list      args;
        unsigned int arg;
        bool         ok = true;

        va_start(args, numAttributes);

        for (arg = 0; arg < numAttributes && ok; ++arg) {
            const char *attr = va_arg(args, const char *);
            const char *val = va_arg(args, const char *);

            if (!attr || !val || val != element.attribute(attr)) {
                ok = false;
            }
        }

        va_end(args);

        if (ok) {
            QDomNode n = element.firstChild();

            if (!n.isNull()) {
                QDomElement e = n.toElement();

                if (!e.isNull() && type == e.tagName()) {
                    return e.text();
                }
            }
        }
    }

    return QString();
}

static KXftConfig::SubPixel::Type strToType(const char *str)
{
    if (0 == strcmp(str, "rgb")) {
        return KXftConfig::SubPixel::Rgb;
    } else if (0 == strcmp(str, "bgr")) {
        return KXftConfig::SubPixel::Bgr;
    } else if (0 == strcmp(str, "vrgb")) {
        return KXftConfig::SubPixel::Vrgb;
    } else if (0 == strcmp(str, "vbgr")) {
        return KXftConfig::SubPixel::Vbgr;
    } else if (0 == strcmp(str, "none")) {
        return KXftConfig::SubPixel::None;
    } else {
		return KXftConfig::SubPixel::NotSet;
    }
}

static KXftConfig::Hint::Style strToStyle(const char *str)
{
    if (0 == strcmp(str, "hintslight")) {
        return KXftConfig::Hint::Slight;
    } else if (0 == strcmp(str, "hintmedium")) {
        return KXftConfig::Hint::Medium;
    } else if (0 == strcmp(str, "hintfull")) {
        return KXftConfig::Hint::Full;
    } else {
        return KXftConfig::Hint::None;
    }
}

KXftConfig::KXftConfig()
    : m_doc("fontconfig")
    , m_file(getConfigFile())
{
    kDebug(1208) << "Using fontconfig file:" << m_file;
    reset();
}

KXftConfig::~KXftConfig()
{
}

bool KXftConfig::reset()
{
    bool ok = false;

    m_madeChanges = false;
    m_hint.reset();
    m_hinting.reset();
    m_excludeRange.reset();
    m_excludePixelRange.reset();
    m_subPixel.reset();
    m_antiAliasing.reset();

    QFile f(m_file);

    if (f.open(QIODevice::ReadOnly)) {
        m_time = getTimeStamp(m_file);
        ok = true;
        m_doc.clear();

        if (m_doc.setContent(&f)) {
            readContents();
        }
        f.close();
    } else {
        ok = !fExists(m_file) && dWritable(getDir(m_file));
    }

    if (m_doc.documentElement().isNull()) {
        m_doc.appendChild(m_doc.createElement("fontconfig"));
    }

    if (ok) {
        //
        // Check exclude range values - i.e. size and pixel size...
        // If "size" range is set, ensure "pixelsize" matches...
        if (!equal(0, m_excludeRange.from) || !equal(0, m_excludeRange.to)) {
            double pFrom = (double)point2Pixel(m_excludeRange.from),
                   pTo = (double)point2Pixel(m_excludeRange.to);

            if (!equal(pFrom, m_excludePixelRange.from) || !equal(pTo, m_excludePixelRange.to)) {
                m_excludePixelRange.from = pFrom;
                m_excludePixelRange.to = pTo;
                m_madeChanges = true;
            }
        } else if (!equal(0, m_excludePixelRange.from) || !equal(0, m_excludePixelRange.to)) {
            // "pixelsize" set, but not "size" !!!
            m_excludeRange.from = (int)pixel2Point(m_excludePixelRange.from);
            m_excludeRange.to = (int)pixel2Point(m_excludePixelRange.to);
            m_madeChanges = true;
        }
    }

    return ok;
}

bool KXftConfig::apply()
{
    bool ok = true;

    if (m_madeChanges) {
        //
        // Check if file has been written since we last read it. If it has, then re-read and add any
        // of our changes...
        if (fExists(m_file) && getTimeStamp(m_file) != m_time) {
            KXftConfig newConfig;

            newConfig.setExcludeRange(m_excludeRange.from, m_excludeRange.to);
            newConfig.setSubPixelType(m_subPixel.type);
            newConfig.setHintStyle(m_hint.style);
            newConfig.setAntiAliasing(m_antiAliasing.state);

            ok = newConfig.changed() ? newConfig.apply() : true;
            if (ok) {
                reset();
            } else {
                m_time = getTimeStamp(m_file);
            }
        } else {
            // Ensure these are always equal...
            m_excludePixelRange.from = (int)point2Pixel(m_excludeRange.from);
            m_excludePixelRange.to = (int)point2Pixel(m_excludeRange.to);

            FcAtomic *atomic = FcAtomicCreate((const unsigned char *)(QFile::encodeName(m_file).data()));

            ok = false;
            if (atomic) {
                if (FcAtomicLock(atomic)) {
                    FILE *f = fopen((char *)FcAtomicNewFile(atomic), "w");

                    if (f) {
                        applySubPixelType();
                        applyHintStyle();
                        applyAntiAliasing();
                        applyExcludeRange(false);
                        applyExcludeRange(true);

                        //
                        // Check document syntax...
                        static const char qtXmlHeader[]   = "<?xml version = '1.0'?>";
                        static const char xmlHeader[]     = "<?xml version=\"1.0\"?>";
                        static const char qtDocTypeLine[] = "<!DOCTYPE fontconfig>";
                        static const char docTypeLine[]   = "<!DOCTYPE fontconfig SYSTEM "
                                                            "\"fonts.dtd\">";

                        QString str(m_doc.toString());
                        int     idx;

                        if (0 != str.indexOf("<?xml")) {
                            str.insert(0, xmlHeader);
                        } else if (0 == str.indexOf(qtXmlHeader)) {
                            str.replace(0, strlen(qtXmlHeader), xmlHeader);
                        }

                        if (-1 != (idx = str.indexOf(qtDocTypeLine))) {
                            str.replace(idx, strlen(qtDocTypeLine), docTypeLine);
                        }

                        //
                        // Write to file...
                        fputs(str.toUtf8(), f);
                        fclose(f);

                        if (FcAtomicReplaceOrig(atomic)) {
                            ok = true;
                            reset(); // Re-read contents..
                        } else {
                            FcAtomicDeleteNew(atomic);
                        }
                    }
                    FcAtomicUnlock(atomic);
                }
                FcAtomicDestroy(atomic);
            }
        }
    }

    return ok;
}

bool KXftConfig::getSubPixelType(SubPixel::Type &type)
{
    type = m_subPixel.type;
    return SubPixel::None != m_subPixel.type;
}

void KXftConfig::setSubPixelType(SubPixel::Type type)
{
    if (type != m_subPixel.type) {
        m_subPixel.type = type;
        m_madeChanges = true;
    }
}

bool KXftConfig::getHintStyle(Hint::Style &style)
{
    if (Hint::NotSet != m_hint.style && !m_hint.toBeRemoved) {
        style = m_hint.style;
        return true;
    } else {
        return false;
    }
}

void KXftConfig::setHintStyle(Hint::Style style)
{
    if ((Hint::NotSet == style && Hint::NotSet != m_hint.style && !m_hint.toBeRemoved) ||
            (Hint::NotSet != style && (style != m_hint.style || m_hint.toBeRemoved))) {
        m_hint.toBeRemoved = (Hint::NotSet == style);
        m_hint.style = style;
        m_madeChanges = true;
    }

    if (Hint::NotSet != style) {
        setHinting(Hint::None != m_hint.style);
    }
}

void KXftConfig::setHinting(bool set)
{
    if (set != m_hinting.set) {
        m_hinting.set = set;
        m_madeChanges = true;
    }
}

bool KXftConfig::getExcludeRange(double &from, double &to)
{
    if (!equal(0, m_excludeRange.from) || !equal(0, m_excludeRange.to)) {
        from = m_excludeRange.from;
        to = m_excludeRange.to;
        return true;
    } else {
        return false;
    }
}

void KXftConfig::setExcludeRange(double from, double to)
{
    double f = from < to ? from : to,
           t = from < to ? to   : from;

    if (!equal(f, m_excludeRange.from) || !equal(t, m_excludeRange.to)) {
        m_excludeRange.from = f;
        m_excludeRange.to = t;
        m_madeChanges = true;
    }
}

QString KXftConfig::description(SubPixel::Type t)
{
    switch (t) {
    default:
    case SubPixel::NotSet:
        return i18nc("use system subpixel setting", "System default");
    case SubPixel::None:
        return i18nc("no subpixel rendering", "None");
    case SubPixel::Rgb:
        return i18n("RGB");
    case SubPixel::Bgr:
        return i18n("BGR");
    case SubPixel::Vrgb:
        return i18n("Vertical RGB");
    case SubPixel::Vbgr:
        return i18n("Vertical BGR");
    }
}

const char *KXftConfig::toStr(SubPixel::Type t)
{
    switch (t) {
    default:
    case SubPixel::None:
        return "none";
    case SubPixel::Rgb:
        return "rgb";
    case SubPixel::Bgr:
        return "bgr";
    case SubPixel::Vrgb:
        return "vrgb";
    case SubPixel::Vbgr:
        return "vbgr";
    }
}

QString KXftConfig::description(Hint::Style s)
{
    switch (s) {
    default:
    case Hint::NotSet:
        return i18nc("use system hinting settings", "System default");
    case Hint::Medium:
        return i18nc("medium hinting", "Medium");
    case Hint::None:
        return i18nc("no hinting", "None");
    case Hint::Slight:
        return i18nc("slight hinting", "Slight");
    case Hint::Full:
        return i18nc("full hinting", "Full");
    }
}

const char *KXftConfig::toStr(Hint::Style s)
{
    switch (s) {
    default:
    case Hint::NotSet:
        return "";
    case Hint::Medium:
        return "hintmedium";
    case Hint::None:
        return "hintnone";
    case Hint::Slight:
        return "hintslight";
    case Hint::Full:
        return "hintfull";
    }
}

void KXftConfig::readContents()
{
    QDomNode n = m_doc.documentElement().firstChild();

    while (!n.isNull()) {
        QDomElement e = n.toElement();

        if (!e.isNull()) {
            if ("match" == e.tagName()) {
                QString str;

                switch (e.childNodes().count()) {
                case 1:
                    if ("font" == e.attribute("target")) {
                        QDomElement ene = e.firstChild().toElement();

                        if (!ene.isNull() && "edit" == ene.tagName()) {
                            if (!(str = getEntry(ene, "const", 2, "name", "rgba", "mode",
                                                 "assign")).isNull()) {
                                m_subPixel.node = n;
                                m_subPixel.type = strToType(str.toLatin1());
                            } else if (!(str = getEntry(ene, "const", 2, "name", "hintstyle", "mode",
                                                        "assign")).isNull()) {
                                m_hint.node = n;
                                m_hint.style = strToStyle(str.toLatin1());
                            } else if (!(str = getEntry(ene, "bool", 2, "name", "hinting", "mode",
                                                        "assign")).isNull()) {
                                m_hinting.node = n;
                                m_hinting.set = str.toLower() != "false";
                            } else if (!(str = getEntry(ene, "bool", 2, "name", "antialias", "mode",
                                                        "assign")).isNull()) {
                                m_antiAliasing.node = n;
                                m_antiAliasing.state = str.toLower() != "false" ?
                                                       AntiAliasing::Enabled : AntiAliasing::Disabled;
                            }
                        }
                    }
                    break;
                case 3: // CPD: Is target "font" or "pattern" ????
                    if ("font" == e.attribute("target")) {
                        bool     foundFalse = false;
                        QDomNode en = e.firstChild();
                        QString  family;
                        double   from = -1.0,
                                 to = -1.0,
                                 pixelFrom = -1.0,
                                 pixelTo = -1.0;

                        while (!en.isNull()) {
                            QDomElement ene = en.toElement();

                            if (!ene.isNull()) {
                                if ("test" == ene.tagName()) {
                                    // kcmfonts used to write incorrectly more or less instead of
                                    // more_eq and less_eq, so read both,
                                    // first the old (wrong) one then the right one
                                    if (!(str = getEntry(ene, "double", 3, "qual", "any", "name",
                                                         "size", "compare", "more")).isNull()) {
                                        from = str.toDouble();
                                    }
                                    if (!(str = getEntry(ene, "double", 3, "qual", "any", "name",
                                                         "size", "compare", "more_eq")).isNull()) {
                                        from = str.toDouble();
                                    }
                                    if (!(str = getEntry(ene, "double", 3, "qual", "any", "name",
                                                         "size", "compare", "less")).isNull()) {
                                        to = str.toDouble();
                                    }
                                    if (!(str = getEntry(ene, "double", 3, "qual", "any", "name",
                                                         "size", "compare", "less_eq")).isNull()) {
                                        to = str.toDouble();
                                    }
                                    if (!(str = getEntry(ene, "double", 3, "qual", "any", "name",
                                                         "pixelsize", "compare", "more")).isNull()) {
                                        pixelFrom = str.toDouble();
                                    }
                                    if (!(str = getEntry(ene, "double", 3, "qual", "any", "name",
                                                         "pixelsize", "compare",
                                                         "more_eq")).isNull()) {
                                        pixelFrom = str.toDouble();
                                    }
                                    if (!(str = getEntry(ene, "double", 3, "qual", "any", "name",
                                                         "pixelsize", "compare", "less")).isNull()) {
                                        pixelTo = str.toDouble();
                                    }
                                    if (!(str = getEntry(ene, "double", 3, "qual", "any", "name",
                                                         "pixelsize", "compare",
                                                         "less_eq")).isNull()) {
                                        pixelTo = str.toDouble();
                                    }
                                } else if ("edit" == ene.tagName() &&
                                           "false" == getEntry(ene, "bool", 2, "name", "antialias",
                                                               "mode", "assign")) {
                                    foundFalse = true;
                                }
                            }

                            en = en.nextSibling();
                        }

                        if ((from >= 0 || to >= 0) && foundFalse) {
                            m_excludeRange.from = from < to ? from : to;
                            m_excludeRange.to  = from < to ? to   : from;
                            m_excludeRange.node = n;
                        } else if ((pixelFrom >= 0 || pixelTo >= 0) && foundFalse) {
                            m_excludePixelRange.from = pixelFrom < pixelTo ? pixelFrom : pixelTo;
                            m_excludePixelRange.to  = pixelFrom < pixelTo ? pixelTo   : pixelFrom;
                            m_excludePixelRange.node = n;
                        }
                    }
                    break;
                default:
                    break;
                }
            }
        }
        n = n.nextSibling();
    }
}

void KXftConfig::applySubPixelType()
{
    if (SubPixel::NotSet == m_subPixel.type) {
        if (!m_subPixel.node.isNull()) {
            m_doc.documentElement().removeChild(m_subPixel.node);
            m_subPixel.node.clear();
        }
    } else {
        QDomElement matchNode = m_doc.createElement("match");
        QDomElement typeNode  = m_doc.createElement("const");
        QDomElement editNode  = m_doc.createElement("edit");
        QDomText    typeText  = m_doc.createTextNode(toStr(m_subPixel.type));

        matchNode.setAttribute("target", "font");
        editNode.setAttribute("mode", "assign");
        editNode.setAttribute("name", "rgba");
        editNode.appendChild(typeNode);
        typeNode.appendChild(typeText);
        matchNode.appendChild(editNode);
        if (m_subPixel.node.isNull()) {
            m_doc.documentElement().appendChild(matchNode);
        } else {
            m_doc.documentElement().replaceChild(matchNode, m_subPixel.node);
        }
        m_subPixel.node = matchNode;
    }
}

void KXftConfig::applyHintStyle()
{
    applyHinting();

    if (Hint::NotSet == m_hint.style) {
        if (!m_hint.node.isNull()) {
            m_doc.documentElement().removeChild(m_hint.node);
            m_hint.node.clear();
        }
        if (!m_hinting.node.isNull()) {
            m_doc.documentElement().removeChild(m_hinting.node);
            m_hinting.node.clear();
        }
    } else {
        QDomElement matchNode = m_doc.createElement("match"),
                    typeNode  = m_doc.createElement("const"),
                    editNode  = m_doc.createElement("edit");
        QDomText    typeText  = m_doc.createTextNode(toStr(m_hint.style));

        matchNode.setAttribute("target", "font");
        editNode.setAttribute("mode", "assign");
        editNode.setAttribute("name", "hintstyle");
        editNode.appendChild(typeNode);
        typeNode.appendChild(typeText);
        matchNode.appendChild(editNode);
        if (m_hint.node.isNull()) {
            m_doc.documentElement().appendChild(matchNode);
        } else {
            m_doc.documentElement().replaceChild(matchNode, m_hint.node);
        }
        m_hint.node = matchNode;
    }
}

void KXftConfig::applyHinting()
{
    QDomElement matchNode = m_doc.createElement("match"),
                typeNode  = m_doc.createElement("bool"),
                editNode  = m_doc.createElement("edit");
    QDomText    typeText  = m_doc.createTextNode(m_hinting.set ? "true" : "false");

    matchNode.setAttribute("target", "font");
    editNode.setAttribute("mode", "assign");
    editNode.setAttribute("name", "hinting");
    editNode.appendChild(typeNode);
    typeNode.appendChild(typeText);
    matchNode.appendChild(editNode);
    if (m_hinting.node.isNull()) {
        m_doc.documentElement().appendChild(matchNode);
    } else {
        m_doc.documentElement().replaceChild(matchNode, m_hinting.node);
    }
    m_hinting.node = matchNode;
}

void KXftConfig::applyExcludeRange(bool pixel)
{
    Exclude &range = pixel ? m_excludePixelRange : m_excludeRange;

    if (equal(range.from, 0) && equal(range.to, 0)) {
        if (!range.node.isNull()) {
            m_doc.documentElement().removeChild(range.node);
            range.node.clear();
        }
    } else {
        QString     fromString,
                    toString;

        fromString.setNum(range.from);
        toString.setNum(range.to);

        QDomElement matchNode    = m_doc.createElement("match"),
                    fromTestNode = m_doc.createElement("test"),
                    fromNode     = m_doc.createElement("double"),
                    toTestNode   = m_doc.createElement("test"),
                    toNode       = m_doc.createElement("double"),
                    editNode     = m_doc.createElement("edit"),
                    boolNode     = m_doc.createElement("bool");
        QDomText    fromText     = m_doc.createTextNode(fromString),
                    toText       = m_doc.createTextNode(toString),
                    boolText     = m_doc.createTextNode("false");

        matchNode.setAttribute("target", "font");   // CPD: Is target "font" or "pattern" ????
        fromTestNode.setAttribute("qual", "any");
        fromTestNode.setAttribute("name", pixel ? "pixelsize" : "size");
        fromTestNode.setAttribute("compare", "more_eq");
        fromTestNode.appendChild(fromNode);
        fromNode.appendChild(fromText);
        toTestNode.setAttribute("qual", "any");
        toTestNode.setAttribute("name", pixel ? "pixelsize" : "size");
        toTestNode.setAttribute("compare", "less_eq");
        toTestNode.appendChild(toNode);
        toNode.appendChild(toText);
        editNode.setAttribute("mode", "assign");
        editNode.setAttribute("name", "antialias");
        editNode.appendChild(boolNode);
        boolNode.appendChild(boolText);
        matchNode.appendChild(fromTestNode);
        matchNode.appendChild(toTestNode);
        matchNode.appendChild(editNode);

        if (!m_antiAliasing.node.isNull()) {
            m_doc.documentElement().removeChild(range.node);
        }
        m_doc.documentElement().appendChild(matchNode);
        range.node = matchNode;
    }
}

KXftConfig::AntiAliasing::State KXftConfig::getAntiAliasing() const
{
    return m_antiAliasing.state;
}

void KXftConfig::setAntiAliasing(AntiAliasing::State state)
{
    if (state != m_antiAliasing.state) {
        m_antiAliasing.state = state;
        m_madeChanges = true;
    }
}

void KXftConfig::applyAntiAliasing()
{
    if (AntiAliasing::NotSet == m_antiAliasing.state) {
        if (!m_antiAliasing.node.isNull()) {
            m_doc.documentElement().removeChild(m_antiAliasing.node);
            m_antiAliasing.node.clear();
        }
    } else {
        QDomElement matchNode = m_doc.createElement("match");
        QDomElement typeNode  = m_doc.createElement("bool");
        QDomElement editNode  = m_doc.createElement("edit");
        QDomText    typeText  = m_doc.createTextNode(m_antiAliasing.state == AntiAliasing::Enabled ?
                                                     "true" : "false");

        matchNode.setAttribute("target", "font");
        editNode.setAttribute("mode", "assign");
        editNode.setAttribute("name", "antialias");
        editNode.appendChild(typeNode);
        typeNode.appendChild(typeText);
        matchNode.appendChild(editNode);
        if (!m_antiAliasing.node.isNull()) {
            m_doc.documentElement().removeChild(m_antiAliasing.node);
        }
        m_doc.documentElement().appendChild(matchNode);
        m_antiAliasing.node = matchNode;
    }
}

// KXftConfig only parses one config file, user's .fonts.conf usually.
// If that one doesn't exist, then KXftConfig doesn't know if antialiasing
// is enabled or not. So try to find out the default value from the default font.
// Maybe there's a better way *shrug*.
bool KXftConfig::aliasingEnabled()
{
    FcPattern *pattern = FcPatternCreate();
    FcConfigSubstitute(0, pattern, FcMatchPattern);
    FcDefaultSubstitute(pattern);
    FcResult result;
    FcPattern *f = FcFontMatch(0, pattern, &result);
    FcBool antialiased = FcTrue;
    FcPatternGetBool(f, FC_ANTIALIAS, 0, &antialiased);
    FcPatternDestroy(f);
    FcPatternDestroy(pattern);
    return antialiased == FcTrue;
}

#endif
