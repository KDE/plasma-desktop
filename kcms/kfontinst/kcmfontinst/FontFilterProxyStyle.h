/*  This file is part of the KDE project

    Copyright (C) 2007 Fredrik Höglund <fredrik@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License (LGPL) as published by the Free Software Foundation;
    either version 2 of the License, or (at your option) any later
    version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/
#ifndef CFONTFILTERPROXYSTYLE_H
#define CFONTFILTERPROXYSTYLE_H
#include <QStyle>

namespace KFI
{

class CFontFilterProxyStyle : public QStyle
{
public:
    CFontFilterProxyStyle(QWidget *parent);
    QStyle *style() const;
    void drawComplexControl(ComplexControl control, const QStyleOptionComplex *option, QPainter *painter,
                            const QWidget *widget) const;
    void drawControl(ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const;
    void drawItemPixmap(QPainter *painter, const QRect &rectangle, int alignment, const QPixmap &pixmap) const;
    void drawItemText(QPainter *painter, const QRect &rectangle, int alignment, const QPalette &palette, bool enabled,
                      const QString &text, QPalette::ColorRole textRole) const;
    void drawPrimitive(PrimitiveElement element, const QStyleOption *option, QPainter *painter,
                       const QWidget *widget) const;
    QPixmap generatedIconPixmap(QIcon::Mode iconMode, const QPixmap &pixmap, const QStyleOption *option) const;
    SubControl hitTestComplexControl(ComplexControl control, const QStyleOptionComplex *option, const QPoint &position,
                                     const QWidget *widget) const;
    QRect itemPixmapRect(const QRect &rectangle, int alignment, const QPixmap &pixmap) const;
    QRect itemTextRect(const QFontMetrics &metrics, const QRect &rectangle, int alignment,
                       bool enabled, const QString &text) const;
    int pixelMetric(PixelMetric metric, const QStyleOption *option, const QWidget *widget) const;
    void polish(QWidget *widget);
    void polish(QApplication *application);
    void polish(QPalette &palette);
    QSize sizeFromContents(ContentsType type, const QStyleOption *option, const QSize &contentsSize,
                           const QWidget *widget) const;
    QIcon standardIcon(StandardPixmap standardIcon, const QStyleOption *option, const QWidget *widget) const;
    QPixmap standardPixmap(StandardPixmap standardPixmap, const QStyleOption *option, const QWidget *widget) const;
    QPalette standardPalette() const;
    int styleHint(StyleHint hint, const QStyleOption *option, const QWidget *widget, QStyleHintReturn *returnData) const;
    QRect subControlRect(ComplexControl control, const QStyleOptionComplex *option, SubControl subControl,
                         const QWidget *widget) const;
    QRect subElementRect(SubElement element, const QStyleOption *option, const QWidget *widget) const;
    void unpolish(QWidget *widget);
    void unpolish(QApplication *application);
    virtual int layoutSpacing(QSizePolicy::ControlType control1, QSizePolicy::ControlType control2,
                              Qt::Orientation orientation, const QStyleOption* option = 0, const QWidget* widget = 0) const override;

protected:
    QWidget *parent;
};

}
#endif

