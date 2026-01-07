/*
 *  SPDX-FileCopyrightText: 2025 Shubham Arora <contact@shubhamarora.dev>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

pragma Singleton
import QtQuick

QtObject {
    enum WidthStrategy {
        Automatic = 0,
        Fixed = 1,
        FixedMaximum = 2
    }
}
