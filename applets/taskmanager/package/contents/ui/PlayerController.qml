/*
    SPDX-FileCopyrightText: 2013 Sebastian Kügler <sebas@kde.org>
    SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>
    SPDX-FileCopyrightText: 2016 Kai Uwe Broulik <kde@privat.broulik.de>
    SPDX-FileCopyrightText: 2017 Roman Gilg <subdiff@gmail.com>
    SPDX-FileCopyrightText: 2020 Nate Graham <nate@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Layouts 1.15

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 3.0 as PlasmaComponents3
import org.kde.plasma.extras 2.0 as PlasmaExtras

RowLayout {
    enabled: !!playerData.CanControl

    readonly property string mprisSourceName: mpris2Source.sourceNameForLauncherUrl(launcherUrl, appPid)
    readonly property var playerData: mprisSourceName != "" ? mpris2Source.data[mprisSourceName] : 0
    readonly property bool playing: !!(playerData.PlaybackStatus === "Playing")

    readonly property var currentMetadata: !!playerData ? playerData.Metadata : ""

    readonly property string track: {
        const xesamTitle = currentMetadata["xesam:title"]
        if (xesamTitle) {
            return xesamTitle;
        }
        // if no track title is given, print out the file name
        const xesamUrl = currentMetadata["xesam:url"] ? currentMetadata["xesam:url"].toString() : ""
        if (!xesamUrl) {
            return "";
        }
        const lastSlashPos = xesamUrl.lastIndexOf('/')
        if (lastSlashPos < 0) {
            return "";
        }
        const lastUrlPart = xesamUrl.substring(lastSlashPos + 1)
        return decodeURIComponent(lastUrlPart) || "";
    }
    readonly property var artists: currentMetadata["xesam:artist"] || [] // stringlist
    readonly property var albumArtists: currentMetadata["xesam:albumArtist"] || [] // stringlist
    readonly property string albumArt: currentMetadata["mpris:artUrl"] || ""

    ColumnLayout {
        Layout.fillWidth: true
        Layout.topMargin: PlasmaCore.Units.smallSpacing
        Layout.bottomMargin: PlasmaCore.Units.smallSpacing
        Layout.rightMargin: isWin ? PlasmaCore.Units.smallSpacing : PlasmaCore.Units.largeSpacing
        spacing: 0

        ScrollableTextWrapper {
            id: songTextWrapper

            Layout.fillWidth: true
            Layout.preferredHeight: songText.height
            implicitWidth: songText.implicitWidth

            PlasmaComponents3.Label {
                id: songText
                parent: songTextWrapper
                width: parent.width
                height: undefined
                lineHeight: 1
                maximumLineCount: artistText.visible? 1 : 2
                wrapMode: Text.NoWrap
                elide: parent.state ? Text.ElideNone : Text.ElideRight
                text: track
                textFormat: Text.PlainText
            }
        }

        ScrollableTextWrapper {
            id: artistTextWrapper

            Layout.fillWidth: true
            Layout.preferredHeight: artistText.height
            implicitWidth: artistText.implicitWidth
            visible: artistText.text !== ""

            PlasmaExtras.DescriptiveLabel {
                id: artistText
                parent: artistTextWrapper
                width: parent.width
                height: undefined
                wrapMode: Text.NoWrap
                lineHeight: 1
                elide: parent.state ? Text.ElideNone : Text.ElideRight
                text: artists.length > 0 ? artists.join(", ") : albumArtists.join(", ")
                font: PlasmaCore.Theme.smallestFont
                textFormat: Text.PlainText
            }
        }
    }

    PlasmaComponents3.ToolButton {
        enabled: !!playerData.CanGoPrevious
        icon.name: LayoutMirroring.enabled ? "media-skip-forward" : "media-skip-backward"
        onClicked: mpris2Source.goPrevious(mprisSourceName)
    }

    PlasmaComponents3.ToolButton {
        enabled: playing ? !!playerData.CanPause : !!playerData.CanPlay
        icon.name: playing ? "media-playback-pause" : "media-playback-start"
        onClicked: {
            if (!playing) {
                mpris2Source.play(mprisSourceName);
            } else {
                mpris2Source.pause(mprisSourceName);
            }
        }
    }

    PlasmaComponents3.ToolButton {
        enabled: !!playerData.CanGoNext
        icon.name: LayoutMirroring.enabled ? "media-skip-backward" : "media-skip-forward"
        onClicked: mpris2Source.goNext(mprisSourceName)
    }
}
