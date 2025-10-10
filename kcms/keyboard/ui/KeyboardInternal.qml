/*
    SPDX-FileCopyrightText: 2024 Evgeny Chesnokov <echesnokov@astralinux.ru>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick

QtObject {

    readonly property string keyboardRepeatRepeat: "repeat"
    readonly property string keyboardRepeatNothing: "nothing"
    readonly property string keyboardRepeatAccent: "accent"

    // Default value for layoutLoopCount (e.g. we cannot use layout looping feature)
    readonly property int noLooping: -1

    // Minimum number of layouts that can be used with layout looping feature
    readonly property int minimumLoopingCount: 2

    // Minimum number of layouts currently available (X.org only allows to have 4 layouts)
    readonly property int minimumAvailableLoops: Math.min(kcm.maxGroupCount, list.count - 1)
}
