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

    signal gotInputSequence(sequence: inputSequence)

    icon.name: {
        switch (inputSequence.type) {
            case InputSequence.Disabled:
                return "action-unavailable-symbolic";
            case InputSequence.Keyboard:
                return "input-keyboard-symbolic";
            case InputSequence.Mouse:
                return "input-mouse-symbolic";
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
        actionDialog.closed.connect(clearSignals);
        actionDialog.gotInputSequence.connect(moveUpSequence);
        actionDialog.open();
        actionDialog.refreshDialogData();
    }
}