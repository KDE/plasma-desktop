/*
    SPDX-FileCopyrightText: 2014 Aleix Pol Gonzalez <aleixpol@blue-systems.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.0
import org.kde.ksvg 1.0 as KSvg
import org.kde.kirigami 2.20 as Kirigami
import "../osd"

KSvg.FrameSvgItem {
    id: osd

    property alias timeout: osdItem.timeout
    property alias osdValue: osdItem.osdValue
    property alias osdMaxValue: osdItem.osdMaxValue
    property alias icon: osdItem.icon
    property alias showingProgress: osdItem.showingProgress

    objectName: "onScreenDisplay"
    visible: false
    width: osdItem.width + margins.left + margins.right
    height: osdItem.height + margins.top + margins.bottom
    imagePath: "dialogs/background"

    Kirigami.Theme.inherit: false
    Kirigami.Theme.colorSet: Kirigami.Theme.Window

    function show() {
        osd.visible = true;
        hideAnimation.restart();
    }

    // avoid leaking ColorScope of lock screen theme into the OSD "popup"
    Item {
        width: osdItem.width
        height: osdItem.height
        anchors.centerIn: parent

        OsdItem {
            id: osdItem
        }
    }

    SequentialAnimation {
        id: hideAnimation
        // prevent press and hold from flickering
        PauseAnimation { duration: Kirigami.Units.shortDuration }
        NumberAnimation {
            target: osd
            property: "opacity"
            from: 1
            to: 0
            duration: osd.timeout
            easing.type: Easing.InQuad
        }
        ScriptAction {
            script: {
                osd.visible = false;
                osd.opacity = 1;
                osd.icon = "";
                osd.osdValue = 0;
            }
        }
    }
}
