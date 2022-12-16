/*
    SPDX-FileCopyrightText: 2018 Roman Gilg <subdiff@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QString>

struct LibinputSettings {
    template<class T>
    T load(QString key, T defVal);

    template<class T>
    void save(QString key, T val);
};
