/***************************************************************************
 *   Copyright (C) 2017 Kai Uwe Broulik <kde@privat.broulik.de>            *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

import QtQuick 2.0

import org.kde.plasma.core 2.0 as PlasmaCore

MouseArea {
    id: audioStreamIconBox
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

    PlasmaCore.FrameSvgItem {
        anchors.fill: audioStreamIcon
        visible: parent.containsMouse
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
                    target: audioStreamIconLoader
                    anchors.rightMargin: Math.round(taskFrame.margins.right * indicatorScale)
                }
            },

            State {
                name: "horizontal"
                when: frame.width > audioStreamIcon.requiredSpace

                AnchorChanges {
                    target: audioStreamIconLoader

                    anchors.top: undefined
                    anchors.verticalCenter: frame.verticalCenter
                }

                PropertyChanges {
                    target: audioStreamIconLoader
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
                    target: audioStreamIconLoader

                    anchors.right: undefined
                    anchors.horizontalCenter: frame.horizontalCenter
                }

                PropertyChanges {
                    target: audioStreamIconLoader

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
