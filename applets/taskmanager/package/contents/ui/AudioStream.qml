/*
    SPDX-FileCopyrightText: 2017 Kai Uwe Broulik <kde@privat.broulik.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15

import org.kde.plasma.core 2.0 as PlasmaCore

MouseArea {
    id: audioStreamIconBox

    width: Math.min(Math.min(iconBox.width, iconBox.height) * 0.4, PlasmaCore.Units.iconSizes.smallMedium)
    height: width
    anchors {
        top: frame.top
        right: frame.right
        rightMargin: taskFrame.margins.right
        topMargin: Math.round(taskFrame.margins.top * indicatorScale)
    }

    readonly property real indicatorScale: 1.2

    activeFocusOnTab: true
    hoverEnabled: true
    onClicked: toggleMuted()

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
                elementId: "audio-volume-high"
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
                elementId: "audio-volume-muted"
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
                     duration: PlasmaCore.Units.longDuration
                 }
             }
        },
        Transition {
             from: ""
             to: "muted"
             SequentialAnimation {
                 NumberAnimation {
                     property: "opacity"
                     duration: PlasmaCore.Units.longDuration
                 }
             }
        },
        Transition {
             to: ""
             NumberAnimation {
                 property: "opacity"
                 duration: PlasmaCore.Units.longDuration
             }
        }
    ]

    opacity: 0
    visible: opacity > 0

    Keys.onReturnPressed: toggleMuted()
    Keys.onEnterPressed: Keys.onReturnPressed(event);
    Keys.onSpacePressed: Keys.onReturnPressed(event);

    Accessible.checkable: true
    Accessible.checked: task.muted
    Accessible.name: task.muted ? i18nc("@action:button", "Unmute") : i18nc("@action:button", "Mute")
    Accessible.description: task.muted ? i18nc("@info:tooltip %1 is the window title", "Unmute %1", model.display) : i18nc("@info:tooltip %1 is the window title", "Mute %1", model.display)
    Accessible.role: Accessible.Button

    PlasmaCore.FrameSvgItem {
        anchors.fill: audioStreamIcon
        visible: parent.containsMouse || parent.activeFocus
        imagePath: "widgets/viewitem"
        prefix: "hover"
    }

    PlasmaCore.Svg {
        id: audioSvg
        imagePath: "icons/audio"
    }

    PlasmaCore.SvgItem {
        id: audioStreamIcon

        // Need audio indicator twice, to keep iconBox in the center.
        readonly property var requiredSpace: Math.min(iconBox.width, iconBox.height)
                                             + Math.min(Math.min(iconBox.width, iconBox.height), PlasmaCore.Units.iconSizes.smallMedium) * 2
        svg: audioSvg
        smooth: false

        height: Math.round(Math.min(parent.height * indicatorScale, PlasmaCore.Units.iconSizes.smallMedium))
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
                    width: PlasmaCore.Units.roundToIconSize(Math.min(Math.min(iconBox.width, iconBox.height), PlasmaCore.Units.iconSizes.smallMedium))
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
                    width: PlasmaCore.Units.roundToIconSize(Math.min(Math.min(iconBox.width, iconBox.height), PlasmaCore.Units.iconSizes.smallMedium))
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
