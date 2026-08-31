// SPDX-License-Identifier: LGPL-2.0-or-later
// SPDX-FileCopyrightText: 2015 Marco Martin <mart@kde.org>
// SPDX-FileCopyrightText: 2026 Harald Sitter <sitter@kde.org>

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.kirigami.platform as Platform

import org.kde.plasma.core as PlasmaCore
import org.kde.kirigami as Kirigami

Item {
    id: root

    readonly property int maximumNotificationWidth: Math.min(width * 0.25, Platform.Units.gridUnit * 20)
    readonly property int maximumNotificationCount: 8

    enum MessageType {
        NotificationType,
        ErrorType,
        PromptType
    }

    function showMessage(type: int, message: string) {
        if (!message) {
            console.warn("showMessage: message is empty, ignoring")
            return;
        }

        notificationsModel.append({
            text: message,
            type: type
        })

        // remove the oldest notification if new notification count would exceed limit
        if (notificationsModel.count === maximumNotificationCount) {
            if ((listView.itemAtIndex(0) as QQC2.Control).hovered === true) {
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
            required property string text
            required property int type

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
                    onTapped: root.hideMessage(delegate.index)
                }

                Timer {
                    id: timer
                    interval: 8000 // completely arbitrary, there is no semantic Unit that makes sense for us
                    running: !delegate.hovered
                    onTriggered: root.hideMessage(delegate.index)
                }

                Kirigami.Icon {
                    implicitWidth: Platform.Units.iconSizes.sizeForLabels
                    implicitHeight: Platform.Units.iconSizes.sizeForLabels
                    source: switch(delegate.type) {
                        case MessageManager.ErrorType:
                            return "emblem-important-symbolic"
                        case MessageManager.PromptType:
                            return "system-user-prompt-symbolic"
                        case MessageManager.NotificationType:
                            return "emblem-information-symbolic"
                        default:
                            return "emblem-information-symbolic"
                    }
                }

                QQC2.Label {
                    text: delegate.text
                    elide: Text.ElideRight
                    wrapMode: Text.Wrap
                    textFormat: Text.PlainText
                    Layout.fillWidth: true
                }
            }

            background: PlasmaCore.DialogBackground {
            }
        }
    }
}

