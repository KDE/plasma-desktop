/*
    SPDX-FileCopyrightText: 2017 Kai Uwe Broulik <kde@privat.broulik.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick

import org.kde.plasma.extras as PlasmaExtras
import org.kde.kirigami as Kirigami
import org.kde.ksvg as KSvg

Item {
    id: audioStreamIconBox

    width: Math.min(Math.min(iconBox.width, iconBox.height) * 0.4, Kirigami.Units.iconSizes.smallMedium)
    height: width
    anchors {
        top: frame.top
        right: frame.right
        rightMargin: taskFrame.margins.right
        topMargin: Math.round(taskFrame.margins.top * indicatorScale)
    }

    readonly property real indicatorScale: 1.2

    activeFocusOnTab: true

    // Using States rather than a simple Behavior we can apply different transitions,
    // which allows us to delay showing the icon but hide it instantly still.
    states: [
        State {
            name: "playing"
            when: task.playingAudio && !task.muted
            PropertyChanges {
                target: audioStreamIconBox
                opacity: 1
            }
            PropertyChanges {
                target: audioStreamIcon
                source: "audio-volume-high-symbolic"
            }
        },
        State {
            name: "muted"
            when: task.muted
            PropertyChanges {
                target: audioStreamIconBox
                opacity: 1
            }
            PropertyChanges {
                target: audioStreamIcon
                source: "audio-volume-muted-symbolic"
            }
        }
    ]

    transitions: [
        Transition {
             from: ""
             to: "playing"
             SequentialAnimation {
                 // Delay showing the play indicator so we don't flash it for brief sounds.
                 PauseAnimation {
                     duration: !task.delayAudioStreamIndicator || inPopup ? 0 : 2000
                 }
                 NumberAnimation {
                     property: "opacity"
                     duration: Kirigami.Units.longDuration
                 }
             }
        },
        Transition {
             from: ""
             to: "muted"
             SequentialAnimation {
                 NumberAnimation {
                     property: "opacity"
                     duration: Kirigami.Units.longDuration
                 }
             }
        },
        Transition {
             to: ""
             NumberAnimation {
                 property: "opacity"
                 duration: Kirigami.Units.longDuration
             }
        }
    ]

    opacity: 0
    visible: opacity > 0

    Keys.onReturnPressed: event => toggleMuted()
    Keys.onEnterPressed: event => Keys.returnPressed(event);
    Keys.onSpacePressed: event => Keys.returnPressed(event);

    Accessible.checkable: true
    Accessible.checked: task.muted
    Accessible.name: task.muted ? i18nc("@action:button", "Unmute") : i18nc("@action:button", "Mute")
    Accessible.description: task.muted ? i18nc("@info:tooltip %1 is the window title", "Unmute %1", model.display) : i18nc("@info:tooltip %1 is the window title", "Mute %1", model.display)
    Accessible.role: Accessible.Button

    HoverHandler {
        id: hoverHandler
    }

    TapHandler {
        id: tapHandler
        gesturePolicy: TapHandler.ReleaseWithinBounds // Exclusive grab
        onTapped: (eventPoint, button) => toggleMuted()
    }

    PlasmaExtras.Highlight {
        anchors.fill: audioStreamIcon
        hovered: hoverHandler.hovered || parent.activeFocus
        pressed: tapHandler.pressed
    }

    Kirigami.Icon {
        id: audioStreamIcon

        // Need audio indicator twice, to keep iconBox in the center.
        readonly property real requiredSpace: Math.min(iconBox.width, iconBox.height)
            + Math.min(Math.min(iconBox.width, iconBox.height), Kirigami.Units.iconSizes.smallMedium) * 2

        source: "audio-volume-high-symbolic"
        selected: tapHandler.pressed

        height: Math.round(Math.min(parent.height * indicatorScale, Kirigami.Units.iconSizes.smallMedium))
        width: height

        anchors {
            verticalCenter: parent.verticalCenter
            horizontalCenter: parent.horizontalCenter
        }

        states: [
            State {
                name: "verticalIconsOnly"
                when: tasks.vertical && frame.width < audioStreamIcon.requiredSpace

                PropertyChanges {
                    target: audioStreamIconBox
                    anchors.rightMargin: Math.round(taskFrame.margins.right * indicatorScale)
                }
            },

            State {
                name: "horizontal"
                when: frame.width > audioStreamIcon.requiredSpace

                AnchorChanges {
                    target: audioStreamIconBox

                    anchors.top: undefined
                    anchors.verticalCenter: frame.verticalCenter
                }

                PropertyChanges {
                    target: audioStreamIconBox
                    width: Kirigami.Units.iconSizes.roundedIconSize(Math.min(Math.min(iconBox.width, iconBox.height), Kirigami.Units.iconSizes.smallMedium))
                }

                PropertyChanges {
                    target: audioStreamIcon

                    height: parent.height
                    width: parent.width
                }
            },

            State {
                name: "vertical"
                when: frame.height > audioStreamIcon.requiredSpace

                AnchorChanges {
                    target: audioStreamIconBox

                    anchors.right: undefined
                    anchors.horizontalCenter: frame.horizontalCenter
                }

                PropertyChanges {
                    target: audioStreamIconBox

                    anchors.topMargin: taskFrame.margins.top
                    width: Kirigami.Units.iconSizes.roundedIconSize(Math.min(Math.min(iconBox.width, iconBox.height), Kirigami.Units.iconSizes.smallMedium))
                }

                PropertyChanges {
                    target: audioStreamIcon

                    height: parent.height
                    width: parent.width
                }
            }
        ]
    }
}
