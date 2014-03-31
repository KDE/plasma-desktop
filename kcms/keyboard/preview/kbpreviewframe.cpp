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


#include "kbpreviewframe.h"

#include <QFile>
#include <QFont>

#include <KApplication>
#include <KLocalizedString>


static const QColor keyBorderColor("#d4d4d4");
static const QColor lev12color("#d4d4d4");
static const QColor lev34color("#FF3300");
static const int sz=20, kszx=70, kszy=70;

static const int xOffset[] = {15, 15, 40, 40 };
static const int yOffset[] = {10, 40, 10, 40 };
static const QColor color[] = { lev12color, lev12color, lev34color, lev34color };


// TODO: parse geometry files and display keyboard according to current keyboard model

KbPreviewFrame::KbPreviewFrame(QWidget *parent) :
    QFrame(parent)
{
     setFrameStyle( QFrame::Box );
     setFrameShadow(QFrame::Sunken);
}

void KbPreviewFrame::paintTLDE(QPainter &painter,int &x,int &y)
{
    painter.setPen(keyBorderColor);
    painter.drawRect(x, y, kszx, kszy);

    const QList <QString> symbols = keyboardLayout.TLDE.symbols;

    for(int level=0; level<symbols.size(); level++) {
    	painter.setPen(color[level]);
    	painter.drawText(x+xOffset[level], y+yOffset[level], sz, sz, Qt::AlignTop, symbol.getKeySymbol(symbols.at(level)));
    }

}

void KbPreviewFrame::paintAERow(QPainter &painter,int &x,int &y)
{
    paintTLDE(painter, x, y);

    const int noAEk=12;
    for(int i=0; i<noAEk; i++){
        x+=kszx;

        painter.setPen(keyBorderColor);
        painter.drawRect(x, y, kszx, kszy);

        QList<QString> symbols = keyboardLayout.AE[i].symbols;

        for(int level=0; level<symbols.size(); level++) {
        	painter.setPen(color[level]);
        	painter.drawText(x+xOffset[level], y+yOffset[level], sz, sz, Qt::AlignTop, symbol.getKeySymbol(symbols.at(level)));
        }
    }

    x += kszx;

    const int bkspszx=100,bk1x=10;//,bk1y=20,
    const int bk2y=60;

    painter.setPen(keyBorderColor);
    painter.drawRect(x, y,bkspszx,kszy);

    painter.setPen(lev12color);
//    painter.drawText(x+bk1x, y+bk1y,i18n("<--"));
    painter.drawText(x+bk1x, y+bk2y,i18n("Backspace"));
}

void KbPreviewFrame::paintADRow(QPainter &painter,int &x,int&y)
{
    const int noADk=12;
    const int tabszx=100;
    const int tab3y=45;

    painter.setPen(keyBorderColor);
    painter.drawRect(x, y, tabszx,kszy);

    painter.setPen(lev12color);
//    painter.drawText(x+tab1x, y+tab1y,i18n("<--"));
    painter.drawText(x+xOffset[0], y+tab3y, i18nc("Tab key", "Tab"));
//    painter.drawText(x+tab2x, y+tab2y,i18n("-->"));
    x+=tabszx;


    for(int i=0; i<noADk; i++){
        QList<QString> symbols = keyboardLayout.AD[i].symbols;

        painter.setPen(keyBorderColor);
        painter.drawRect(x, y,kszx,kszy);

        for(int level=0; level<symbols.size(); level++) {
        	painter.setPen(color[level]);
        	painter.drawText(x+xOffset[level], y+yOffset[level], sz, sz, Qt::AlignTop, symbol.getKeySymbol(symbols.at(level)));
        }

        x+=kszx;
    }

    painter.setPen(keyBorderColor);
    painter.drawRect(x, y,kszx,kszy);

    QList<QString> symbols = keyboardLayout.BKSL.symbols;

    for(int level=0; level<symbols.size(); level++) {
    	painter.setPen(color[level]);
    	painter.drawText(x+xOffset[level], y+yOffset[level], sz, sz, Qt::AlignTop, symbol.getKeySymbol(symbols.at(level)));
    }
}

void KbPreviewFrame::paintACRow(QPainter &painter,int &x,int &y)
{
    const int sz = 20, kszx = 70, kszy = 70, capszx = 100;
    const int noACk = 11;
    const int lvl2x = 40, shifx = 10, shify = 60, retsz = 140;

    painter.setPen(keyBorderColor);
    painter.drawRect(x, y,capszx,kszy);

    painter.setPen(lev12color);
//    painter.drawText(x+shifx, y+sz,i18n("^"));
    painter.drawText(x+shifx, y+shify,i18n("Caps Lock"));
    x+=capszx;

    for(int i=0; i<noACk; i++){
        painter.setPen(keyBorderColor);
        painter.drawRect(x, y,kszx,kszy);

        QList<QString> symbols = keyboardLayout.AC[i].symbols;

        for(int level=0; level<symbols.size(); level++) {
        	painter.setPen(color[level]);
        	painter.drawText(x+xOffset[level], y+yOffset[level], sz, sz, Qt::AlignTop, symbol.getKeySymbol(symbols.at(level)));
        }

        x+=kszx;
    }

    painter.setPen(keyBorderColor);
    painter.drawRect(x, y,retsz,kszy);

    painter.setPen(lev12color);
//    painter.drawText(x+ret1x, y+ret1y,i18n("|"));
//    painter.drawText(x+ret2x, y+ret2y,i18n("<--"));
    painter.drawText(x+shify,y+lvl2x,i18n("Enter"));
}

