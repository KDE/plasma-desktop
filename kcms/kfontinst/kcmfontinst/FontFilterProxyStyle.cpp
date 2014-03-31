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

#include <QWidget>

#include "FontFilterProxyStyle.h"

namespace KFI
{

CFontFilterProxyStyle::CFontFilterProxyStyle(QWidget *parent)
    : QStyle(), parent(parent)
{
    setParent(parent);
}

QStyle *CFontFilterProxyStyle::style() const
{
    return parent->parentWidget()->style();
}

void CFontFilterProxyStyle::drawComplexControl(ComplexControl control, const QStyleOptionComplex *option,
                                        QPainter *painter, const QWidget *widget) const
{
    style()->drawComplexControl(control, option, painter, widget);
}

void CFontFilterProxyStyle::drawControl(ControlElement element, const QStyleOption *option, QPainter *painter,
                                 const QWidget *widget) const
{
    style()->drawControl(element, option, painter, widget);
}

void CFontFilterProxyStyle::drawItemPixmap(QPainter *painter, const QRect &rectangle, int alignment,
                                    const QPixmap &pixmap) const
{
    style()->drawItemPixmap(painter, rectangle, alignment, pixmap);
}

void CFontFilterProxyStyle::drawItemText(QPainter *painter, const QRect &rectangle, int alignment, const QPalette &palette,
                                  bool enabled, const QString &text, QPalette::ColorRole textRole) const
{
    style()->drawItemText(painter, rectangle, alignment, palette, enabled, text, textRole);
}

void CFontFilterProxyStyle::drawPrimitive(PrimitiveElement element, const QStyleOption *option, QPainter *painter,
                                   const QWidget *widget) const
{
    style()->drawPrimitive(element, option, painter, widget);
}

QPixmap CFontFilterProxyStyle::generatedIconPixmap(QIcon::Mode iconMode, const QPixmap &pixmap,
                                            const QStyleOption *option) const
{
    return style()->generatedIconPixmap(iconMode, pixmap, option);
}

QStyle::SubControl CFontFilterProxyStyle::hitTestComplexControl(ComplexControl control, const QStyleOptionComplex *option,
                                                         const QPoint &position, const QWidget *widget) const
{
    return style()->hitTestComplexControl(control, option, position, widget);
}

QRect CFontFilterProxyStyle::itemPixmapRect(const QRect &rectangle, int alignment, const QPixmap &pixmap) const
{
    return style()->itemPixmapRect(rectangle, alignment, pixmap);
}

QRect CFontFilterProxyStyle::itemTextRect(const QFontMetrics &metrics, const QRect &rectangle, int alignment,
                                   bool enabled, const QString &text) const
{
    return style()->itemTextRect(metrics, rectangle, alignment, enabled, text);
}

int CFontFilterProxyStyle::pixelMetric(PixelMetric metric, const QStyleOption *option, const QWidget *widget) const
{
    return style()->pixelMetric(metric, option, widget);
}

void CFontFilterProxyStyle::polish(QWidget *widget)
{
    style()->polish(widget);
}

void CFontFilterProxyStyle::polish(QApplication *application)
{
    style()->polish(application);
}

void CFontFilterProxyStyle::polish(QPalette &palette)
{
    style()->polish(palette);
}

QSize CFontFilterProxyStyle::sizeFromContents(ContentsType type, const QStyleOption *option,
                                       const QSize &contentsSize, const QWidget *widget) const
{
    return style()->sizeFromContents(type, option, contentsSize, widget);
}

QIcon CFontFilterProxyStyle::standardIcon(StandardPixmap standardIcon, const QStyleOption *option,
                                   const QWidget *widget) const
{
    return style()->standardIcon(standardIcon, option, widget);
}

QPixmap CFontFilterProxyStyle::standardPixmap(StandardPixmap standardPixmap, const QStyleOption *option,
                                       const QWidget *widget) const
{
    return style()->standardPixmap(standardPixmap, option, widget);
}

QPalette CFontFilterProxyStyle::standardPalette() const
{
    return style()->standardPalette();
}

int CFontFilterProxyStyle::styleHint(StyleHint hint, const QStyleOption *option, const QWidget *widget,
                              QStyleHintReturn *returnData) const
{
    return style()->styleHint(hint, option, widget, returnData);
}

QRect CFontFilterProxyStyle::subControlRect(ComplexControl control, const QStyleOptionComplex *option,
                                     SubControl subControl, const QWidget *widget) const
{
    return style()->subControlRect(control, option, subControl, widget);
}

QRect CFontFilterProxyStyle::subElementRect(SubElement element, const QStyleOption *option,
                                     const QWidget *widget) const
{
    return style()->subElementRect(element, option, widget);
}

void CFontFilterProxyStyle::unpolish(QWidget *widget)
{
    style()->unpolish(widget);
}

void CFontFilterProxyStyle::unpolish(QApplication *application)
{
    style()->unpolish(application);
}

int CFontFilterProxyStyle::layoutSpacing(QSizePolicy::ControlType control1, QSizePolicy::ControlType control2, Qt::Orientation orientation, const QStyleOption* option, const QWidget* widget) const
{
    return style()->layoutSpacing(control1, control2, orientation, option, widget);
}

}

