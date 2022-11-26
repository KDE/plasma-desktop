/*
    SPDX-FileCopyrightText: 2013 Sebastian Kügler <sebas@kde.org>
    SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>
    SPDX-FileCopyrightText: 2016 Kai Uwe Broulik <kde@privat.broulik.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.15

MouseArea {
    property var modelIndex
    // winId won't be an int wayland
    property var winId // FIXME Legacy
    property Item rootTask

    acceptedButtons: Qt.LeftButton | Qt.MiddleButton | Qt.RightButton
    hoverEnabled: true
    enabled: winId !== 0

    onClicked: {
        switch (mouse.button) {
        case Qt.LeftButton:
            tasksModel.requestActivate(modelIndex);
            rootTask.hideImmediately();
            backend.cancelHighlightWindows();
            break;
        case Qt.MiddleButton:
            backend.cancelHighlightWindows();
            tasksModel.requestClose(modelIndex);
            break;
        case Qt.RightButton:
            tasks.createContextMenu(rootTask, modelIndex).show();
            break;
        }
    }

    onContainsMouseChanged: {
        tasks.windowsHovered([winId], containsMouse);
    }
}
