/*
    SPDX-FileCopyrightText: 2013 Alexander Mezin <mezin.alexander@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "propertyinfo.h"

#include <QVariant>

#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/extensions/XInput2.h>

void XDeleter(void *p)
{
    if (p) {
        XFree(p);
    }
}

PropertyInfo::PropertyInfo()
    : type(0)
    , format(0)
    , nitems(0)
    , f(nullptr)
    , i(nullptr)
    , b(nullptr)
    , display(nullptr)
    , device(0)
    , prop(0)
{
}

PropertyInfo::PropertyInfo(Display *display, int device, Atom prop, Atom floatType)
    : type(0)
    , format(0)
    , nitems(0)
    , f(nullptr)
    , i(nullptr)
    , b(nullptr)
    , display(display)
    , device(device)
    , prop(prop)
{
    unsigned char *dataPtr = nullptr;
    unsigned long bytes_after;
    XIGetProperty(display, device, prop, 0, 1000, False, AnyPropertyType, &type, &format, &nitems, &bytes_after, &dataPtr);
    data = QSharedPointer<unsigned char>(dataPtr, XDeleter);

    if (format == CHAR_BIT && type == XA_INTEGER) {
        b = reinterpret_cast<char *>(dataPtr);
    }
    if (format == sizeof(int) * CHAR_BIT && (type == XA_INTEGER || type == XA_CARDINAL)) {
        i = reinterpret_cast<int *>(dataPtr);
    }
    if (format == sizeof(float) * CHAR_BIT && floatType && type == floatType) {
        f = reinterpret_cast<float *>(dataPtr);
    }
}

QVariant PropertyInfo::value(unsigned offset) const
{
    QVariant v;
    if (offset >= nitems) {
        return v;
    }

    if (b) {
        v = QVariant(static_cast<int>(b[offset]));
    }
    if (i) {
        v = QVariant(i[offset]);
    }
    if (f) {
        v = QVariant(f[offset]);
    }

    return v;
}

void PropertyInfo::set()
{
    XIChangeProperty(display, device, prop, type, format, XIPropModeReplace, data.data(), nitems);
}
