/*
    SPDX-FileCopyrightText: 2017 Weng Xuetian <wengxt@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
import QtQuick 2.1
import org.kde.kirigami 2.20 as Kirigami
import org.kde.ksvg 1.0 as KSvg

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
            if (selected)
                return "hover";

            return "normal";
        }

        anchors {
            fill: parent
        }
    }
}
