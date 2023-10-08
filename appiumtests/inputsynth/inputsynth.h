/*
    SPDX-FileCopyrightText: 2023 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "inputsynth_export.h"

extern "C" {
INPUTSYNTH_EXPORT void init_application();
INPUTSYNTH_EXPORT void unload_application();

INPUTSYNTH_EXPORT void init_fake_input();
INPUTSYNTH_EXPORT void key_press(int keycode);
INPUTSYNTH_EXPORT void key_release(int keycode);
}
