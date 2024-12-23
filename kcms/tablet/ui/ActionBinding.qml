/*
    SPDX-FileCopyrightText: 2024 Joshua Goins <joshua.goins@kdab.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2

import org.kde.kirigami as Kirigami
import org.kde.plasma.tablet.kcm
import org.kde.kcmutils
import org.kde.kquickcontrols

QQC2.Button {
    id: root

    property inputSequence inputSequence
    property string name
    property bool supportsPenButton: true
    property bool supportsRelativeEvents: false

    signal gotInputSequence(sequence: inputSequence)

    icon.name: {
        switch (inputSequence.type) {
            case InputSequence.Disabled:
                return "action-unavailable-symbolic";
            case InputSequence.Keyboard:
                return "input-keyboard-symbolic";
            case InputSequence.Mouse:
                return "input-mouse-symbolic";
            case InputSequence.Pen:
                return "tool_pen-symbolic";
            case InputSequence.Scroll:
                return "input-mouse-click-middle";
            case InputSequence.ApplicationDefined:
                return "applications-all-symbolic";
        }
    }
    text: inputSequence.toString()

    function moveUpSequence(sequence: inputSequence): void {
        root.gotInputSequence(sequence);
        actionDialog.gotInputSequence.disconnect(moveUpSequence);
    }

    function clearSignals(): void {
        actionDialog.closed.disconnect(clearSignals);
        actionDialog.gotInputSequence.disconnect(moveUpSequence);
    }

    onClicked: {
        actionDialog.name = name;
        actionDialog.inputSequence = inputSequence;
        actionDialog.supportsPenButton = supportsPenButton;
        actionDialog.supportsRelativeEvents = supportsRelativeEvents;
        actionDialog.closed.connect(clearSignals);
        actionDialog.gotInputSequence.connect(moveUpSequence);
        actionDialog.open();
        actionDialog.refreshDialogData();
    }
}
