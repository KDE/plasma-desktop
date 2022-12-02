/*
    SPDX-FileCopyrightText: 2021 Cyril Rossi <cyril.rossi@enioka.com>
    SPDX-FileCopyrightText: 2022 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Controls 2.4 as QQC2
import QtQuick.Layouts 1.3
import QtQuick.Window 2.15

import org.kde.kirigami 2.13 as Kirigami

QQC2.Control {
    id: delegate
    property Item viewPort
    readonly property string screenName: model.screenName
    readonly property int screenId: model.screenId
    property bool containsDrag
    property alias contentsLayout: contentsLayout

    width: Math.min(Kirigami.Units.gridUnit * 25, Math.floor(viewPort.width / Math.min(repeater.count, Math.floor(viewPort.width / (Kirigami.Units.gridUnit * 12)))))

    contentItem: ColumnLayout {
        id: contentsLayout
        width: Math.min(parent.width, Kirigami.Units.gridUnit * 15)

        Rectangle {
            id: screenRect
            Layout.fillWidth: true
            Layout.preferredHeight: width / 1.6
            color: Kirigami.Theme.backgroundColor
            border.color: Kirigami.Theme.textColor
            Rectangle {
                anchors.fill: parent
                z: 9
                color: "black"
                opacity: delegate.containsDrag ? 0.3 : 0
                Behavior on opacity {
                    OpacityAnimator {
                        duration: Kirigami.Units.longDuration
                        easing.type: Easing.InOutQuad
                    }
                }
            }

            Repeater {
                id: containmentRepeater
                model: containments

                Rectangle {
                    id: contRect
                    property real homeX
                    property real homeY
                    property string oldState
                    readonly property int edgeDistance: {
                        return (state === "left" || state === "right" ? width : height) * model.edgePosition;
                    }

                    width: moveButton.width
                    height: moveButton.height
                    border.color: Kirigami.Theme.textColor
                    color: Kirigami.Theme.backgroundColor
                    state: model.edge
                    visible: !model.isDestroyed
                    z: state === "floating" ? 0 : 1

                    HoverHandler {
                        cursorShape: Qt.OpenHandCursor
                    }
                    DragHandler {
                        id: dragHandler
                        property QQC2.Control targetDelegate

                        cursorShape: Qt.ClosedHandCursor
                        onActiveChanged: {
                            if (active) {
                                delegate.z = 1;
                            } else {
                                if (targetDelegate) {
                                    resetAnim.restart();
                                    containmentRepeater.model.moveContainementToScreen(model.containmentId, targetDelegate.screenId)
                                    targetDelegate.containsDrag = false;
                                    targetDelegate = null;
                                } else {
                                    resetAnim.restart();
                                }
                            }
                        }
                        onTranslationChanged: {
                            if (!active) {
                                if (targetDelegate) {
                                    targetDelegate.containsDrag = false;
                                    targetDelegate = null;
                                }
                                return;
                            }
                            let pos = contRect.mapToItem(delegate.parent, dragHandler.centroid.position.x, dragHandler.centroid.position.y);
                            let otherDelegate = delegate.parent.childAt(pos.x, pos.y);
                            if (targetDelegate && targetDelegate !== otherDelegate) {
                                targetDelegate.containsDrag = false;
                            }
                            if (!otherDelegate || otherDelegate === delegate) {
                                targetDelegate = null;
                            } else if (otherDelegate && otherDelegate !== delegate
                                && otherDelegate.hasOwnProperty("screenId")
                                && otherDelegate.hasOwnProperty("containsDrag")) {
                                targetDelegate = otherDelegate;
                                targetDelegate.containsDrag = true;
                            }
                        }
                    }
                    SequentialAnimation {
                        id: resetAnim
                        property var targetDelegatePos: dragHandler.targetDelegate
                                ? dragHandler.targetDelegate.contentsLayout.mapToItem(delegate.contentsLayout, 0, 0)
                                : Qt.point(0, 0)
                        ParallelAnimation {
                            XAnimator {
                                target: contRect
                                from: contRect.x
                                to: contRect.homeX + resetAnim.targetDelegatePos.x
                                duration: Kirigami.Units.longDuration
                                easing.type: Easing.InOutQuad
                            }
                            YAnimator {
                                target: contRect
                                from: contRect.y
                                to: contRect.homeY + resetAnim.targetDelegatePos.y
                                duration: Kirigami.Units.longDuration
                                easing.type: Easing.InOutQuad
                            }
                        }
                        PropertyAction {
                            target: delegate
                            property: "z"
                            value: 0
                        }
                    }

                    Image {
                        id: containmentImage
                        anchors {
                            fill: parent
                            margins: 1
                        }
                        // It needs to reload the image from disk when the file changes
                        cache: false
                        source: model.imageSource
                        fillMode: model.edge == "floating" ? Image.PreserveAspectCrop : Image.PreserveAspectFit
                    }

                    QQC2.Button {
                        id: moveButton
                        icon.name: "open-menu-symbolic"
                        checked: contextMenu.visible
                        anchors {
                            right: parent.right
                            top: parent.top
                            topMargin: model.edge == "floating"
                                ? model.panelCountAtTop * moveButton.height + Kirigami.Units.largeSpacing
                                : 0
                            rightMargin: model.edge == "floating"
                                ? (moveButton.LayoutMirroring.enabled ? model.panelCountAtLeft : model.panelCountAtRight) * moveButton.height + Kirigami.Units.largeSpacing
                                : 0
                        }
                        onClicked: {
                            contextMenu.open()
                        }

                        QQC2.Menu {
                            id: contextMenu
                            y: moveButton.height
                            Repeater {
                                model: ShellContainmentModel
                                QQC2.MenuItem {
                                    text: edge == "floating"
                                        ? i18nd("plasma_shell_org.kde.plasma.desktop", "Swap with Desktop on Screen %1", model.screenName)
                                        : i18nd("plasma_shell_org.kde.plasma.desktop", "Move to Screen %1", model.screenName)
                                    visible: model.screenName !== delegate.screenName
                                    height: visible ? implicitHeight : 0
                                    onTriggered: {
                                        containmentRepeater.model.moveContainementToScreen(containmentId, screenId)
                                    }
                                }
                            }
                            QQC2.MenuSeparator {
                                visible: contRect.state !== "floating"
                            }
                            QQC2.MenuItem {
                                text: i18nd("plasma_shell_org.kde.plasma.desktop", "Remove Panel")
                                icon.name: "edit-delete"
                                onTriggered: containments.remove(containmentId)
                                visible: contRect.state !== "floating"
                            }
                        }
                    }

                    states: [
                        State {
                            name: "floating"
                            PropertyChanges {
                                target: contRect;
                                width: screenRect.width
                                height: screenRect.height
                                color: "transparent"
                            }
                        },
                        State {
                            name: "top"
                            PropertyChanges {
                                target: contRect;
                                width: screenRect.width
                                y: homeY
                                homeX: 0
                                homeY: contRect.edgeDistance
                            }
                        },
                        State {
                            name: "right"
                            PropertyChanges {
                                target: contRect;
                                x: homeX
                                homeX: screenRect.width - contRect.width - contRect.edgeDistance;
                                height: screenRect.height
                                homeY: 0
                            }
                        },
                        State {
                            name: "bottom"
                            PropertyChanges {
                                target: contRect;
                                y: homeY
                                homeX: 0
                                homeY: screenRect.height - contRect.height - contRect.edgeDistance;
                                width: screenRect.width
                            }
                        },
                        State {
                            name: "left"
                            PropertyChanges {
                                target: contRect;
                                height: screenRect.height
                                x: homeX
                                homeX: contRect.edgeDistance
                                homeY: 0
                            }
                        }
                    ]
                }
            }
        }
        QQC2.Label {
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WordWrap
            text: model.isPrimary
                ? i18nd("plasma_shell_org.kde.plasma.desktop", "%1 (primary)", model.screenName)
                : model.screenName
        }
    }
}
