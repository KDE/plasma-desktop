/*
    SPDX-FileCopyrightText: 2018 Roman Gilg <subdiff@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef LIBINPUTSETTINGS_H
#define LIBINPUTSETTINGS_H

#include <QString>

struct LibinputSettings {
    template<class T>
    T load(QString key, T defVal);

    template<class T>
    void save(QString key, T val);
};

#endif // LIBINPUTSETTINGS_H
