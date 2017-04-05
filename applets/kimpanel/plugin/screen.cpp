/*
 * Copyright 2014 Weng Xuetian <wengxt@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 */

#include <limits.h>
#include <QGuiApplication>
#include <QScreen>
#include "screen.h"

int pointToRect(int x, int y, const QRect& r)
{
    int dx = 0;
    int dy = 0;
    if (x < r.left()) {
        dx = r.left() -x;
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

Screen::Screen(QObject* parent) : QObject(parent)
{
}

Screen::~Screen()
{
}

QScreen *screenForPoint(int x, int y) {
    auto screens = qApp->screens();
    QScreen* closestScreen = nullptr;
    int shortestDistance = INT_MAX;
    foreach(QScreen* screen, screens) {
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

