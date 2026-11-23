/*
    SPDX-FileCopyrightText: 2026 Matthias Berndt <matthias_berndt@gmx.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami

// Measures how much pointer movement is rotated: the user moves the mouse back and forth along
// what feels like a horizontal line to them, and the rotation that makes this movement horizontal
// on screen is suggested to them. Back and forth movement doesn't tell left from right, so only
// rotations within ±90° of the applied one are found.
QQC2.ApplicationWindow {
    id: root

    // The rotation in effect while measuring, which the measured movement is relative to
    required property int appliedRotation
    // Name of the device to measure with, or empty when there is only one. KWin merges all
    // pointer devices into one pointer, so the movement of a particular one can't be told apart.
    required property string deviceName

    signal accepted()
    signal rejected()

    // How far the pointer has to move in total, in logical pixels. Short movements only have a few
    // possible directions due to rounding, so this is better than a number of movements.
    readonly property real requiredDistance: root.width * 3
    // Share of the most extreme directions on either side that is ignored
    readonly property real trimmedShare: 0.1

    property var movements: []
    property real movedDistance: 0
    property bool measurementFinished: false
    property int measuredRotation: 0

    title: i18ndc("kcmmouse", "@title:window", "Measure Angle Adjustment")

    // Finds the axis along which the pointer was moved back and forth.
    // movements is an array of {dx, dy}, the distances between consecutive pointer positions
    // in logical pixels, with y pointing down. The result is the angle of the axis in degrees
    // clockwise from horizontal, in (-90, 90]: 10 means right and slightly down, or left and
    // slightly up.
    //
    // A movement and its reverse lie on the same axis, but their directions, e.g. 10° and -170°,
    // can't be averaged. So the movements going against a rough guess of the axis are turned
    // around first: horizontal or vertical, whichever the movements mostly are. Then all of
    // them point roughly the same way, and their directions can be averaged, without the most
    // extreme ones on either side, which are outliers. Movement across the axis, e.g. from a
    // shaky hand, only ends up on both sides evenly when the guess is close, though, so this
    // is repeated with the result as the guess.
    function movementAxis(movements: var): int {
        let horizontal = 0;
        let vertical = 0;
        for (const movement of movements) {
            horizontal += Math.abs(movement.dx);
            vertical += Math.abs(movement.dy);
        }
        const guess = horizontal >= vertical ? 0 : Math.PI / 2;
        const axis = root.meanDirection(movements, root.meanDirection(movements, guess));

        const degrees = Math.round(axis * 180 / Math.PI);
        return degrees > 90 ? degrees - 180 : degrees <= -90 ? degrees + 180 : degrees;
    }

    // Mean direction of the movements in radians, after turning the ones around that go
    // against the given direction and leaving out the most extreme ones on either side
    function meanDirection(movements: var, direction: real): real {
        const cos = Math.cos(direction);
        const sin = Math.sin(direction);
        const angles = movements.map(movement => {
            // The movement in coordinates rotated by direction, turned around if needed
            const along = movement.dx * cos + movement.dy * sin;
            const across = movement.dy * cos - movement.dx * sin;
            return along < 0 ? Math.atan2(-across, -along) : Math.atan2(across, along);
        }).sort((a, b) => a - b);

        const trimmed = Math.floor(angles.length * root.trimmedShare);
        const kept = angles.slice(trimmed, angles.length - trimmed);
        return direction + kept.reduce((sum, angle) => sum + angle, 0) / kept.length;
    }

    function reject(): void {
        root.rejected();
        root.close();
    }

    function restart(): void {
        root.movements = [];
        root.movedDistance = 0;
        root.measurementFinished = false;
    }

    function addMovement(dx: real, dy: real): void {
        root.movements.push({ dx, dy });
        root.movedDistance += Math.hypot(dx, dy);
        if (root.movedDistance >= root.requiredDistance) {
            root.measuredRotation = ((root.appliedRotation - root.movementAxis(root.movements)) % 360 + 360) % 360;
            root.measurementFinished = true;
        }
    }

    // Unlike a MouseArea below the content, this also gets the movement over controls that
    // handle hovering themselves, like the warning and the buttons
    HoverHandler {
        id: movementHandler

        property point last: Qt.point(-1, -1)

        enabled: !root.measurementFinished

        function isAtEdge(position: point): bool {
            return position.x <= 0 || position.y <= 0 || position.x >= root.width - 1 || position.y >= root.height - 1;
        }

        // The pointer may have moved while disabled, so don't measure from where it was before
        onEnabledChanged: {
            movementHandler.last = Qt.point(-1, -1);
        }

        onPointChanged: {
            const last = movementHandler.last;
            const position = movementHandler.point.position;
            // At the edge of the screen the pointer stops, which distorts the movement
            if (last.x >= 0 && (position.x !== last.x || position.y !== last.y)
                    && !movementHandler.isAtEdge(last) && !movementHandler.isAtEdge(position)) {
                root.addMovement(position.x - last.x, position.y - last.y);
            }
            movementHandler.last = position;
        }
    }

    ColumnLayout {
        anchors.centerIn: parent
        width: Math.min(parent.width - Kirigami.Units.gridUnit * 4, Kirigami.Units.gridUnit * 30)
        spacing: Kirigami.Units.largeSpacing

        Kirigami.Heading {
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.Wrap
            text: root.title
        }

        Kirigami.InlineMessage {
            Layout.fillWidth: true
            visible: root.deviceName.length > 0
            type: Kirigami.MessageType.Warning
            text: i18ndc("kcmmouse", "@info %1 is the name of a pointing device", "Please make sure you are using the device “%1”.", root.deviceName)
        }

        QQC2.Label {
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.Wrap
            text: root.measurementFinished
                // Shown the way the spin box in the settings shows it
                ? i18ndc("kcmmouse", "@info %1 is an angle in degrees", "The measured angle is %1°.", root.measuredRotation > 180 ? root.measuredRotation - 360 : root.measuredRotation)
                : i18ndc("kcmmouse", "@info", "Move the mouse back and forth a few times, in the direction that feels like left and right to you. Don't watch the pointer: it doesn't matter which way it moves on the screen.")
        }

        QQC2.ProgressBar {
            Layout.fillWidth: true
            visible: !root.measurementFinished
            from: 0
            to: root.requiredDistance
            value: root.movedDistance
        }

        RowLayout {
            Layout.alignment: Qt.AlignHCenter
            spacing: Kirigami.Units.largeSpacing

            QQC2.Button {
                visible: root.measurementFinished
                text: i18ndc("kcmmouse", "@action:button Use the measured angle adjustment", "&OK")
                icon.name: "dialog-ok"
                onClicked: {
                    root.accepted();
                    root.close();
                }
            }

            QQC2.Button {
                visible: root.measurementFinished
                text: i18ndc("kcmmouse", "@action:button Angle adjustment calibration", "&Measure Again")
                icon.name: "view-refresh"
                onClicked: root.restart();
            }

            QQC2.Button {
                text: root.measurementFinished
                    ? i18ndc("kcmmouse", "@action:button Discard the measured angle adjustment", "&Discard")
                    : i18ndc("kcmmouse", "@action:button Angle adjustment calibration", "&Cancel")
                icon.name: root.measurementFinished ? "edit-delete" : "dialog-cancel"
                onClicked: root.reject();
            }
        }
    }

    Shortcut {
        sequences: [StandardKey.Cancel]
        onActivated: root.reject();
    }
}
