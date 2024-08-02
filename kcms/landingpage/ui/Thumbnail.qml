/*
    SPDX-FileCopyrightText: 2021 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2018 Furkan Tokac <furkantokac34@gmail.com>
    SPDX-FileCopyrightText: 2019 Nate Graham <nate@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami

QQC2.RadioButton {
    id: delegate

    property alias imageSource: image.source

    implicitWidth: contentItem.implicitWidth
    implicitHeight: contentItem.implicitHeight

    contentItem: ColumnLayout {
        spacing: 0

        Kirigami.ShadowedRectangle {
            implicitWidth: implicitHeight * 1.6
            implicitHeight: Kirigami.Units.gridUnit * 5
            radius: Kirigami.Units.cornerRadius
            Kirigami.Theme.inherit: false
            Kirigami.Theme.colorSet: Kirigami.Theme.View

            shadow.xOffset: 0
            shadow.yOffset: 2
            shadow.size: 10
            shadow.color: Qt.rgba(0, 0, 0, 0.3)

            color: {
                if (delegate.checked) {
                    return Kirigami.Theme.highlightColor;
                } else if (delegate.hovered) {
                    // Match appearance of hovered list items
                    return Qt.rgba(Kirigami.Theme.highlightColor.r,
                                Kirigami.Theme.highlightColor.g,
                                Kirigami.Theme.highlightColor.b,
                                0.5);
                } else {
                    return Kirigami.Theme.backgroundColor;
                }
            }

            Image {
                id: image
                anchors {
                    fill: parent
                    margins: Kirigami.Units.smallSpacing
                }
                asynchronous: true
                sourceSize: Qt.size(width * Screen.devicePixelRatio,
                                    height * Screen.devicePixelRatio)
            }
        }

        QQC2.Label {
            id: label
            Layout.fillWidth: true
            Layout.topMargin: Kirigami.Units.smallSpacing
            text: delegate.text
            textFormat: Text.PlainText
            horizontalAlignment: Text.AlignHCenter
        }

        Rectangle {
            Layout.preferredWidth: label.paintedWidth
            Layout.preferredHeight: 1
            Layout.alignment: Qt.AlignHCenter

            color: Kirigami.Theme.highlightColor

            opacity: delegate.visualFocus ? 1 : 0
        }
    }

    indicator: Item {}
    background: Item {}
}
