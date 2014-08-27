import QtQuick 2.0

import org.kde.plasma.private.shell 2.0

Item {
    id: main

    property string scriptPath
    property alias mode: interactiveConsole.mode

    onScriptPathChanged: {
        interactiveConsole.loadScript(scriptPath);
    }

    InteractiveConsoleWindow {
        id: interactiveConsole

        onVisibleChanged: {
            main.visible = visible;
        }
    }

    Component.onCompleted: {
        interactiveConsole.scriptEngine = scriptEngine;
        interactiveConsole.visible = true;
    }
}

