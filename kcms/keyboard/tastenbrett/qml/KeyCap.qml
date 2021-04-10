/*
    Copyright 2019 Harald Sitter <sitter@kde.org>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of
    the License or (at your option) version 3 or any later version
    accepted by the membership of KDE e.V. (or its successor approved
    by the membership of KDE e.V.), which shall act as a proxy
    defined in Section 14 of version 3 of the license.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
