/*
    SPDX-FileCopyrightText: 2013 Sebastian Kügler <sebas@kde.org>
    SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>
    SPDX-FileCopyrightText: 2016 Kai Uwe Broulik <kde@privat.broulik.de>
    SPDX-FileCopyrightText: 2017 Roman Gilg <subdiff@gmail.com>
    SPDX-FileCopyrightText: 2020 Nate Graham <nate@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.6
import QtQuick.Layouts 1.1
import QtGraphicalEffects 1.0
import QtQml.Models 2.2

import org.kde.plasma.core 2.0 as PlasmaCore
// for Highlight
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.components 3.0 as PlasmaComponents3
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.kquickcontrolsaddons 2.0 as KQuickControlsAddons

ColumnLayout {
    property var submodelIndex
    property int flatIndex: isGroup && index != undefined ? index : 0

    readonly property bool playing: hasPlayer && playerData.PlaybackStatus === "Playing"
    readonly property bool canControl: hasPlayer && playerData.CanControl
    readonly property bool canPlay: hasPlayer && playerData.CanPlay
    readonly property bool canPause: hasPlayer && playerData.CanPause
    readonly property bool canGoBack: hasPlayer && playerData.CanGoPrevious
    readonly property bool canGoNext: hasPlayer && playerData.CanGoNext
    readonly property bool canRaise: hasPlayer && playerData.CanRaise
    readonly property var currentMetadata: hasPlayer ? playerData.Metadata : ({})

    readonly property string track: {
        var xesamTitle = currentMetadata["xesam:title"]
        if (xesamTitle) {
            return xesamTitle;
        }
        // if no track title is given, print out the file name
        var xesamUrl = currentMetadata["xesam:url"] ? currentMetadata["xesam:url"].toString() : ""
        if (!xesamUrl) {
            return "";
        }
        var lastSlashPos = xesamUrl.lastIndexOf('/')
        if (lastSlashPos < 0) {
            return "";
        }
        var lastUrlPart = xesamUrl.substring(lastSlashPos + 1)
        return decodeURIComponent(lastUrlPart);
    }
    readonly property string artist: currentMetadata["xesam:artist"] || ""
    readonly property string albumArt: currentMetadata["mpris:artUrl"] || ""

    spacing: PlasmaCore.Units.smallSpacing

    // text labels + close button
    RowLayout {
        id: header
        // match spacing of DefaultToolTip.qml in plasma-framework
        spacing: isWin ? PlasmaCore.Units.smallSpacing : PlasmaCore.Units.largeSpacing

        // This number controls the overall size of the window tooltips
        Layout.maximumWidth: PlasmaCore.Units.gridUnit * 16
        Layout.minimumWidth: isWin ? Layout.maximumWidth : 0
        Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
        // match margins of DefaultToolTip.qml in plasma-framework
        Layout.margins: isWin ? 0 : PlasmaCore.Units.gridUnit / 2

        // all textlabels
        ColumnLayout {
            spacing: 0
            // app name
            PlasmaExtras.Heading {
                id: appNameHeading
                level: 3
                maximumLineCount: 1
                lineHeight: isWin ? 1 : appNameHeading.lineHeight
                Layout.fillWidth: true
                elide: Text.ElideRight
                text: appName
                opacity: flatIndex == 0
                visible: text.length !== 0
                textFormat: Text.PlainText
            }
            // window title
            PlasmaComponents.Label {
                readonly property string title: generateTitle()
                id: winTitle
                maximumLineCount: 1
                Layout.fillWidth: true
                elide: Text.ElideRight
                text: (!hasPlayer || !title.includes(songText.text)) ? title : ""
                opacity: 0.75
                visible: title.length !== 0 && title !== appNameHeading.text
                textFormat: Text.PlainText
            }
            // subtext
            PlasmaComponents.Label {
                id: subtext
                maximumLineCount: 1
                Layout.fillWidth: true
                elide: Text.ElideRight
                text: isWin ? generateSubText() : ""
                opacity: 0.6
                visible: text.length !== 0 && text !== appNameHeading.text
                textFormat: Text.PlainText
            }
        }

        // Count badge.
        // The badge itself is inside an item to better center the text in the bubble
        Item {
            Layout.alignment: Qt.AlignRight | Qt.AlignTop
            Layout.preferredHeight: closeButton.height
            Layout.preferredWidth: closeButton.width
            visible: flatIndex === 0 && smartLauncherCountVisible

            Badge {
                anchors.centerIn: parent
                height: PlasmaCore.Units.iconSizes.smallMedium
                number: smartLauncherCount
            }
        }

        // close button
        PlasmaComponents3.ToolButton {
            id: closeButton
            Layout.alignment: Qt.AlignRight | Qt.AlignTop
            visible: isWin
            icon.name: "window-close"
            onClicked: {
                backend.cancelHighlightWindows();
                tasksModel.requestClose(submodelIndex);
            }
        }
    }

    // thumbnail container
    Item {
        id: thumbnailSourceItem

        Layout.minimumWidth: header.width
        Layout.preferredHeight: header.width / 2

        clip: true
        visible: toolTipDelegate.isWin

        readonly property bool isMinimized: isGroup ? IsMinimized == true : isMinimizedParent
        // TODO: this causes XCB error message when being visible the first time
        readonly property var winId: toolTipDelegate.isWin && toolTipDelegate.windows[flatIndex] !== undefined ? toolTipDelegate.windows[flatIndex] : 0

        // There's no PlasmaComponents3 version
        PlasmaComponents.Highlight {
            anchors.fill: hoverHandler
            visible: hoverHandler.containsMouse
            pressed: hoverHandler.containsPress
        }

        PlasmaCore.WindowThumbnail {
            id: x11Thumbnail

            anchors.fill: hoverHandler
            // Indent a little bit so that neither the thumbnail nor the drop
            // shadow can cover up the highlight
            anchors.margins: PlasmaCore.Units.smallSpacing * 2


            visible: !albumArtImage.visible && !thumbnailSourceItem.isMinimized && Number.isInteger(thumbnailSourceItem.winId)
            winId: Number.isInteger(thumbnailSourceItem.winId) ? thumbnailSourceItem.winId : 0
        }

        Loader {
            id: pipeWireLoader
            anchors.fill: hoverHandler
            // Indent a little bit so that neither the thumbnail nor the drop
            // shadow can cover up the highlight
            anchors.margins: PlasmaCore.Units.smallSpacing * 2

            active: !albumArtImage.visible && !Number.isInteger(thumbnailSourceItem.winId)

            //In a loader since we might not have PipeWire available yet
            source: "PipeWireThumbnail.qml"
        }

        DropShadow {
            anchors.fill: pipeWireLoader.active ? pipeWireLoader : x11Thumbnail
            visible: pipeWireLoader.active ? pipeWireLoader.item.visible : x11Thumbnail.visible
            horizontalOffset: 0
            verticalOffset: Math.round(3 * PlasmaCore.Units.devicePixelRatio)
            radius: Math.round(8.0 * PlasmaCore.Units.devicePixelRatio)
            samples: Math.round(radius * 1.5)
            color: "Black"
            source: pipeWireLoader.active ? pipeWireLoader.item : x11Thumbnail
        }

        ShaderEffect {
            id: albumArtBackground
            readonly property Image source: albumArtImage

            anchors.centerIn: parent
            visible: source.available
            layer.enabled: true
            opacity: 0.25
            layer.effect: FastBlur {
                source: albumArtBackground
                anchors.fill: source
                radius: 30
            }
            // use State to avoid unnecessary reevaluation of width and height
            // otherwise the size will be reevaluated many times when isGroup is true
            states: State {
                name: "albumArtImageReady"
                // paintedWidth and paintedHeight become positive when image is ready
                when: albumArtImage.available && albumArtImage.status === Image.Ready
                PropertyChanges {
                    target: albumArtBackground
                    // set explicit to true to further avoid unnecessary reevaluation of size
                    explicit: true
                    // manual implementation of Image.PreserveAspectCrop
                    width: (source.paintedWidth / source.paintedHeight) <= (parent.width / parent.height) ? parent.width : parent.height * (source.paintedWidth / source.paintedHeight)
                    height: (source.paintedWidth / source.paintedHeight) <= (parent.width / parent.height) ? parent.width * (source.paintedHeight / source.paintedWidth) : parent.height
                }
            }
        }

        Image {
            id: albumArtImage
            // also Image.Loading to prevent loading thumbnails just because the album art takes a split second to load
            // don't show album art if window title doesn't include media title (eg we're in a different browser tab)
            readonly property bool available: (status === Image.Ready || status === Image.Loading) && generateTitle().includes(track)

            anchors.fill: hoverHandler
            // Indent by one pixel to make sure we never cover up the entire highlight
            anchors.margins: 1
            sourceSize: Qt.size(parent.width, parent.height)

            asynchronous: true
            source: albumArt
            fillMode: Image.PreserveAspectFit
            visible: available
        }

        // when minimized, we don't have a preview on X11, so show the icon
        PlasmaCore.IconItem {
            width: parent.width
            height: thumbnailSourceItem.height
            anchors.horizontalCenter: parent.horizontalCenter
            source: thumbnailSourceItem.isMinimized && !albumArtImage.visible && Number.isInteger(thumbnailSourceItem.winId) ? icon : ""
            animated: false
            usesPlasmaTheme: false
            visible: valid && !pipeWireLoader.active
        }

        ToolTipWindowMouseArea {
            id: hoverHandler
            anchors.fill: parent
            rootTask: parentTask
            modelIndex: submodelIndex
            winId: thumbnailSourceItem.winId
        }
    }

    // Player controls row
    RowLayout {
        Layout.maximumWidth: header.Layout.maximumWidth
        // Match margins of header
        Layout.leftMargin: isWin ? 0 : PlasmaCore.Units.gridUnit / 2
        Layout.rightMargin: isWin ? 0 : PlasmaCore.Units.gridUnit / 2

        visible: hasPlayer
        enabled: canControl

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
                    text: track || ""
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
                    text: artist || ""
                    font: PlasmaCore.Theme.smallestFont
                    textFormat: Text.PlainText
                }
            }
        }

        PlasmaComponents3.ToolButton {
            enabled: canGoBack
            icon.name: LayoutMirroring.enabled ? "media-skip-forward" : "media-skip-backward"
            onClicked: mpris2Source.goPrevious(mprisSourceName)
        }

        PlasmaComponents3.ToolButton {
            enabled: playing ? canPause : canPlay
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
            enabled: canGoNext
            icon.name: LayoutMirroring.enabled ? "media-skip-backward" : "media-skip-forward"
            onClicked: mpris2Source.goNext(mprisSourceName)
        }
    }

    function generateTitle() {
        if (!isWin) {
            return genericName != undefined ? genericName : "";
        }

        var text;
        if (isGroup) {
            if (model.display === undefined) {
                return "";
            }
            text = model.display.toString();
        } else {
            text = displayParent;
        }

        // KWin appends increasing integers in between pointy brackets to otherwise equal window titles.
        // In this case save <#number> as counter and delete it at the end of text.
        var counter = text.match(/<\d+>\W*$/);
        text = text.replace(/\s*<\d+>\W*$/, "");

        // Remove appName from the end of text.
        var appNameRegex = new RegExp(appName + "$", "i");
        text = text.replace(appNameRegex, "");
        text = text.replace(/\s*(?:-|—)*\s*$/, "");

        // Add counter back at the end.
        if (counter !== null) {
            if (text === "") {
                text = counter;
            } else {
                text = text + " " + counter;
            }
        }

        // In case the window title had only redundant information (i.e. appName), text is now empty.
        // Add a hyphen to indicate that and avoid empty space.
        if (text === "") {
            text = "—";
        }
        return text.toString();
    }

    function generateSubText() {
        if (activitiesParent === undefined) {
            return "";
        }

        var subTextEntries = [];

        var onAllDesktops = (isGroup ? IsOnAllVirtualDesktops : isOnAllVirtualDesktopsParent) === true;
        if (!plasmoid.configuration.showOnlyCurrentDesktop && virtualDesktopInfo.numberOfDesktops > 1) {
            var virtualDesktops = isGroup ? VirtualDesktops : virtualDesktopParent;

            if (!onAllDesktops && virtualDesktops !== undefined && virtualDesktops.length > 0) {
                var virtualDesktopNameList = new Array();

                for (var i = 0; i < virtualDesktops.length; ++i) {
                    virtualDesktopNameList.push(virtualDesktopInfo.desktopNames[virtualDesktopInfo.desktopIds.indexOf(virtualDesktops[i])]);
                }

                subTextEntries.push(i18nc("Comma-separated list of desktops", "On %1",
                    virtualDesktopNameList.join(", ")));
            } else if (onAllDesktops) {
                subTextEntries.push(i18nc("Comma-separated list of desktops", "Pinned to all desktops"));
            }
        }

        var act = isGroup ? Activities : activitiesParent;
        if (act === undefined) {
            return subTextEntries.join("\n");
        }

        if (act.length === 0 && activityInfo.numberOfRunningActivities > 1) {
            subTextEntries.push(i18nc("Which virtual desktop a window is currently on",
                "Available on all activities"));
        } else if (act.length > 0) {
           var activityNames = [];

            for (var i = 0; i < act.length; i++) {
                var activity = act[i];
                var activityName = activityInfo.activityName(act[i]);
                if (activityName === "") {
                    continue;
                }
                if (plasmoid.configuration.showOnlyCurrentActivity) {
                    if (activity !== activityInfo.currentActivity) {
                        activityNames.push(activityName);
                    }
                } else if (activity !== activityInfo.currentActivity) {
                    activityNames.push(activityName);
                }
            }

            if (plasmoid.configuration.showOnlyCurrentActivity) {
                if (activityNames.length > 0) {
                    subTextEntries.push(i18nc("Activities a window is currently on (apart from the current one)",
                        "Also available on %1", activityNames.join(", ")));
                }
            } else if (activityNames.length > 0) {
                subTextEntries.push(i18nc("Which activities a window is currently on",
                    "Available on %1", activityNames.join(", ")));
            }
        }

        return subTextEntries.join("\n");
    }
}
