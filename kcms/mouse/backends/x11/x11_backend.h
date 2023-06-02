/*
    SPDX-FileCopyrightText: 2017 Xuetian Weng <wengxt@gmail.com>
    SPDX-FileCopyrightText: 2018 Roman Gilg <subdiff@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "inputbackend.h"

#include <QtGui/private/qtx11extras_p.h>

#include <X11/Xdefs.h>

class X11Backend : public InputBackend
{
    Q_OBJECT

public:
    static X11Backend *implementation(QObject *parent = nullptr);
    ~X11Backend();

    void kcmInit() override;

    bool isValid() const override
    {
        return m_dpy != nullptr;
    }

    QString currentCursorTheme();
    void applyCursorTheme(const QString &name, int size);

protected:
    X11Backend(QObject *parent = nullptr);

    // We may still need to do something on non-X11 platform due to Xwayland.
    Display *m_dpy = nullptr;
    bool m_platformX11;
};
