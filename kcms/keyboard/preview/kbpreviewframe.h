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
#ifndef KBPREVIEWFRAME_H
#define KBPREVIEWFRAME_H

#include "keyboardlayout.h"

#include "keysymhelper.h"
#include "keyaliases.h"

#include <QPainter>
#include <QFrame>
#include <QHash>
#include <QToolTip>
#include <QLoggingCategory>


Q_DECLARE_LOGGING_CATEGORY(KEYBOARD_PREVIEW)


class Geometry;
class GShape;


class KbPreviewFrame : public QFrame
{
    Q_OBJECT

private:
    static const int width = 1100, height = 490;

    KeySymHelper symbol;
    Aliases alias;
    QStringList tooltip;
    QList <QPoint> tipPoint;
    int l_id;
    Geometry &geometry;
    float scaleFactor;
    KbLayout keyboardLayout;

    void drawKeySymbols(QPainter &painter, QPoint temp[], const GShape &s, const QString &name);
    void drawShape(QPainter &painter, const GShape &s, int x, int y, int i, const QString &name);

    int itemAt(const QPoint &pos);


protected:
    bool event(QEvent *event);

public:
    explicit KbPreviewFrame(QWidget *parent = 0);
    virtual ~KbPreviewFrame();
    void paintEvent(QPaintEvent *event);
    void generateKeyboardLayout(const QString &layout, const QString &layoutVariant, const QString &model);
    int getWidth() const;
    int getHeight() const;

    int getLevel()
    {
        return keyboardLayout.getLevel();
    }

    void setL_id(int lId)
    {
        l_id = lId;
        repaint();
    }

    QString getLayoutName() const
    {
        return keyboardLayout.getLayoutName();
    }

    float getScaleFactor()
    {
        return scaleFactor;
    }
};

#endif // KBPREVIEWFRAME_H 
