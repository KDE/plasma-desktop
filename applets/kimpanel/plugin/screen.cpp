/*
    SPDX-FileCopyrightText: 2014 Weng Xuetian <wengxt@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "screen.h"
#include <QGuiApplication>
#include <QScreen>
#include <limits.h>

int pointToRect(int x, int y, const QRect &r)
{
    int dx = 0;
    int dy = 0;
    if (x < r.left()) {
        dx = r.left() - x;
    } else if (x > r.right()) {
        dx = x - r.right();
    }
    if (y < r.top()) {
        dy = r.top() - y;
    } else if (y > r.bottom()) {
        dy = y - r.bottom();
    }
    return dx + dy;
}

Screen::Screen(QObject *parent)
    : QObject(parent)
{
}

Screen::~Screen()
{
}

QScreen *screenForPoint(int x, int y)
{
    auto screens = qApp->screens();
    QScreen *closestScreen = nullptr;
    int shortestDistance = INT_MAX;
    foreach (QScreen *screen, screens) {
        auto rect = screen->availableGeometry();
        rect.setSize(rect.size() * screen->devicePixelRatio());
        int thisDistance = pointToRect(x, y, rect);
        if (thisDistance < shortestDistance) {
            shortestDistance = thisDistance;
            closestScreen = screen;
        }
    }

    if (!closestScreen) {
        closestScreen = qApp->primaryScreen();
    }

    return closestScreen;
}

QRect Screen::geometryForPoint(int x, int y)
{
    auto closestScreen = screenForPoint(x, y);

    if (closestScreen) {
        auto rect = closestScreen->availableGeometry();
        rect.setSize(rect.size() * closestScreen->devicePixelRatio());
        return rect;
    }
    return QRect();
}

qreal Screen::devicePixelRatioForPoint(int x, int y)
{
    auto closestScreen = screenForPoint(x, y);

    if (closestScreen) {
        return closestScreen->devicePixelRatio();
    }
    return 1.0;
}