void KbPreviewFrame::paintABRow(QPainter &painter,int &x,int &y)
{
    const int noABk=10;
    for(int i=0; i<noABk; i++) {
        painter.setPen(keyBorderColor);
        painter.drawRect(x, y,kszx,kszy);

        QList<QString> symbols = keyboardLayout.AB[i].symbols;

        for(int level=0; level<symbols.size(); level++) {
        	painter.setPen(color[level]);
        	painter.drawText(x+xOffset[level], y+yOffset[level], sz, sz, Qt::AlignTop, symbol.getKeySymbol(symbols.at(level)));
        }

        x+=kszx;
    }
}

void KbPreviewFrame::paintBottomRow(QPainter &painter,int &x,int &y)
{
    const int txtx=30, txty=35, ctrlsz=100, altsz=100, spsz=400, kszy=70;

    painter.setPen(keyBorderColor);
    painter.drawRect(x, y, ctrlsz, kszy);
    painter.setPen(lev12color);
    painter.drawText(x+txtx, y+txty,i18n("Ctrl"));

    x+=ctrlsz;

    painter.setPen(keyBorderColor);
    painter.drawRect(x, y, altsz, kszy);
    painter.setPen(lev12color);
    painter.drawText(x+txtx, y+txty,i18n("Alt"));

    x+=altsz;

    painter.setPen(keyBorderColor);
    painter.drawRect(x, y, spsz, kszy);

    x+=spsz;

    painter.drawRect(x, y, altsz, kszy);

    painter.setPen(lev34color);
    painter.drawText(x+txtx, y+txty,i18n("AltGr"));

    x+=ctrlsz;

    painter.setPen(keyBorderColor);
    painter.drawRect(x, y, ctrlsz, kszy);

    painter.setPen(lev12color);
    painter.drawText(x+txtx, y+txty, i18n("Ctrl"));
}

void KbPreviewFrame::paintFnKeys(QPainter &painter,int &x,int &y)
{
    const int escsz=50, escx=20, escy=55;

    painter.setPen(keyBorderColor);
    painter.drawRect(x, y, escsz, escsz);

    painter.setPen(lev12color);
    painter.drawText(escx, escy, i18n("Esc"));

    const int spacex=50;
    x+=spacex;

    const int fnkeyspace=60, fnkeysizex=50, fnkeysizey=50, fkc=15, fky=30, fnkig=4, fng=3;
    int f=1;

    for(int i=0;i<fng;i++){
        x+=spacex;

        for(int j=0;j<fnkig;j++){
            x += fnkeyspace;
            painter.setPen(keyBorderColor);
            painter.drawRect(x, y, fnkeysizex, fnkeysizey);

            painter.setPen(lev12color);
            painter.drawText(x+fkc, y+fky, i18nc("Function key", "F%1", f));
            f++;
        }
    }
}

void KbPreviewFrame::paintEvent(QPaintEvent *)
{
    QPainter painter(this);

    QFont kbfont;
    kbfont.setPointSize(12);

    painter.setFont(kbfont);
    painter.setBrush(QBrush(Qt::darkGray));

    const int strtx=0,strty=0,endx=1390,endy=490,kszy=70;
    const int row1x=10,row1y=30,row2x=10,row2y=90,row5x=10,row5y=330,row3x=10,row3y=170,shifx=10,shify=60,row4x=10,row4y=250,row6x=110,row6y=410;
    const int shiftsz=155;

    painter.setPen(keyBorderColor);
    painter.drawRect(strtx, strty, endx, endy);

    painter.setPen(lev12color);
    painter.setBrush(QBrush(Qt::black));

    int x, y;
    x=row1x;
    y=row1y;

    paintFnKeys(painter,x, y);

    x=row2x;
    y=row2y;

    paintAERow(painter,x, y);

    x=row3x;
    y=row3y;

    paintADRow(painter,x, y);

    x=row4x;
    y=row4y;

    paintACRow(painter,x, y);

    x=row5x;
    y=row5y;

    painter.setPen(keyBorderColor);
    painter.drawRect(x, y,shiftsz,kszy);
    painter.setPen(lev12color);
    painter.drawText(x+shifx, y+shify,i18n("Shift"));
    x+=shiftsz;

    paintABRow(painter,x, y);

    painter.setPen(keyBorderColor);
    painter.drawRect(x, y,shiftsz,kszy);
    painter.setPen(lev12color);
    painter.drawText(x+shifx, y+shify,i18n("Shift"));

    x=row6x;
    y=row6y;

    paintBottomRow(painter,x, y);

    if( symbol.isFailed() ) {
        painter.setPen(keyBorderColor);
        painter.drawRect(strtx, strty, endx, endy);

        const int midx=470, midy=240;
        painter.setPen(lev12color);
        painter.drawText(midx, midy, i18n("No preview found"));
    }

}


void KbPreviewFrame::generateKeyboardLayout(const QString& layout, const QString& layoutVariant)
{
    QString filename = keyboardLayout.findSymbolBaseDir();
    filename.append(layout);

    QFile file(filename);
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    QString content = file.readAll();
    file.close();

    QList<QString> symstr = content.split("xkb_symbols ");

    if( layoutVariant.isEmpty() ) {
        keyboardLayout.generateLayout(symstr.at(1), layout);
    }
    else {
        for(int i=1;i<symstr.size();i++) {
            QString h=symstr.at(i);
            int k=h.indexOf("\"");
            h=h.mid(k);
            k=h.indexOf("{");
            h=h.left(k);
            h=h.remove(" ");
            QString f="\"";
            f.append(layoutVariant);
            f.append("\"");
            f=f.remove(" ");

            if(h==f){
                keyboardLayout.generateLayout(symstr.at(i), layout);
                break;
            }
        }
    }
}

