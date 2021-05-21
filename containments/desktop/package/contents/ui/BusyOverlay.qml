/*
    SPDX-FileCopyrightText: 2011-2013 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.0
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.extras 2.0 as PlasmaExtras

PlasmaComponents.BusyIndicator {
    id: busyIndicator

    anchors.centerIn: parent
    z: appletContainer.z + 1
    visible: applet.busy

    running: visible

    function disappear() {
        appearAnim.running = false;
        disappearAnim.running = true;
    }

    onRunningChanged: {
        if (running) {
            disappearAnim.running = false;
            appearAnim.running = true;
        }
    }

    PlasmaExtras.AppearAnimation {
        id: appearAnim
        targetItem: busyIndicator
        running: false
    }

    SequentialAnimation {
        id: disappearAnim
        running: false
        PlasmaExtras.DisappearAnimation {
            targetItem: busyIndicator
        }
        ScriptAction {
            script: {
                // unload ourselves
                busyLoader.source = "";
            }
        }
    }
}
