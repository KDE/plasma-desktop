/*
    SPDX-FileCopyrightText: 2014 Aaron Seigo <aseigo@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick

import org.kde.plasma.private.shell

Item {
    id: main

    property string scriptPath
    property alias mode: interactiveConsole.mode
    signal visibleChanged(bool visible)

    onScriptPathChanged: {
        interactiveConsole.loadScript(scriptPath);
    }

    InteractiveConsoleWindow {
        id: interactiveConsole

        onVisibleChanged: {
            main.visibleChanged(visible);
        }
    }

    Component.onCompleted: {
        interactiveConsole.scriptEngine = scriptEngine;
        interactiveConsole.visible = true;
    }
}

