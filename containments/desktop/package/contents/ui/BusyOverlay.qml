/*
 *   Copyright 2011-2013 Sebastian Kügler <sebas@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2 or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Library General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
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
