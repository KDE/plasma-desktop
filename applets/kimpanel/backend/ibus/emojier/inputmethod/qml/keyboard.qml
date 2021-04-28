/*
    This file is part of the KDE project.

    SPDX-FileCopyrightText: 2016 Martin Flöser <mgraesslin@kde.org>
    SPDX-FileCopyrightText: 2021 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
import QtQuick 2.15
import QtQuick.Layouts 1.12
import QtQuick.Window 2.12
import QtQuick.Controls 2.15 as QQC2
import org.kde.kirigami 2.10 as Kirigami
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.emoji 1.0
import org.kde.plasma.emoji.im 1.0

PlasmaCore.Dialog
{
    id: rx
    visible: rep.count > 0
    backgroundHints: PlasmaCore.Dialog.StandardBackground

    property alias inputMethod: bracket.inputMethod

    mainItem: RowLayout {
        width: Math.min(Screen.width, emojiLayout.implicitWidth)
        height: PlasmaCore.Units.iconSizes.large

        id: emojiLayout

        InputBracket {
            id: bracket
        }

        EmojiModel {
            id: emoji
        }

        Repeater {
            id: rep
            model: SearchModelFilter {
                sourceModel: emoji
                search: bracket.bracketedText.length === 0 ? "soemthing-that-really-does-not-exist" : bracket.bracketedText
            }

            delegate: MouseArea {
                QQC2.Label {
                    font.pointSize: 25
                    font.family: 'emoji' // Avoid monochrome fonts like DejaVu Sans
                    fontSizeMode: model.display.length > 5 ? Text.Fit : Text.FixedSize
                    minimumPointSize: 10
                    text: model.display
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter

                    anchors.fill: parent
                    anchors.margins: 1
                }

                Layout.alignment: Qt.AlignVCenter
                width: PlasmaCore.Units.iconSizes.large
                height: PlasmaCore.Units.iconSizes.large

                QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
                QQC2.ToolTip.text: model.toolTip
                QQC2.ToolTip.visible: mouse.containsMouse

                opacity: mouse.containsMouse ? 0.7 : 1

                Keys.onReturnPressed: {
                    reportEmoji()
                }

                function reportEmoji() {
                    bracket.replaceBracket(model.display)
                }

                id: mouse
                hoverEnabled: true
                onClicked: reportEmoji()
            }
        }
    }
}
