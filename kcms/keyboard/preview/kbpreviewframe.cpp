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

#include "geometry_parser.h"
#include "geometry_components.h"
#include "keyboardlayout.h"
#include "symbol_parser.h"

#include <QFile>
#include <QFont>
#include <QFileDialog>
#include <QHelpEvent>
#include <QMessageBox>
#include <QRect>
#include <QDesktopWidget>
#include <QApplication>

#include <KLocale>

#include <math.h>


static const QColor keyBorderColor("#d4d4d4");
static const QColor lev12color(Qt::black);
static const QColor lev34color("#0033FF");
static const QColor unknownSymbolColor("#FF3300");
static const int xOffset[] = {10, 10, -15, -15 };
static const int yOffset[] = {5, -20, 5, -20 };
static const QColor color[] = { lev12color, lev12color, lev34color, lev34color };
static const int keyLevel[3][4] = { { 1, 0, 3, 2}, { 1, 0, 5, 4}, { 1, 0, 7, 6} };
static const QRegExp fkKey("^FK\\d+$");


KbPreviewFrame::KbPreviewFrame(QWidget *parent) :
    QFrame(parent),
    geometry(*new Geometry())
{
    setFrameStyle(QFrame::Box);
    setFrameShadow(QFrame::Sunken);
    setMouseTracking(true);
    scaleFactor = 1;
    l_id = 0;
}

KbPreviewFrame::~KbPreviewFrame()
{
    delete &geometry;
}


int KbPreviewFrame::getWidth() const
{
    return geometry.width;
}
int KbPreviewFrame::getHeight() const
{
    return geometry.height;
}


//writes text on the keys call only by paintevent
void KbPreviewFrame::drawKeySymbols(QPainter &painter, QPoint temp[], const GShape &s, const QString &name)
{
    int keyindex = keyboardLayout.findKey(name);
    int szx = scaleFactor * s.size(0) / 2 < 20 ? scaleFactor * s.size(0) / 3 : 20;
    int szy = scaleFactor * s.size(1) / 2 < 20 ? scaleFactor * s.size(1) / 3 : 20;
    QFont kbfont;
    if (szx > szy) {
        kbfont.setPointSize(szy / 2 < 9 ? szy : 9);
    } else {
        kbfont.setPointSize(szx / 2 < 9 ? szx / 2 : 9);
    }

    painter.setFont(kbfont);

    int cordinate[] = {0, 3, 1, 2};
    float tooltipX = 0, toolTipY = 0;
    QString tip;
    if (keyindex != -1) {
        KbKey key = keyboardLayout.keyList.at(keyindex);

        for (int level = 0; level < (key.getSymbolCount() < 4 ? key.getSymbolCount() : 4); level++) {

            if (keyLevel[l_id][level] < key.getSymbolCount()) {

                QString txt = symbol.getKeySymbol(key.getSymbol(keyLevel[l_id][level]));

                QColor txtColor = txt[0] == -1 ? unknownSymbolColor : color[level];

                painter.setPen(txtColor);

                painter.drawText(temp[cordinate[level]].x() + xOffset[level]*scaleFactor / 2.5, temp[cordinate[level]].y() + yOffset[level]*scaleFactor / 2.5, szx, szy, Qt::AlignTop, txt);

                QString currentSymbol = key.getSymbol(keyLevel[l_id][level]);
                currentSymbol = currentSymbol.size() < 3 ? currentSymbol.append("\t") : currentSymbol;

                if (level == 0) {
                    tip.append(currentSymbol);
                } else {
                    tip.append("\n" + currentSymbol);
                }
            }
        }

        for (int i = 0 ; i < 4; i++) {
            tooltipX += temp[i].x();
            toolTipY += temp[i].y();
        }

        tooltipX = tooltipX / 4;
        toolTipY = toolTipY / 4;
        QPoint tooltipPoint = QPoint(tooltipX, toolTipY);

        tooltip.append(tip);
        tipPoint.append(tooltipPoint);
    } else {
        painter.setPen(Qt::black);

        if (name.contains(fkKey)) {
            QString tempName = name;
            tempName.remove("K");
            painter.drawText(temp[0].x() + s.size(0) - 10, temp[0].y() + 3 * scaleFactor * s.size(1) / 5, tempName);
        } else {
            painter.setFont(kbfont);
            painter.drawText(temp[0].x() + s.size(0) - 10, temp[0].y() + 3 * scaleFactor * s.size(1) / 5, name);
        }
        tip = name;

        for (int i = 0 ; i < 4; i++) {
            tooltipX += temp[i].x();
            toolTipY += temp[i].y();
        }

        tooltipX = tooltipX / 4;
        toolTipY = toolTipY / 4;
        QPoint tooltipPoint = QPoint(tooltipX, toolTipY);

        tooltip.append(tip);
        tipPoint.append(tooltipPoint);
    }
}


