#ifndef __STYLE_H__
#define __STYLE_H__

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

#include <QSet>
#include <QMetaType>
#include <QDBusArgument>
#include "kfontinst_export.h"
#include "File.h"

class QDomElement;
class QTextStream;

namespace KFI
{

class KFONTINST_EXPORT Style
{
    public:

    Style(quint32 v=0, bool sc=false, qulonglong ws=0) : itsValue(v), itsWritingSystems(ws), itsScalable(sc) { }
    Style(const QDomElement &elem, bool loadFiles);

    bool operator==(const Style &o) const                          { return itsValue==o.itsValue; }

    QString                 toXml(bool disabled, const QString &family, QTextStream &s) const;
    FileCont::ConstIterator add(const File &f) const               { return itsFiles.insert(f); }
    void                    remove(const File &f) const            { itsFiles.remove(f); }
    quint32                 value() const                          { return itsValue; }
    void                    setWritingSystems(qulonglong ws) const { itsWritingSystems=ws; }
    qulonglong              writingSystems() const                 { return itsWritingSystems; }
    const FileCont &        files() const                          { return itsFiles; }
    void                    setScalable(bool sc=true) const        { itsScalable=sc; }
    bool                    scalable() const                       { return itsScalable; }
    void                    clearFiles() const                     { itsFiles.clear(); }
    void                    setFiles(const FileCont &f) const      { itsFiles=f; }
    void                    addFiles(const FileCont &f) const      { itsFiles+=f; }
    void                    removeFiles(const FileCont &f) const   { itsFiles-=f; }

    private:

    quint32            itsValue;
    mutable qulonglong itsWritingSystems;
    mutable bool       itsScalable;
    mutable FileCont   itsFiles;
};

typedef QSet<Style> StyleCont;

inline Q_DECL_EXPORT uint qHash(const Style &key)
{
    return key.value();
}

}

Q_DECLARE_METATYPE(KFI::Style)
Q_DECL_EXPORT QDBusArgument & operator<<(QDBusArgument &argument, const KFI::Style &obj);
Q_DECL_EXPORT const QDBusArgument & operator>>(const QDBusArgument &argument, KFI::Style &obj);

#endif
