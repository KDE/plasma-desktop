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
                            const QWidget *widget) const Q_DECL_OVERRIDE;
    void drawControl(ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const Q_DECL_OVERRIDE;
    void drawItemPixmap(QPainter *painter, const QRect &rectangle, int alignment, const QPixmap &pixmap) const Q_DECL_OVERRIDE;
    void drawItemText(QPainter *painter, const QRect &rectangle, int alignment, const QPalette &palette, bool enabled,
                      const QString &text, QPalette::ColorRole textRole) const Q_DECL_OVERRIDE;
    void drawPrimitive(PrimitiveElement element, const QStyleOption *option, QPainter *painter,
                       const QWidget *widget) const Q_DECL_OVERRIDE;
    QPixmap generatedIconPixmap(QIcon::Mode iconMode, const QPixmap &pixmap, const QStyleOption *option) const Q_DECL_OVERRIDE;
    SubControl hitTestComplexControl(ComplexControl control, const QStyleOptionComplex *option, const QPoint &position,
                                     const QWidget *widget) const Q_DECL_OVERRIDE;
    QRect itemPixmapRect(const QRect &rectangle, int alignment, const QPixmap &pixmap) const Q_DECL_OVERRIDE;
    QRect itemTextRect(const QFontMetrics &metrics, const QRect &rectangle, int alignment,
                       bool enabled, const QString &text) const Q_DECL_OVERRIDE;
    int pixelMetric(PixelMetric metric, const QStyleOption *option, const QWidget *widget) const Q_DECL_OVERRIDE;
    void polish(QWidget *widget) Q_DECL_OVERRIDE;
    void polish(QApplication *application) Q_DECL_OVERRIDE;
    void polish(QPalette &palette) Q_DECL_OVERRIDE;
    QSize sizeFromContents(ContentsType type, const QStyleOption *option, const QSize &contentsSize,
                           const QWidget *widget) const Q_DECL_OVERRIDE;
    QIcon standardIcon(StandardPixmap standardIcon, const QStyleOption *option, const QWidget *widget) const Q_DECL_OVERRIDE;
    QPixmap standardPixmap(StandardPixmap standardPixmap, const QStyleOption *option, const QWidget *widget) const Q_DECL_OVERRIDE;
    QPalette standardPalette() const Q_DECL_OVERRIDE;
    int styleHint(StyleHint hint, const QStyleOption *option, const QWidget *widget, QStyleHintReturn *returnData) const Q_DECL_OVERRIDE;
    QRect subControlRect(ComplexControl control, const QStyleOptionComplex *option, SubControl subControl,
                         const QWidget *widget) const Q_DECL_OVERRIDE;
    QRect subElementRect(SubElement element, const QStyleOption *option, const QWidget *widget) const Q_DECL_OVERRIDE;
    void unpolish(QWidget *widget) Q_DECL_OVERRIDE;
    void unpolish(QApplication *application) Q_DECL_OVERRIDE;
    virtual int layoutSpacing(QSizePolicy::ControlType control1, QSizePolicy::ControlType control2,
                              Qt::Orientation orientation, const QStyleOption* option = 0, const QWidget* widget = 0) const override;

protected:
    QWidget *parent;
};

}
#endif

