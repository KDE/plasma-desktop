/*
    SPDX-FileCopyrightText: 2016 Kai Uwe Broulik <kde@privat.broulik.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts

import org.kde.plasma.components 3.0 as PlasmaComponents3
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.kirigami 2 as Kirigami
import org.kde.plasma.private.mpris as Mpris

Item {
    visible: instantiator.count > 0
    implicitHeight: Kirigami.Units.gridUnit * 3
    implicitWidth: Kirigami.Units.gridUnit * 16

    Repeater {
        id: instantiator
        model: Mpris.MultiplexerModel { }

        RowLayout {
            id: controlsRow

            anchors.fill: parent
            spacing: 0
            enabled: model.canControl

            Image {
                id: albumArt
                Layout.preferredWidth: height
                Layout.fillHeight: true
                visible: status === Image.Loading || status === Image.Ready
                asynchronous: true
                fillMode: Image.PreserveAspectFit
                source: model.artUrl
                sourceSize.height: height * Screen.devicePixelRatio
            }

            Item { // spacer
                width: Kirigami.Units.smallSpacing
                height: 1
            }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 0

                PlasmaComponents3.Label {
                    Layout.fillWidth: true
                    elide: Text.ElideRight
                    font.pointSize: Kirigami.Theme.defaultFont.pointSize + 1
                    maximumLineCount: 1
                    text: model.track.length > 0
                            ? model.track
                            : (model.playbackStatus > Mpris.PlaybackStatus.Stopped
                                ? i18nd("plasma_lookandfeel_org.kde.lookandfeel", "No title")
                                : i18nd("plasma_lookandfeel_org.kde.lookandfeel", "No media playing"))
                    textFormat: Text.PlainText
                    wrapMode: Text.NoWrap
                }

                PlasmaExtras.DescriptiveLabel {
                    Layout.fillWidth: true
                    wrapMode: Text.NoWrap
                    elide: Text.ElideRight
                    // if no artist is given, show player name instead
                    text: model.artist || model.identity
                    textFormat: Text.PlainText
                    font.pointSize: Kirigami.Theme.smallFont.pointSize + 1
                    maximumLineCount: 1
                }
            }

            PlasmaComponents3.ToolButton {
                Layout.preferredHeight: Kirigami.Units.gridUnit * 2
                Layout.preferredWidth: Layout.preferredHeight
                visible: model.canGoBack || model.canGoNext
                enabled: model.canGoPrevious
                focusPolicy: Qt.TabFocus
                icon.name: LayoutMirroring.enabled ? "media-skip-forward" : "media-skip-backward"
                onClicked: {
                    fadeoutTimer.running = false
                    model.container.Previous()
                }
                Accessible.name: i18nd("plasma_lookandfeel_org.kde.lookandfeel", "Previous track")
            }

            PlasmaComponents3.ToolButton {
                Layout.fillHeight: true
                Layout.preferredWidth: height // make this button bigger
                focusPolicy: Qt.TabFocus
                icon.name: model.playbackStatus === Mpris.PlaybackStatus.Playing ? "media-playback-pause" : "media-playback-start"
                onClicked: {
                    fadeoutTimer.running = false
                    model.container.PlayPause()
                }
                Accessible.name: i18nd("plasma_lookandfeel_org.kde.lookandfeel", "Play or Pause media")
            }

            PlasmaComponents3.ToolButton {
                Layout.preferredHeight: Kirigami.Units.gridUnit * 2
                Layout.preferredWidth: Layout.preferredHeight
                visible: model.canGoBack || model.canGoNext
                enabled: model.canGoNext
                focusPolicy: Qt.TabFocus
                icon.name: LayoutMirroring.enabled ? "media-skip-backward" : "media-skip-forward"
                Accessible.name: i18nd("plasma_lookandfeel_org.kde.lookandfeel", "Next track")
                onClicked: {
                    fadeoutTimer.running = false
                    model.container.Next()
                }
            }
        }
    }
}
