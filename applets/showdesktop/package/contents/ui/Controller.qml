/*
    SPDX-FileCopyrightText: 2022 ivan (@ratijas) tkachenko <me@ratijas.tk>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQml 2.15

QtObject {
    /**
     * Whether the effect is currently active, and can be deactivated.
     */
    property bool active

    property string titleActive
    property string titleInactive

    property string descriptionActive
    property string descriptionInactive

    readonly property string title: active ? titleActive : titleInactive
    readonly property string description: active ? descriptionActive : descriptionInactive

    // virtual
    function toggle() {}
}
