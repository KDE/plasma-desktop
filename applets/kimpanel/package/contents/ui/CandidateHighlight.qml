/*
    SPDX-FileCopyrightText: 2017 Weng Xuetian <wengxt@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
import QtQuick
import org.kde.kirigami as Kirigami
import org.kde.ksvg as KSvg

Item {
    id: highlight

    property bool selected: false
    property alias marginHints: background.margins;

    Behavior on opacity {
        NumberAnimation {
            duration: Kirigami.Units.longDuration
            easing.type: Easing.OutQuad
        }
    }

    KSvg.FrameSvgItem {
        id: background
        imagePath: "widgets/viewitem"
        prefix: {
            if (highlight.selected)
                return "hover";

            return "normal";
        }

        anchors {
            fill: parent
        }
    }
}
