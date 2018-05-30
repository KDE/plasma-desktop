/*
 * Copyright 2017 Xuetian Weng <wengxt@gmail.com>
 * Copyright 2018 Roman Gilg <subdiff@gmail.com>
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

#ifndef X11BACKEND_H
#define X11BACKEND_H

#include "inputbackend.h"

#include <QX11Info>
#include <X11/Xdefs.h>

class X11Backend : public InputBackend
{
    Q_OBJECT

public:
    static X11Backend *implementation(QObject *parent = nullptr);
    ~X11Backend();

    void kcmInit() override;

    bool isValid() const override { return m_dpy != nullptr; }

    QString currentCursorTheme();
    void applyCursorTheme(const QString &name, int size);

protected:
    X11Backend(QObject *parent = nullptr);

    // We may still need to do something on non-X11 platform due to Xwayland.
    Display* m_dpy = nullptr;
    bool m_platformX11;
};

#endif // X11BACKEND_H
