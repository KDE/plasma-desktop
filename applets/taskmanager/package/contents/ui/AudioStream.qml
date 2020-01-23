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
                     duration: units.longDuration
                 }
             }
        },
        Transition {
             from: ""
             to: "muted"
             SequentialAnimation {
                 NumberAnimation {
                     property: "opacity"
                     duration: units.longDuration
                 }
             }
        },
        Transition {
             to: ""
             NumberAnimation {
                 property: "opacity"
                 duration: units.longDuration
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
        svg: audioSvg
        smooth: false

        height: Math.round(Math.min(parent.height * 1.4, units.iconSizes.smallMedium) / 2) * 2
        width: height

        anchors {
            verticalCenter: parent.verticalCenter
            right: parent.right
            // Avoid overlap to the right.
            rightMargin: (parent.width - width) / 2 + units.smallSpacing / 4
        }

        states: [
            State {
                name: "horizontal"
                // When there is enough space for the audio icon, to fit right of the centered task icon.
                when: (frame.width > Math.min(iconBox.width, iconBox.height) +
                       Math.min(Math.min(iconBox.width, iconBox.height), units.iconSizes.smallMedium) * 2)

                AnchorChanges {
                    target: audioStreamIconLoader

                    anchors.top: undefined
                    anchors.verticalCenter: frame.verticalCenter
                }

                PropertyChanges {
                    target: audioStreamIconLoader

                    anchors.rightMargin: iconBox.adjustMargin(true, parent.width, taskFrame.margins.right)
                    width: units.roundToIconSize(Math.min(Math.min(iconBox.width, iconBox.height), units.iconSizes.smallMedium))
                }

                PropertyChanges {
                    target: audioStreamIcon

                    height: parent.height
                    width: parent.width
                }
            },

            State {
                name: "vertical"
                // When audio icon can fit above the centered task icon.
                when: (frame.height > Math.min(iconBox.width, iconBox.height) +
                       Math.min(Math.min(iconBox.width, iconBox.height), units.iconSizes.smallMedium) * 2)

                AnchorChanges {
                    target: audioStreamIconLoader

                    anchors.right: undefined
                    anchors.horizontalCenter: frame.horizontalCenter
                }

                PropertyChanges {
                    target: audioStreamIconLoader

                    anchors.topMargin: iconBox.adjustMargin(false, frame.height, taskFrame.margins.top)
                    width: units.roundToIconSize(Math.min(Math.min(iconBox.width, iconBox.height), units.iconSizes.smallMedium))
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
