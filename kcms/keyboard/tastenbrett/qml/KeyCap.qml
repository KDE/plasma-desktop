/*
    SPDX-FileCopyrightText: 2019 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

import QtQuick 2.12

Item {
    property QtObject key

    // NB: we scale hight only by a portion of the available space if
    //   some of the levels aren't there. Visually it looks weird when some
    //   labels are full center, so let's still bias them towards an edge, but
    //   increase the height ever so slightly

    KeyCapLabel {
        id: topLeft
        width: topRight.text == "" ? parent.width : parent.width / 2.0
        horizontalAlignment: Text.AlignLeft
        height: (bottomLeft.text == "" && bottomRight.text == "") ? parent.height / 1.7 : parent.height  /2.0
        anchors.top: parent.top
        anchors.left: parent.left
        text: key.cap.topLeft
    }
    KeyCapLabel {
        id: topRight
        width: parent.width /2.0
        horizontalAlignment: Text.AlignRight
        height: parent.height  /2.0
        anchors.top: parent.top
        anchors.right: parent.right
        text: key.cap.topRight
    }
    KeyCapLabel {
        id: bottomLeft
        width: bottomRight.text == "" ? parent.width : parent.width / 2.0
        horizontalAlignment: Text.AlignLeft
        height: (topLeft.text == "" && topRight.text == "") ? parent.height / 1.7 : parent.height  /2.0
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        text: key.cap.bottomLeft
    }
    KeyCapLabel {
        id: bottomRight
        width: parent.width /2.0
        horizontalAlignment: Text.AlignRight
        height: parent.height  /2.0
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        text: key.cap.bottomRight
    }
}
