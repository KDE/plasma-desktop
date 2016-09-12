/*
*   Copyright 2013 by Sebastian Kügler <sebas@kde.org>
*   Copyright 2014 by Martin Gräßlin <mgraesslin@kde.org>
*   Copyright 2016 by Kai Uwe Broulik <kde@privat.broulik.de>
*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU Library General Public License as
*   published by the Free Software Foundation; either version 2, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU Library General Public License for more details
*
*   You should have received a copy of the GNU Library General Public
*   License along with this program; if not, write to the
*   Free Software Foundation, Inc.,
*   51 Franklin Street, Fifth Floor, Boston, MA  2.010-1301, USA.
*/

import QtQuick 2.0
import QtQuick.Layouts 1.1
import QtGraphicalEffects 1.0

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.kquickcontrolsaddons 2.0 as KQuickControlsAddons

Column {
    id: tooltipContentItem

    property Item toolTip
    property var parentIndex
    property var windows
    property string mainText
    property string subText
    property variant icon
    property url launcherUrl
    property bool group: (windows !== undefined && windows.length > 1)

    readonly property string mprisSourceName: mpris2Source.sourceNameForLauncherUrl(launcherUrl)
    readonly property bool hasPlayer: !!mprisSourceName && !!playerData

    readonly property var playerData: mpris2Source.data[mprisSourceName]
    readonly property bool playing: hasPlayer && playerData.PlaybackStatus === "Playing"
    readonly property bool canControl: hasPlayer && playerData.CanControl
    readonly property bool canGoBack: hasPlayer && playerData.CanGoPrevious
    readonly property bool canGoNext: hasPlayer && playerData.CanGoNext
    readonly property bool canRaise: hasPlayer && playerData.CanRaise
    readonly property var currentMetadata: hasPlayer ? playerData.Metadata : ({})

    readonly property string track: {
        var xesamTitle = currentMetadata["xesam:title"]
        if (xesamTitle) {
            return xesamTitle
        }
        // if no track title is given, print out the file name
        var xesamUrl = currentMetadata["xesam:url"] ? currentMetadata["xesam:url"].toString() : ""
        if (!xesamUrl) {
            return ""
        }
        var lastSlashPos = xesamUrl.lastIndexOf('/')
        if (lastSlashPos < 0) {
            return ""
        }
        var lastUrlPart = xesamUrl.substring(lastSlashPos + 1)
        return decodeURIComponent(lastUrlPart)
    }
    readonly property string artist: currentMetadata["xesam:artist"] || ""
    readonly property string albumArt: currentMetadata["mpris:artUrl"] || ""

    readonly property int thumbnailWidth: units.gridUnit * 15
    readonly property int thumbnailHeight: units.gridUnit * 10

    property int preferredTextWidth: theme.mSize(theme.defaultFont).width * 30

    Layout.minimumWidth: Math.max(thumbnailWidth, windowRow.width, appLabelRow.width) + units.largeSpacing / 2
    Layout.minimumHeight: childrenRect.height + units.largeSpacing
    Layout.maximumWidth: Layout.minimumWidth
    Layout.maximumHeight: Layout.minimumHeight

    LayoutMirroring.enabled: Qt.application.layoutDirection === Qt.RightToLeft
    LayoutMirroring.childrenInherit: true

    spacing: units.largeSpacing / 2

    states: State {
        when: tooltipContentItem.hasPlayer

        PropertyChanges {
            target: thumbnailSourceItem
            opacity: 0 // cannot set visible to false or else WindowThumbnail won't provide thumbnail
        }
        PropertyChanges {
            target: playerControlsOpacityMask
            visible: true
            source: thumbnailSourceItem
            maskSource: playerControlsShadowMask
        }
        PropertyChanges {
            target: playerControlsRow
            visible: true
        }
    }

    Item {
        id: thumbnailContainer
        width: Math.max(parent.width, windowRow.width)
        height: albumArtImage.available ? albumArtImage.height :
                raisePlayerArea.visible ? raisePlayerArea.height :
                                          windowRow.height

        Item {
            id: thumbnailSourceItem
            anchors.fill: parent

            PlasmaExtras.ScrollArea {
                id: scrollArea
                anchors.horizontalCenter: parent.horizontalCenter
                width: Math.max(windowRow.width, thumbnailWidth)
                height: parent.height

                visible: !albumArtImage.available

                verticalScrollBarPolicy: Qt.ScrollBarAlwaysOff
                horizontalScrollBarPolicy: Qt.ScrollBarAlwaysOff

                Component.onCompleted: {
                    flickableItem.interactive = Qt.binding(function() {
                        return contentItem.width > viewport.width;
                    });
                }

                Row {
                    id: windowRow
                    spacing: units.largeSpacing

                    Repeater {
                        model: plasmoid.configuration.showToolTips && !albumArtImage.available ? windows : null

                        PlasmaCore.WindowThumbnail {
                            id: windowThumbnail

                            width: thumbnailWidth
                            height: thumbnailHeight

                            winId: modelData

                            ToolTipWindowMouseArea {
                                anchors.fill: parent
                                modelIndex: tasksModel.makeModelIndex(parentIndex, group ? index : -1)
                                winId: modelData
                                thumbnailItem: parent
                            }
                        }
                    }
                }
            }

            Image {
                id: albumArtImage
                // also Image.Loading to prevent loading thumbnails just because the album art takes a split second to load
                readonly property bool available: status === Image.Ready || status === Image.Loading

                anchors.centerIn: parent
                width: parent.width
                height: thumbnailHeight
                sourceSize: Qt.size(thumbnailWidth, thumbnailHeight)
                asynchronous: true
                source: albumArt
                fillMode: Image.PreserveAspectCrop
                visible: available

                ToolTipWindowMouseArea {
                    anchors.fill: parent
                    modelIndex: tasksModel.makeModelIndex(parentIndex, group ? index : -1)
                    winId: windows != undefined ? (windows[0] || 0) : 0
                }
            }

            MouseArea {
                id: raisePlayerArea
                anchors.centerIn: parent
                width: thumbnailWidth
                height: thumbnailHeight

                // if there's no window associated with this task, we might still be able to raise the player
                visible: windows == undefined || !windows[0] && canRaise
                onClicked: mpris2Source.raise(mprisSourceName)

                PlasmaCore.IconItem {
                    anchors.fill: parent
                    source: icon
                    animated: false
                    usesPlasmaTheme: false
                    visible: !albumArtImage.available
                }
            }
        }

        Item {
            id: playerControlsShadowMask
            anchors.fill: thumbnailSourceItem
            visible: false // OpacityMask would render it

            Rectangle {
                width: parent.width
                height: parent.height - playerControlsRow.height
            }

            Rectangle {
                anchors.bottom: parent.bottom
                width: parent.width
                height: playerControlsRow.height
                opacity: 0.2
            }
        }

        OpacityMask {
            id: playerControlsOpacityMask
            anchors.fill: thumbnailSourceItem
            visible: false
        }

        // prevent accidental click-through when a control is disabled
        MouseArea {
            anchors.fill: playerControlsRow
            enabled: playerControlsRow.visible
        }

        RowLayout {
            id: playerControlsRow
            anchors {
                horizontalCenter: parent.horizontalCenter
                bottom: thumbnailSourceItem.bottom
            }
            width: thumbnailWidth
            spacing: 0
            enabled: canControl
            visible: false

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 0

                PlasmaExtras.Heading {
                    Layout.fillWidth: true
                    level: 4
                    wrapMode: Text.NoWrap
                    elide: Text.ElideRight
                    text: track || ""
                }

                PlasmaExtras.Heading {
                    Layout.fillWidth: true
                    level: 5
                    wrapMode: Text.NoWrap
                    elide: Text.ElideRight
                    text: artist || ""
                }
            }

            PlasmaComponents.ToolButton {
                enabled: canGoBack
                iconName: LayoutMirroring.enabled ? "media-skip-forward" : "media-skip-backward"
                tooltip: i18nc("Go to previous song", "Previous")
                Accessible.name: tooltip
                onClicked: mpris2Source.goPrevious(mprisSourceName)
            }

            PlasmaComponents.ToolButton {
                Layout.fillHeight: true
                Layout.preferredWidth: height // make this button bigger
                iconName: playing ? "media-playback-pause" : "media-playback-start"
                tooltip: playing ? i18nc("Pause player", "Pause") : i18nc("Start player", "Play")
                Accessible.name: tooltip
                onClicked: mpris2Source.playPause(mprisSourceName)
            }

            PlasmaComponents.ToolButton {
                enabled: canGoNext
                iconName: LayoutMirroring.enabled ? "media-skip-backward" : "media-skip-forward"
                tooltip: i18nc("Go to next song", "Next")
                Accessible.name: tooltip
                onClicked: mpris2Source.goNext(mprisSourceName)
            }
        }
    }

    Row {
        id: appLabelRow
        anchors.left: parent.left
        spacing: units.largeSpacing

        Item {
            id: imageContainer
            width: tooltipIcon.width
            height: tooltipIcon.height

            PlasmaCore.IconItem {
                id: tooltipIcon
                anchors {
                    left: parent.left
                    leftMargin: units.largeSpacing / 2
                }
                width: units.iconSizes.desktop
                height: width
                animated: false
                usesPlasmaTheme: false
                source: icon
            }
        }

        Column {
            id: mainColumn
            spacing: units.smallSpacing

            //This instance is purely for metrics
            PlasmaExtras.Heading {
                id: tooltipMaintextPlaceholder
                visible: false
                level: 3
                text: mainText
                textFormat: Text.PlainText
            }
            PlasmaExtras.Heading {
                id: tooltipMaintext
                level: 3
                width: Math.min(tooltipMaintextPlaceholder.width, preferredTextWidth)
                height: undefined // unset stupid PlasmaComponents.Label default height
                //width: 400
                elide: Text.ElideRight
                wrapMode: Text.WordWrap
                // if there's no subtext allow two lines of window title
                maximumLineCount: tooltipSubtext.visible ? 1 : 2
                lineHeight: 0.95
                text: mainText
                textFormat: Text.PlainText
            }
            PlasmaComponents.Label {
                id: tooltipSubtext
                width: tooltipContentItem.preferredTextWidth
                height: Math.min(theme.mSize(theme.defaultFont), contentHeight)
                wrapMode: Text.WordWrap
                text: subText
                textFormat: Text.PlainText
                opacity: 0.5
                visible: text !== ""
            }
        }
    }
}
