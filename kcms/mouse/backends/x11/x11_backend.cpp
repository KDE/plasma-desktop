/*
    SPDX-FileCopyrightText: 2017 Xuetian Weng <wengxt@gmail.com>
    SPDX-FileCopyrightText: 2018 Roman Gilg <subdiff@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "x11_backend.h"
#include "x11_libinput_backend.h"

#include "logging.h"

#include <config-X11.h>

#include <libinput-properties.h>

#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>
#include <KSharedConfig>
#include <QFile>

#include <X11/Xlib.h>

X11Backend *X11Backend::implementation(QObject *parent)
{
    auto dpy = QX11Info::display();
    Atom testAtom = XInternAtom(dpy, LIBINPUT_PROP_ACCEL, True);

    if (testAtom) {
        qCDebug(KCM_MOUSE) << "Using libinput driver on X11.";
        return new X11LibinputBackend(parent);
    }

    return nullptr;
}

X11Backend::X11Backend(QObject *parent)
    : InputBackend(parent)
{
}

#include "moc_x11_backend.cpp"
