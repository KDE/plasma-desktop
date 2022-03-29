/*
    SPDX-FileCopyrightText: 2017 Xuetian Weng <wengxt@gmail.com>
    SPDX-FileCopyrightText: 2018 Roman Gilg <subdiff@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef X11BACKEND_H
#define X11BACKEND_H

#include "inputbackend.h"

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QX11Info>
#else
#include <QtGui/private/qtx11extras_p.h>
#endif
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

#endif // X11BACKEND_H
