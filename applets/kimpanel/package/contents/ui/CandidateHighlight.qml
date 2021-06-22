/*
    SPDX-FileCopyrightText: 2017 Weng Xuetian <wengxt@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
import QtQuick 2.1
import org.kde.plasma.core 2.0 as PlasmaCore

Item {
    id: highlight

    property bool selected: false
    property alias marginHints: background.margins;

    Behavior on opacity {
        NumberAnimation {
            duration: PlasmaCore.Units.longDuration
            easing.type: Easing.OutQuad
        }
    }

    PlasmaCore.FrameSvgItem {
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
