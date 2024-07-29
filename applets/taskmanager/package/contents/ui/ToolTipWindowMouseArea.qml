/*
    SPDX-FileCopyrightText: 2013 Sebastian Kügler <sebas@kde.org>
    SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>
    SPDX-FileCopyrightText: 2016 Kai Uwe Broulik <kde@privat.broulik.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

pragma ComponentBehavior: Bound

import QtQuick

MouseArea {
    required property /*QModelIndex*/var modelIndex
    required property /*undefined|WId where WId = int|string*/ var winId
    required property Task rootTask

    acceptedButtons: Qt.LeftButton | Qt.MiddleButton | Qt.RightButton
    hoverEnabled: true
    enabled: winId !== undefined

    onClicked: (mouse) => {
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
