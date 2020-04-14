/*
 * Copyright (C) 2013 Alexander Mezin <mezin.alexander@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef PROPERTYINFO_H
#define PROPERTYINFO_H

#include <QSharedPointer>
#include <QX11Info>
#include <X11/Xdefs.h>

void XDeleter(void *p);

struct PropertyInfo
{
    Atom type;
    int format;
    QSharedPointer<unsigned char> data;
    unsigned long nitems;

    float *f;
    int *i;
    char *b;

    Display *display;
    int device;
    Atom prop;

    PropertyInfo();
    PropertyInfo(Display *display, int device, Atom prop, Atom floatType);
    QVariant value(unsigned offset) const;

    void set();
};

#endif // PROPERTYINFO_H
