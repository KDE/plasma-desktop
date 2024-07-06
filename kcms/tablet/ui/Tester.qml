/*
    SPDX-FileCopyrightText: Joshua Goins <josh@redstrate.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Window
import QtQuick.Layouts
import QtQuick.Shapes
import QtQuick.Controls as QQC2

import org.kde.kirigami as Kirigami
import org.kde.plasma.tablet.kcm

QQC2.ApplicationWindow {
    id: root

    property bool toolDown: false

    minimumWidth: 400
    minimumHeight: 200

    title: i18ndc("kcm_tablet", "@title", "Tablet Tester")

    function insideDrawingSquare(local_x: real, local_y: real): bool {
        return local_x >= 0 &&
                local_y >= 0 &&
                local_x <= drawingSquare.width &&
                local_y <= drawingSquare.height;
    }

    function scrollLogToBottom(): void {
        logScrollView.QQC2.ScrollBar.vertical.position = 1.0 - logScrollView.QQC2.ScrollBar.vertical.size;
    }

    Connections {
        target: tabletEvents

        function onToolDown(hardware_serial_hi: int, hardware_serial_lo: int, window_x: real, window_y: real): void {
            const local = drawingSquare.mapFromItem(container, window_x, window_y);

            if (insideDrawingSquare(local.x, local.y)) {
                root.toolDown = true;
                penPath.path = [];

                penLogText.append(i18nd("kcm_tablet", "Stylus press X=%1 Y=%2", local.x, local.y));
                scrollLogToBottom();
            }
        }

        function onToolUp(hardware_serial_hi: int, hardware_serial_lo: int, window_x: real, window_y: real): void {
            const local = drawingSquare.mapFromItem(container, window_x, window_y);

            root.toolDown = false;

            penLogText.append(i18nd("kcm_tablet", "Stylus release X=%1 Y=%2", local.x, local.y));
            scrollLogToBottom();
        }

        function onToolMotion(hardware_serial_hi: int, hardware_serial_lo: int, window_x: real, window_y: real): void {
            const local = drawingSquare.mapFromItem(container, window_x, window_y);

            if (insideDrawingSquare(local.x, local.y) && root.toolDown) {
                penPath.path.push(local);

                penLogText.append(i18nd("kcm_tablet", "Stylus move X=%1 Y=%2", local.x, local.y));
                scrollLogToBottom();
            }
        }
    }

    TabletEvents {
        id: tabletEvents

        anchors.fill: parent

        onPadButtonsChanged: {
            if (!path.endsWith(form.padDevice.sysName)) {
                return;
            }
            buttonsRepeater.model = buttonCount
        }
    }

    Item {
        id: container
        anchors.fill: parent

        RowLayout {
            id: layout

            spacing: Kirigami.Units.largeSpacing

            anchors {
                fill: parent
                margins: Kirigami.Units.largeSpacing
            }

            Rectangle {
                id: drawingSquare

                implicitWidth: 220
                implicitHeight: 220

                color: Kirigami.Theme.backgroundColor
                clip: true

                Layout.fillHeight: true
                Layout.preferredWidth: Math.round(root.width / 3)

                Kirigami.Theme.inherit: false
                Kirigami.Theme.colorSet: Kirigami.Theme.View

                // Vertical lines
                Repeater {
                    id: verticalRepeater

                    model: 8
                    delegate: Shape {
                        id: verticalShape

                        anchors.fill: parent

                        Kirigami.Theme.inherit: false
                        Kirigami.Theme.colorSet: Kirigami.Theme.Window

                        ShapePath {
                            strokeWidth: 1
                            strokeColor: Kirigami.Theme.backgroundColor
                            fillColor: "transparent"

                            startX: line.x

                            PathLine {
                                id: line

                                x: index * (verticalShape.width / verticalRepeater.count)
                                y: verticalShape.height
                            }
                        }
                    }
                }

                // Horizontal lines
                Repeater {
                    id: horizontalRepeater

                    model: 8
                    delegate: Shape {
                        id: horizontalShape

                        anchors.fill: parent

                        Kirigami.Theme.inherit: false
                        Kirigami.Theme.colorSet: Kirigami.Theme.Window

                        ShapePath {
                            strokeWidth: 1
                            strokeColor: Kirigami.Theme.backgroundColor
                            fillColor: "transparent"

                            startY: line.y

                            PathLine {
                                id: line

                                x: horizontalShape.width
                                y: index * (horizontalShape.height / horizontalRepeater.count) - 1
                            }
                        }
                    }
                }

                // Pen Path
                Shape {
                    anchors.fill: parent

                    ShapePath {
                        strokeWidth: 2
                        strokeColor: Kirigami.Theme.highlightColor
                        fillColor: "transparent"
                        PathPolyline {
                            id: penPath
                        }
                    }
                }

                // This is here just to catch events from going to the window, nothing more
                MouseArea {
                    anchors.fill: parent
                }
            }

            ColumnLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.preferredHeight: drawingSquare.implicitHeight

                spacing: Kirigami.Units.largeSpacing

                QQC2.ScrollView {
                    id: logScrollView

                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    QQC2.TextArea {
                        id: penLogText

                        text: i18nd("kcm_tablet", "## Legend:\n# X, Y - event coordinate\n")
                        readOnly: true
                        wrapMode: TextEdit.Wrap
                    }
                }

                QQC2.Button {
                    text: i18ndc("kcm_tablet", "Clear the event log", "Clear")
                    icon.name: "edit-clear-symbolic"

                    onClicked: {
                        penLogText.clear();
                        penPath.path = [];
                    }

                    Layout.fillWidth: true
                }
            }
        }
    }

    // FIXME: For some reason, setting this declaratively ends up with a broken window until it's resized.
    Component.onCompleted: width = 730
}
