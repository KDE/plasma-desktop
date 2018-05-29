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

#ifndef EVDEVSETTINGS_H
#define EVDEVSETTINGS_H

class X11EvdevBackend;

enum class Handed {
    Right = 0,
    Left = 1,
    NotSupported = -1
};

struct EvdevSettings
{
    void save();
    void load(X11EvdevBackend *);
    void apply(X11EvdevBackend *, bool force = false);

    bool handedEnabled;
    bool handedNeedsApply;
    Handed handed;
    double accelRate;
    int thresholdMove;
    int doubleClickInterval;
    int dragStartTime;
    int dragStartDist;
    bool singleClick;
    int wheelScrollLines;
    bool reverseScrollPolarity;
};

#endif // EVDEVSETTINGS_H
