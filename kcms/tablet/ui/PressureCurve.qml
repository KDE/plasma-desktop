/*
    SPDX-FileCopyrightText: Joshua Goins <joshua.goins@kdab.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Window
import QtQuick.Layouts
import QtQuick.Shapes
import QtQuick.Controls as QQC2

import org.kde.kirigami as Kirigami
import org.kde.plasma.tablet.kcm as KCM

QQC2.Control {
    id: root

    property point controlPoint1
    property point controlPoint2
    property bool isDefault: true

    implicitWidth: 220
    implicitHeight: 220

    leftPadding: 0
    rightPadding: 0
    topPadding: 0
    bottomPadding: 0

    clip: true

    function forceReloadControlPoints(): void {
        firstControlCircle.controlPoint = controlPoint1;
        secondControlCircle.controlPoint = controlPoint2;
    }

    background: Rectangle {
        color: Kirigami.Theme.backgroundColor

        Kirigami.Theme.inherit: false
        Kirigami.Theme.colorSet: Kirigami.Theme.View
    }

    contentItem: Item {
        // Vertical grid lines
        Repeater {
            id: verticalRepeater

            model: 4
            delegate: Shape {
                anchors.fill: parent
                preferredRendererType: Shape.CurveRenderer

                Kirigami.Theme.inherit: false
                Kirigami.Theme.colorSet: Kirigami.Theme.Window

                ShapePath {
                    strokeWidth: 1
                    strokeColor: Kirigami.Theme.backgroundColor
                    fillColor: "transparent"

                    startX: line.x
                    startY: 0

                    PathLine {
                        id: line

                        x: index * (root.width / verticalRepeater.count)
                        y: root.height
                    }
                }
            }
        }

        // Horizontal grid lines
        Repeater {
            id: horizontalRepeater

            model: 4

            delegate: Shape {
                anchors.fill: parent
                preferredRendererType: Shape.CurveRenderer

                Kirigami.Theme.inherit: false
                Kirigami.Theme.colorSet: Kirigami.Theme.Window

                ShapePath {
                    strokeWidth: 1
                    strokeColor: Kirigami.Theme.backgroundColor
                    fillColor: "transparent"

                    startX: 0
                    startY: line.y

                    PathLine {
                        id: line

                        x: root.width
                        y: index * (root.height / horizontalRepeater.count)
                    }
                }
            }
        }

        Shape {
            anchors.fill: parent
            preferredRendererType: Shape.CurveRenderer

            ShapePath {
                id: path

                strokeWidth: 2
                strokeColor: kcm.defaultsIndicatorsVisible && !root.isDefault ? Kirigami.Theme.neutralTextColor : (root.activeFocus ? Kirigami.Theme.highlightColor : Kirigami.Theme.disabledTextColor)
                fillColor: Qt.alpha(strokeColor, 0.2)
                simplify: true

                startX: -2
                startY: root.height + 2

                PathCubic {
                    x: root.width + 2
                    y: 0
                    control1X: root.width * controlPoint1.x
                    control1Y: root.height * (1.0 - controlPoint1.y)
                    control2X: root.width * controlPoint2.x
                    control2Y: root.height * (1.0 - controlPoint2.y)
                }

                PathLine {
                    x: root.width + 2
                    y: root.height + 2
                }
            }

            // line to first control point
            ShapePath {
                strokeWidth: 1
                strokeColor: Kirigami.Theme.disabledTextColor

                startX: 0.0
                startY: root.height

                PathLine {
                    x: root.width * controlPoint1.x
                    y: root.height * (1.0 - controlPoint1.y)
                }
            }

            // line to second control point
            ShapePath {
                strokeWidth: 1
                strokeColor: Kirigami.Theme.disabledTextColor

                startX: root.width
                startY: 0.0

                PathLine {
                    x: root.width * controlPoint2.x
                    y: root.height * (1.0 - controlPoint2.y)
                }
            }
        }

        component ControlCircle: Rectangle {
            id: circle

            required property point controlPoint

            width: 15
            height: width

            radius: width
            color: Kirigami.Theme.disabledTextColor

            x: (root.width * controlPoint.x) - (width / 2)
            y: (root.height * (1.0 - controlPoint.y)) - (height / 2)

            DragHandler {
                target: null // Do not update the item position, we want to control that ourselves
                cursorShape: Qt.DragMoveCursor

                onActiveChanged: {
                    if (active) {
                        root.forceActiveFocus(Qt.MouseFocusReason);
                    }
                }

                xAxis {
                    minimum: 0.0
                    maximum: root.width

                    onActiveValueChanged: delta => {
                        let point = controlPoint;
                        const newDelta = delta / root.width;
                        const newPosX = point.x * root.width + delta;

                        if (newPosX > xAxis.minimum && newPosX < xAxis.maximum) {
                            point.x += newDelta;
                            circle.setPoint(point);
                        }
                    }
                }

                // QtQuick's origin is at the top-left, while the curve's coordinate space is in the bottom-left hence all the flips.
                yAxis {
                    minimum: 0.0
                    maximum: root.height

                    onActiveValueChanged: delta => {
                        let point = controlPoint;
                        const newDelta = delta / root.height;
                        const newPosY = (1.0 - point.y) * root.height + delta;

                        if (newPosY > yAxis.minimum && newPosY < yAxis.maximum) {
                            point.y += -newDelta; // we're flipping here because of the aforementioned coordinate space difference
                            circle.setPoint(point);
                        }
                    }
                }
            }
        }

        ControlCircle {
            id: firstControlCircle

            controlPoint: controlPoint1

            function setPoint(newPoint: point): void {
                controlPoint1 = newPoint;
            }
        }

        ControlCircle {
            id: secondControlCircle

            controlPoint: controlPoint2

            function setPoint(newPoint: point): void {
                controlPoint2 = newPoint;
            }
        }

        TapHandler {
            onTapped: root.forceActiveFocus(Qt.MouseFocusReason)
        }
    }
}
