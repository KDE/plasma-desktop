/*
    SPDX-FileCopyrightText: 2013 Alexander Mezin <mezin.alexander@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QVersionNumber>
#include <QtGui/private/qtx11extras_p.h>

#include <X11/Xdefs.h>

#include <memory>

void XDeleter(void *p);

struct PropertyInfo {
    Atom type;
    int format;
    std::shared_ptr<unsigned char> data;
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
