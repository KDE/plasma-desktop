#ifndef FOLDER_H
#define FOLDER_H

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

#include <QString>
#include <QSet>
#include <sys/time.h>
#include "Family.h"
#include "Style.h"
#include "File.h"
#include "Misc.h"

namespace KFI
{

class Folder
{
    struct CfgFile
    {
        bool modified();
        void updateTimeStamp();

        bool    dirty;
        QString name;
        time_t  timestamp;
    };

    public:

    struct FlatFont : Misc::TFont
    {
        FlatFont(const Family &fam, const Style &style, const File &f)
            : Misc::TFont(fam.name(), style.value()),
              writingSystems(style.writingSystems()),
              scalable(style.scalable()),
              file(f)
        {
        }
        bool operator==(const FlatFont &o) const  { return file.path()==o.file.path(); }

        qulonglong writingSystems;
        bool       scalable;
        File       file;
    };

    struct Flat : public QSet<FlatFont>
    {
        Families build(bool system) const;
    };
    
    Folder() { }
    ~Folder();

    void                      init(bool system, bool systemBus);
    const QString &           location() const                           { return itsLocation; }
    bool                      allowToggling() const;
    void                      loadDisabled();
    void                      saveDisabled();
    void                      setDisabledDirty()                         { itsDisabledCfg.dirty=true; }
    bool                      disabledDirty() const                      { return itsDisabledCfg.dirty; }
    QStringList               toXml(int max=0);
    Families                  list();
    bool                      contains(const QString &family, quint32 style);
    void                      add(const Family &family);
    void                      addModifiedDir(const QString &dir)         { itsModifiedDirs.insert(dir); }
    void                      addModifiedDirs(const QSet<QString> &dirs) { itsModifiedDirs+=dirs; }
    bool                      isModified() const                         { return !itsModifiedDirs.isEmpty(); }
    void                      clearModified()                            { itsModifiedDirs.clear(); }
    void                      configure(bool force=false);
    Flat                      flatten() const;
    const FamilyCont &        fonts() const                              { return itsFonts; }
    FamilyCont::ConstIterator addFont(const Family &fam)                 { return itsFonts.insert(fam); }
    void                      removeFont(const Family &fam)              { itsFonts.remove(fam); }
    void                      clearFonts()                               { itsFonts.clear(); }

    private:

    bool          itsIsSystem;
    FamilyCont    itsFonts;
    CfgFile       itsDisabledCfg;
    QString       itsLocation;
    QSet<QString> itsModifiedDirs;
};

inline Q_DECL_EXPORT uint qHash(const Folder::FlatFont &key)
{
    return qHash(key.file); // +qHash(key.index());
}

}

#endif
