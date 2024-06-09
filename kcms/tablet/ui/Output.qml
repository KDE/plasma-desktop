/*
    SPDX-FileCopyrightText: 2019 Roman Gilg <subdiff@gmail.com>
    SPDX-FileCopyrightText: 2012 Dan Vratil <dvratil@redhat.com>

    SPDX-License-Identifier: GPL-2.0-or-later

    Adapted from KScreen
*/

import QtQuick
import QtQuick.Layouts
import Qt5Compat.GraphicalEffects

import org.kde.kirigami as Kirigami

Item {
    id: output

    property bool isSelected: true

    onIsSelectedChanged: {
        if (isSelected) {
            z = 89;
        } else {
            z = 0;
        }
    }

    Rectangle {
        id: outline
        radius: Kirigami.Units.cornerRadius
        color: Kirigami.Theme.backgroundColor

        anchors.fill: parent

        border {
            color: isSelected ? Kirigami.Theme.highlightColor : Kirigami.Theme.disabledTextColor
            width: 1

            ColorAnimation on color {
                duration: Kirigami.Units.longDuration
            }
        }
    }

    Item {
        id: orientationPanelContainer

        anchors.fill: output
        visible: false

        Rectangle {
            id: orientationPanel
            anchors {
                left: parent.left
                right: parent.right
                bottom: parent.bottom
            }

            height: Kirigami.Units.largeSpacing
            color: isSelected ? Kirigami.Theme.highlightColor : Kirigami.Theme.disabledTextColor
            smooth: true

            ColorAnimation on color {
                duration: Kirigami.Units.longDuration
            }
        }
    }

    OpacityMask {
        anchors.fill: orientationPanelContainer
        source: orientationPanelContainer
        maskSource: outline
    }
}