//draws key shape on QFrame called only by paint event
void KbPreviewFrame::drawShape(QPainter &painter, const GShape &s, int x, int y, int i, const QString &name)
{
    painter.setPen(Qt::black);
    int cordi_count = s.getCordi_count();

    if (geometry.sectionList[i].getAngle() == 0) {
        if (cordi_count == 1) {
            int width = s.getCordii(0).x();
            int height = s.getCordii(0).y();

            painter.drawRoundedRect(scaleFactor * x + 2, scaleFactor * y, scaleFactor * width, scaleFactor * height, 4, 4);

            QPoint temp[4];

            temp[0] = QPoint(scaleFactor * x, scaleFactor * y);
            temp[1] = QPoint(scaleFactor * (s.getCordii(0).x() + x), scaleFactor * y);
            temp[2] = QPoint(scaleFactor * (s.getCordii(0).x() + x), scaleFactor * (s.getCordii(0).y() + y));
            temp[3] = QPoint(scaleFactor * (x), scaleFactor * (s.getCordii(0).y() + y));

            drawKeySymbols(painter, temp, s, name);
        } else {
            QPoint temp[cordi_count];

            for (int i = 0; i < cordi_count; i++) {
                temp[i].setX(scaleFactor * (s.getCordii(i).x() + x + 1));
                temp[i].setY(scaleFactor * (s.getCordii(i).y() + y + 1));
            }

            painter.drawPolygon(temp, cordi_count);
            drawKeySymbols(painter, temp, s, name);
        }
    } else {
        QPoint temp[cordi_count == 1 ? 4 : cordi_count];
        int size;

        if (cordi_count == 1) {
            temp[0] = QPoint(x, y);
            temp[1] = QPoint(s.getCordii(0).x() + x, y);
            temp[2] = QPoint(s.getCordii(0).x() + x, s.getCordii(0).y() + y);
            temp[3] = QPoint(x, s.getCordii(0).y() + y);
            size = 4;
        } else {
            size = cordi_count;

            for (int i = 0; i < cordi_count; i++) {
                temp[i].setX((s.getCordii(i).x() + x + 1));
                temp[i].setY((s.getCordii(i).y() + y + 1));
            }
        }

        double refX, refY;

        refX = geometry.sectionList[i].getLeft();
        refY = geometry.sectionList[i].getTop();

        //qCDebug(KEYBOARD_PREVIEW) <<"\ntransform";
        for (int j = 0; j < size; j++) {
            double x = temp[j].x() - refX;
            double y = temp[j].y() - refY;

            //qCDebug(KEYBOARD_PREVIEW) <<"(" <<x <<"," <<y <<")->";

            float theta = (3.1459 * geometry.sectionList[i].getAngle()) / 180;
            double x_ = x * cos(theta) - y * sin(theta);

            //qCDebug(KEYBOARD_PREVIEW) <<"x_= " <<x <<"*" <<cos(theta) <<"-" <<y <<"*" <<sin(theta);

            double y_ = x * sin(theta) + y * cos(theta);

            //qCDebug(KEYBOARD_PREVIEW) <<"\ny_= " <<x <<"*" <<sin(theta) <<"+" <<y <<"*" <<cos(theta);
            //qCDebug(KEYBOARD_PREVIEW) <<"(" <<x_ <<"," <<y_ <<")\n";

            temp[j] = QPoint(scaleFactor * (x_ + refX), scaleFactor * (y_ + refY));
        }

        /*for(int i=0;i<size;i++){
            qCDebug(KEYBOARD_PREVIEW) <<temp[i];
        }*/

        painter.drawPolygon(temp, size);
        drawKeySymbols(painter, temp, s, name);
    }

}


