/*
    SPDX-FileCopyrightText: 2017 Xuetian Weng <wengxt@gmail.com>
    SPDX-FileCopyrightText: 2018 Roman Gilg <subdiff@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef EVDEVSETTINGS_H
#define EVDEVSETTINGS_H

class X11EvdevBackend;

enum class Handed {
    Right = 0,
    Left = 1,
    NotSupported = -1,
};

struct EvdevSettings {
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
