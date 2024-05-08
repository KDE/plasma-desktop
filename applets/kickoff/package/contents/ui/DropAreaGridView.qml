/*
 * SPDX-FileCopyrightText: 2021 Noah Davis <noahadvs@gmail.com>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

pragma ComponentBehavior: Bound

import QtQuick

KickoffGridView {
    id: root
    KickoffDropArea {
        z: -1
        parent: root
        anchors.fill: parent
        targetView: root.view
        scrollUpMargin: root.header.height * 2
        scrollDownMargin: root.footer.height * 2
    }
}
