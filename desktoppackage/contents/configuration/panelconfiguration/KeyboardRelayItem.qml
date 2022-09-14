/*
    SPDX-FileCopyrightText: 2022 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15

Item {
    activeFocusOnTab: true

    Keys.onUpPressed: Keys.onLeftPressed(event);
    Keys.onDownPressed: Keys.onRightPressed(event);
    Keys.onLeftPressed: if (panel.configOverlay.currentAppletIndex() > 0) {
        if ((event.modifiers & Qt.ControlModifier) && (event.modifiers & Qt.ShiftModifier)) {
            panel.configOverlay.moveTo(panel.configOverlay.currentAppletIndex() - 1);
        } else {
            panel.configOverlay.setCurrentApplet(panel.configOverlay.currentAppletIndex() - 1);
        }
        // Always accept the event
    }
    Keys.onRightPressed: {
        if (panel.configOverlay.currentAppletIndex() + 1 < panel.configOverlay.count) {
            if ((event.modifiers & Qt.ControlModifier) && (event.modifiers & Qt.ShiftModifier)) {
                panel.configOverlay.moveTo(panel.configOverlay.currentAppletIndex() + 1);
            } else {
                panel.configOverlay.setCurrentApplet(panel.configOverlay.currentAppletIndex() + 1);
            }
        }
    }

    Keys.onPressed: {
        if (event.modifiers !== Qt.NoModifier) {
            return;
        }

        switch (event.key) {
        case Qt.Key_C:
        case Qt.Key_D:
        case Qt.Key_S:
            panel.configOverlay.configureCurrentApplet();
            break;
        case Qt.Key_A:
            panel.configOverlay.showAlternativesMenu();
            break;
        case Qt.Key_Backspace:
        case Qt.Key_Delete: {
            panel.configOverlay.removeCurrentApplet();
            break;
        }
        default:
            return;
        }

        event.accepted = true;
    }

    onActiveFocusChanged: if (activeFocus) {
        panel.configOverlay.setCurrentApplet(0);
    } else {
        panel.configOverlay.setCurrentApplet(-1);
    }
}
