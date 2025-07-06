/*
    SPDX-FileCopyrightText: 2024 Joshua Goins <joshua.goins@kdab.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2

import org.kde.kirigami as Kirigami
import org.kde.plasma.tablet.kcm as KCM
import org.kde.kcmutils
import org.kde.kquickcontrols

QQC2.Button {
    id: root

    required property string name
    property KCM.inputSequence inputSequence
    property bool supportsPenButton: true
    property bool supportsRelativeEvents: false

    signal gotInputSequence(sequence: KCM.inputSequence)

    icon.name: {
        switch (inputSequence.type) {
            case KCM.InputSequence.Disabled:
                return "action-unavailable-symbolic";
            case KCM.InputSequence.Keyboard:
                return "input-keyboard-symbolic";
            case KCM.InputSequence.Mouse:
                return "input-mouse-symbolic";
            case KCM.InputSequence.Pen:
                return "tool_pen-symbolic";
            case KCM.InputSequence.Scroll:
                return "input-mouse-click-middle";
            case KCM.InputSequence.ApplicationDefined:
                return "applications-all-symbolic";
        }
    }
    text: inputSequence.toString()

    function moveUpSequence(sequence: KCM.inputSequence): void {
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