//event handling for tooltip
bool KbPreviewFrame::event(QEvent *event)
{

    if (event->type() == QEvent::ToolTip) {
        QHelpEvent *helpEvent = static_cast<QHelpEvent *>(event);
        int index = itemAt(helpEvent->pos());

        if (index != -1) {
            QToolTip::showText(helpEvent->globalPos(), tooltip.at(index));
        } else {
            QToolTip::hideText();
            event->ignore();
        }

        return true;
    }
    return QWidget::event(event);
}


void KbPreviewFrame::paintEvent(QPaintEvent *)
{
    if (geometry.getParsing() && keyboardLayout.getParsedSymbol()) {
        QPainter painter(this);

        QFont kbfont;
        kbfont.setPointSize(9);

        painter.setFont(kbfont);
        painter.setBrush(QBrush("#C3C8CB"));
        painter.setRenderHint(QPainter::Antialiasing);

        const int strtx = 0, strty = 0, endx = geometry.getWidth(), endy = geometry.getHeight();


        painter.setPen("#EDEEF2");

        painter.drawRect(strtx, strty, scaleFactor * endx + 60, scaleFactor * endy + 60);

        painter.setPen(Qt::black);
        painter.setBrush(QBrush("#EDEEF2"));

        for (int i = 0; i < geometry.getSectionCount(); i++) {

            painter.setPen(Qt::black);

            for (int j = 0; j < geometry.sectionList[i].getRowCount(); j++) {

                int keyn = geometry.sectionList[i].rowList[j].getKeyCount();

                for (int k = 0; k < keyn; k++) {

                    Key temp = geometry.sectionList[i].rowList[j].keyList[k];

                    int x = temp.getPosition().x();
                    int y = temp.getPosition().y();

                    GShape s;

                    s = geometry.findShape(temp.getShapeName());
                    QString name = temp.getName();

                    drawShape(painter, s, x, y, i, name);

                }
            }
        }

        if (symbol.isFailed()) {
            painter.setPen(keyBorderColor);
            painter.drawRect(strtx, strty, endx, endy);

            const int midx = 470, midy = 240;
            painter.setPen(lev12color);
            painter.drawText(midx, midy, i18n("No preview found"));
        }
    } else {
        QMessageBox errorBox;
        errorBox.setText("Unable to open Preview !");
        errorBox.exec();
    }

}

// this function draws the keyboard preview on a QFrame
void KbPreviewFrame::generateKeyboardLayout(const QString &layout, const QString &layoutVariant, const QString &model)
{
    geometry = grammar::parseGeometry(model);
    int endx = geometry.getWidth(), endy = geometry.getHeight();

    QDesktopWidget *desktopWidget = qApp->desktop();
    QRect screenGeometry = desktopWidget->screenGeometry();
    int screenWidth = screenGeometry.width();

    scaleFactor = 2.5;
    while (scaleFactor * endx + screenWidth / 20 > screenWidth) {
        scaleFactor -= 0.2;
    }
    qCDebug(KEYBOARD_PREVIEW) << "scale factor: 2.5 ->" << scaleFactor;

    setFixedSize(scaleFactor * endx + 60, scaleFactor * endy + 60);
    qCDebug(KEYBOARD_PREVIEW) << screenWidth << ":" << scaleFactor << scaleFactor *endx + 60 << scaleFactor *endy + 60;
    keyboardLayout = grammar::parseSymbols(layout, layoutVariant);
}


//this functions give the index of the tooltip over which mouse hovers
int KbPreviewFrame::itemAt(const QPoint &pos)
{
    int distance =  10000;
    int closest = 0;
    for (int i = 0; i < tipPoint.size(); i++) {

        int temp = sqrt((pos.x() - tipPoint.at(i).x()) * (pos.x() - tipPoint.at(i).x()) + (pos.y() - tipPoint.at(i).y()) * (pos.y() - tipPoint.at(i).y()));

        if (distance > temp) {
            distance = temp;
            closest = i;
        }
    }

    if (distance < 25) {
        return closest;
    }

    return -1;
}
