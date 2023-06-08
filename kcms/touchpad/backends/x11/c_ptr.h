/*
    SPDX-FileCopyrightText: 2022 Xaver Hugl <xaver.hugl@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#pragma once

struct CDeleter {
    template<typename T>
    void operator()(T *ptr)
    {
        if (ptr) {
            free(ptr);
        }
    }
};
