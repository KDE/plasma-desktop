/*
 *  Copyright (C) 2012 Shivam Makkar (amourphious1992@gmail.com)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */


#include "keyboardlayout.h"
#include "keysymbols.h"

#include <QMessageBox>
#include <QList>
#include <QFile>
#include <QDir>

#include <QX11Info>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/XKBlib.h>
#include <X11/extensions/XKBrules.h>
#include <fixx11h.h>
#include <config-workspace.h>


//TODO: replace this with grammar parser (e.g. antlr)


KeyboardLayout::KeyboardLayout()
{
}

void KeyboardLayout::generateLayout(QString a,const QString& cname)
{
    includeSymbol(a,cname);
    int i=a.indexOf("name[Group1]=");
    i+=13;

    QString n=a.mid(i);
    n=n.simplified();
    i=n.indexOf("\"",1);
    layoutName=n.left(i);
    layoutName.remove("\"");
    layoutName.simplified();
    i=n.indexOf("key");
    n=n.mid(i);

    QList<QString> st;
    st=n.split("key");

    KeySymbols dum;
    QString r,y;

    for(int k=0;k<st.size();k++){
        dum.setKey(st.at(k));
        if(dum.keyname.startsWith("Lat"))
            dum.keyname=alias.getAlias(cname,dum.keyname);
        if(dum.keyname=="TLDE"){
            r=st.at(k);
            TLDE.setKey(r);
        }
        if(dum.keyname=="BKSL"){
            r=st.at(k);
            BKSL.setKey(r);
        }
        if(dum.keyname.contains("AE")){
            QString ind=dum.keyname.right(2);
            int index=ind.toInt();
            r=st.at(k);
            AE[index-1].setKey(r);
        }
        if(dum.keyname.contains("AD")){
            QString ind=dum.keyname.right(2);
            int index=ind.toInt();
            r=st.at(k);
            AD[index-1].setKey(r);
        }
        if(dum.keyname.contains("AC")){
            QString ind=dum.keyname.right(2);
            int index=ind.toInt();
            r=st.at(k);
            AC[index-1].setKey(r);
        }
        if(dum.keyname.contains("AB")){
            QString ind=dum.keyname.right(2);
            int index=ind.toInt();
            r=st.at(k);
            AB[index-1].setKey(r);
        }
    }
}

void KeyboardLayout::includeSymbol(QString a,const QString& cname)
{
    int k=a.indexOf("include");
    a=a.mid(k);

    QList<QString>tobeinclude;
    tobeinclude=a.split("include");

    QString r;
    for(int o=1;o<tobeinclude.size();o++){
        QString d=tobeinclude.at(o);
        d.simplified();
        int k=d.indexOf("\"",2);

        QString incsym=d.left(k);
        incsym.remove(" ");
        incsym.remove("\"");

        QList<QString> incfile;
        incfile=incsym.split("(");
        for(int i=0;i<incfile.size();i++){
                QString z=incfile.at(i);
                z.remove(" ");
            incfile[i]=z;
        }
        if(incfile.size()==1)
            incfile<<"basic";
        else{
            QString ns=incfile.at(1);
            ns.remove(")");
            incfile[1]=ns;
        }
        r=incfile.at(0);
        r.append(incfile.at(1));

        QString filename=findSymbolBaseDir();
        filename.append(incfile.at(0));

        QFile file(filename);
        file.open(QIODevice::ReadOnly | QIODevice::Text);

        QString content = file.readAll();
        QList<QString> symstrlist;

        symstrlist=content.split("xkb_symbols ");
        for(int u=1;u<symstrlist.size();u++){
            QString cur=symstrlist.at(u);
            int pos = cur.indexOf("{");
            cur=cur.left(pos);
            if(cur.contains(incfile.at(1))){
                generateLayout(symstrlist.at(u),cname);
                break;
            }
        }
    }
}

QString KeyboardLayout::findSymbolBaseDir()
{
    QString xkbParentDir;

    QString base(XLIBDIR);
    if( base.count('/') >= 3 ) {
        // .../usr/lib/X11 -> /usr/share/X11/xkb vs .../usr/X11/lib -> /usr/X11/share/X11/xkb
        QString delta = base.endsWith("X11") ? "/../../share/X11" : "/../share/X11";
        QDir baseDir(base + delta);
        if( baseDir.exists() ) {
            xkbParentDir = baseDir.absolutePath();
        }
        else {
            QDir baseDir(base + "/X11");	// .../usr/X11/lib/X11/xkb (old XFree)
            if( baseDir.exists() ) {
                xkbParentDir = baseDir.absolutePath();
            }
        }
    }

    if( xkbParentDir.isEmpty() ) {
        xkbParentDir = "/usr/share/X11";
    }

    return QString("%1/xkb/symbols/").arg(xkbParentDir);
}
