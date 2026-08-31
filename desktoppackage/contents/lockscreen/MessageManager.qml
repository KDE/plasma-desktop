// SPDX-License-Identifier: LGPL-2.0-or-later
// SPDX-FileCopyrightText: 2015 Marco Martin <mart@kde.org>
// SPDX-FileCopyrightText: 2026 Harald Sitter <sitter@kde.org>

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Templates as T
import QtQuick.Layouts
import org.kde.kirigami.platform as Platform

import org.kde.plasma.core as PlasmaCore
import org.kde.kirigami as Kirigami

Item {
    id: root

    readonly property int maximumNotificationWidth: Math.min(width * 0.25, Platform.Units.gridUnit * 20)
    readonly property int maximumNotificationCount: 8

    function showMessage(type, message) {
        console.warn("showMessage", type, message)

        if (message === undefined || message === null || message === "") {
            console.warn("showMessage: message is empty, ignoring")
            return;
        }

        notificationsModel.append({
            text: message,
            closeInterval: Kirigami.Units.veryLongDuration,
            type: type
        })

        // remove the oldest notification if new notification count would exceed limit
        if (notificationsModel.count === maximumNotificationCount) {
            if ((listView.itemAtIndex(0) as T.Control).hovered === true) {
                hideMessage(1)
            } else {
                hideMessage()
            }
        }
    }

    /*!
     * \brief Remove a notification at specific index. By default, index is set to 0.
     */
    function hideMessage(index = 0) {
        if (index >= 0 && notificationsModel.count > index) {
            notificationsModel.remove(index)
        }
    }

    function clearMessages() {
        notificationsModel.clear()
    }

    ListModel {
        id: notificationsModel
    }

    ListView {
        id: listView

        anchors.fill: parent
        anchors.bottomMargin: Platform.Units.largeSpacing

        leftMargin: SafeArea.margins.left
        rightMargin: SafeArea.margins.right
        topMargin: SafeArea.margins.top
        bottomMargin: SafeArea.margins.bottom

        spacing: Platform.Units.largeSpacing
        model: notificationsModel
        verticalLayoutDirection: ListView.BottomToTop
        keyNavigationEnabled: false
        reuseItems: false  // do not reuse items, otherwise delegates do not hide themselves properly
        focus: false
        interactive: false

        add: Transition {
            id: addAnimation
            ParallelAnimation {
                alwaysRunToEnd: true
                NumberAnimation {
                    property: "opacity"
                    from: 0
                    to: 1
                    duration: Platform.Units.longDuration
                    easing.type: Easing.OutCubic
                }
                NumberAnimation {
                    property: "y"
                    from: addAnimation.ViewTransition.destination.y - Platform.Units.gridUnit * 3
                    duration: Platform.Units.longDuration
                    easing.type: Easing.OutCubic
                }
            }
        }
        displaced: Transition {
            ParallelAnimation {
                alwaysRunToEnd: true
                NumberAnimation {
                    property: "y"
                    duration: Platform.Units.longDuration
                    easing.type: Easing.InOutCubic
                }
                NumberAnimation {
                    property: "opacity"
                    duration: Platform.Units.longDuration
                    to: 1
                }
            }
        }
        remove: Transition {
            ParallelAnimation {
                alwaysRunToEnd: true
                NumberAnimation {
                    property: "opacity"
                    from: 1
                    to: 0
                    duration: Platform.Units.longDuration
                    easing.type: Easing.InCubic
                }
                NumberAnimation {
                    property: "y"
                    to: Platform.Units.gridUnit * 3
                    duration: Platform.Units.longDuration
                    easing.type: Easing.InCubic
                }
                PropertyAction {
                    property: "transformOrigin"
                    value: Item.Bottom
                }
                PropertyAnimation {
                    property: "scale"
                    from: 1
                    to: 0
                    duration: Platform.Units.longDuration
                    easing.type: Easing.InCubic
                }
            }
        }

        delegate: QQC2.Control {
            id: delegate

            required property int index
            required property int closeInterval
            required property string text
            required property string type

            hoverEnabled: true

            anchors.right: parent ? parent.right : undefined // parent is transiently falsy
            width: root.maximumNotificationWidth
            z: {
                if (delegate.hovered) {
                    return 2;
                } else if (delegate.index === 0) {
                    return 1;
                } else {
                    return 0;
                }
            }

            leftPadding: Platform.Units.largeSpacing * 2
            rightPadding: Platform.Units.largeSpacing * 2
            topPadding: Platform.Units.largeSpacing * 2
            bottomPadding: Platform.Units.largeSpacing * 2

            contentItem: RowLayout {
                spacing: Platform.Units.mediumSpacing

                TapHandler {
                    acceptedButtons: Qt.LeftButton
                    onTapped: eventPoint => root.hideMessage(delegate.index)
                }

                Timer {
                    id: timer
                    interval: delegate.closeInterval
                    running: !delegate.hovered
                    onTriggered: root.hideMessage(delegate.index)
                }

                Kirigami.Icon {
                    implicitWidth: Platform.Units.iconSizes.sizeForLabels
                    implicitHeight: Platform.Units.iconSizes.sizeForLabels
                    source: switch(delegate.type) {
                        case "error":
                            return "emblem-important-symbolic"
                        case "prompt":
                            return "system-user-prompt-symbolic"
                        default:
                            return "emblem-information-symbolic"
                    }
                }

                QQC2.Label {
                    text: delegate.text
                    elide: Text.ElideRight
                    wrapMode: Text.Wrap
                    Layout.fillWidth: true
                }
            }

            background: PlasmaCore.DialogBackground {
            }
        }
    }
}

