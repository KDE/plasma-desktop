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
import QtGraphicalEffects 1.15

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 3.0 as PlasmaComponents3
import org.kde.plasma.extras 2.0 as PlasmaExtras

ColumnLayout {
    property var submodelIndex
    property int flatIndex: isGroup && index != undefined ? index : 0
    readonly property int appPid: isGroup && model.AppPid !== undefined ? model.AppPid : pidParent

    // HACK: Avoid blank space in the tooltip after closing a window
    ListView.onPooled: width = height = 0
    ListView.onReused: width = height = undefined

    readonly property string title: {
        if (!isWin) {
            return genericName || "";
        }

        let text;
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
        text = `${(text.match(/.*(?=\s*(—|-))/) || [""])[0]}${(text.match(/<\d+>/) || [""]).pop()}`;

        // In case the window title had only redundant information (i.e. appName), text is now empty.
        // Add a hyphen to indicate that and avoid empty space.
        if (text === "") {
            text = "—";
        }
        return text;
    }
    readonly property bool titleIncludesTrack: playerController.item
        && title.includes(playerController.item.track)

    spacing: PlasmaCore.Units.smallSpacing

    // text labels + close button
    RowLayout {
        id: header
        // match spacing of DefaultToolTip.qml in plasma-framework
        spacing: isWin ? PlasmaCore.Units.smallSpacing : PlasmaCore.Units.largeSpacing

        // This number controls the overall size of the window tooltips
        Layout.maximumWidth: toolTipDelegate.tooltipInstanceMaximumWidth
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
            PlasmaComponents3.Label {
                id: winTitle
                maximumLineCount: 1
                Layout.fillWidth: true
                elide: Text.ElideRight
                text: titleIncludesTrack ? "" : title
                opacity: 0.75
                visible: title.length !== 0 && title !== appNameHeading.text
                textFormat: Text.PlainText
            }
            // subtext
            PlasmaComponents3.Label {
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
        PlasmaExtras.Highlight {
            anchors.fill: hoverHandler
            visible: hoverHandler.item ? hoverHandler.item.containsMouse : false
            pressed: hoverHandler.item ? hoverHandler.item.containsPress : false
            hovered: true
        }

        Loader {
            id: thumbnailLoader
            active: !albumArtImage.visible && Number.isInteger(thumbnailSourceItem.winId) && flatIndex !== -1 // Avoid loading when the instance is going to be destroyed
            asynchronous: true
            visible: active
            anchors.fill: hoverHandler
            // Indent a little bit so that neither the thumbnail nor the drop
            // shadow can cover up the highlight
            anchors.margins: PlasmaCore.Units.smallSpacing * 2

            sourceComponent: thumbnailSourceItem.isMinimized ? iconItem : x11Thumbnail

            Component {
                id: x11Thumbnail

                PlasmaCore.WindowThumbnail {
                    winId: thumbnailSourceItem.winId
                }
            }

            // when minimized, we don't have a preview on X11, so show the icon
            Component {
                id: iconItem

                PlasmaCore.IconItem {
                    source: icon
                    animated: false
                    usesPlasmaTheme: false
                    visible: valid
                }
            }
        }

        Loader {
            id: pipeWireLoader
            anchors.fill: hoverHandler
            // Indent a little bit so that neither the thumbnail nor the drop
            // shadow can cover up the highlight
            anchors.margins: thumbnailLoader.anchors.margins

            active: !albumArtImage.visible && !Number.isInteger(thumbnailSourceItem.winId) && flatIndex !== -1
            asynchronous: true
            //In a loader since we might not have PipeWire available yet (WITH_PIPEWIRE could be undefined in plasma-workspace/libtaskmanager/declarative/taskmanagerplugin.cpp)
            source: "PipeWireThumbnail.qml"
        }

        Loader {
            active: (pipeWireLoader.item && pipeWireLoader.item.visible) || (thumbnailLoader.status === Loader.Ready && !thumbnailSourceItem.isMinimized)
            asynchronous: true
            visible: active
            anchors.fill: pipeWireLoader.active ? pipeWireLoader : thumbnailLoader

            sourceComponent: DropShadow {
                horizontalOffset: 0
                verticalOffset: Math.round(3 * PlasmaCore.Units.devicePixelRatio)
                radius: Math.round(8.0 * PlasmaCore.Units.devicePixelRatio)
                samples: Math.round(radius * 1.5)
                color: "Black"
                source: pipeWireLoader.active ? pipeWireLoader.item : thumbnailLoader.item // source could be undefined when albumArt is available, so put it in a Loader.
            }
        }

        Loader {
            active: albumArtImage.visible && albumArtImage.status === Image.Ready && flatIndex !== -1 // Avoid loading when the instance is going to be destroyed
            asynchronous: true
            visible: active
            anchors.centerIn: hoverHandler

            sourceComponent: ShaderEffect {
                id: albumArtBackground
                readonly property Image source: albumArtImage

                // Manual implementation of Image.PreserveAspectCrop
                readonly property real scaleFactor: Math.max(hoverHandler.width / source.paintedWidth, hoverHandler.height / source.paintedHeight)
                width: Math.round(source.paintedWidth * scaleFactor)
                height: Math.round(source.paintedHeight * scaleFactor)
                layer.enabled: true
                opacity: 0.25
                layer.effect: FastBlur {
                    source: albumArtBackground
                    anchors.fill: source
                    radius: 30
                }
            }
        }

        Image {
            id: albumArtImage
            // also Image.Loading to prevent loading thumbnails just because the album art takes a split second to load
            // if this is a group tooltip, we check if window title and track match, to allow distinguishing the different windows
            // if this app is a browser, we also check the title, so album art is not shown when the user is on some other tab
            // in all other cases we can safely show the album art without checking the title
            readonly property bool available: (status === Image.Ready || status === Image.Loading)
                && (!(isGroup || backend.applicationCategories(launcherUrl).includes("WebBrowser")) || titleIncludesTrack)

            anchors.fill: hoverHandler
            // Indent by one pixel to make sure we never cover up the entire highlight
            anchors.margins: 1
            sourceSize: Qt.size(parent.width, parent.height)

            asynchronous: true
            source: playerController.item ? playerController.item.albumArt : ""
            fillMode: Image.PreserveAspectFit
            visible: available
        }

        // hoverHandler has to be unloaded after the instance is pooled in order to avoid getting the old containsMouse status when the same instance is reused, so put it in a Loader.
        Loader {
            id: hoverHandler
            active: flatIndex !== -1
            anchors.fill: parent
            sourceComponent: ToolTipWindowMouseArea {
                rootTask: parentTask
                modelIndex: submodelIndex
                winId: thumbnailSourceItem.winId
            }
        }
    }

    // Player controls row, load on demand so group tooltips could be loaded faster
    Loader {
        id: playerController
        active: hasPlayer && flatIndex !== -1 // Avoid loading when the instance is going to be destroyed
        asynchronous: true
        visible: active
        Layout.fillWidth: true
        Layout.maximumWidth: header.Layout.maximumWidth
        Layout.leftMargin: header.Layout.margins
        Layout.rightMargin: header.Layout.margins

        source: "PlayerController.qml"
    }

    // Volume controls
    Loader {
        active: parentTask
             && pulseAudio.item
             && parentTask.audioIndicatorsEnabled
             && parentTask.hasAudioStream
             && flatIndex !== -1 // Avoid loading when the instance is going to be destroyed
        asynchronous: true
        visible: active
        Layout.fillWidth: true
        Layout.maximumWidth: header.Layout.maximumWidth
        Layout.leftMargin: header.Layout.margins
        Layout.rightMargin: header.Layout.margins
        sourceComponent: RowLayout {
            PlasmaComponents3.ToolButton { // Mute button
                icon.width: PlasmaCore.Units.iconSizes.small
                icon.height: PlasmaCore.Units.iconSizes.small
                icon.name: if (checked) {
                    "audio-volume-muted"
                } else if (slider.displayValue <= 25) {
                    "audio-volume-low"
                } else if (slider.displayValue <= 75) {
                    "audio-volume-medium"
                } else {
                    "audio-volume-high"
                }
                onClicked: parentTask.toggleMuted()
                checked: parentTask.muted

                PlasmaComponents3.ToolTip {
                    text: parent.checked ?
                        i18nc("button to unmute app", "Unmute %1", parentTask.appName)
                        : i18nc("button to mute app", "Mute %1", parentTask.appName)
                }
            }

            PlasmaComponents3.Slider {
                id: slider

                readonly property int displayValue: Math.round(value / to * 100)
                readonly property int loudestVolume: {
                    let v = 0
                    parentTask.audioStreams.forEach((stream) => {
                        v = Math.max(v, stream.volume)
                    })
                    return v
                }

                Layout.fillWidth: true
                from: pulseAudio.item.minimalVolume
                to: pulseAudio.item.normalVolume
                value: loudestVolume
                stepSize: to / 100
                opacity: parentTask.muted ? 0.5 : 1

                Accessible.name: i18nc("Accessibility data on volume slider", "Adjust volume for %1", parentTask.appName)

                onMoved: parentTask.audioStreams.forEach((stream) => {
                    let v = Math.max(from, value)
                    if (v > 0 && loudestVolume > 0) { // prevent divide by 0
                        // adjust volume relative to the loudest stream
                        v = Math.min(Math.round(stream.volume / loudestVolume * v), to)
                    }
                    stream.model.Volume = v
                    stream.model.Muted = v === 0
                })
            }
            PlasmaComponents3.Label { // percent label
                Layout.alignment: Qt.AlignHCenter
                Layout.minimumWidth: percentMetrics.advanceWidth
                horizontalAlignment: Qt.AlignRight
                text: i18nc("volume percentage", "%1%", slider.displayValue)
                TextMetrics {
                    id: percentMetrics
                    text: i18nc("only used for sizing, should be widest possible string", "100%")
                }
            }
        }
    }

    function generateSubText() {
        if (activitiesParent === undefined) {
            return "";
        }

        let subTextEntries = [];

        const onAllDesktops = (isGroup ? IsOnAllVirtualDesktops : isOnAllVirtualDesktopsParent) === true;
        if (!plasmoid.configuration.showOnlyCurrentDesktop && virtualDesktopInfo.numberOfDesktops > 1) {
            const virtualDesktops = isGroup ? VirtualDesktops : virtualDesktopParent;

            if (!onAllDesktops && virtualDesktops !== undefined && virtualDesktops.length > 0) {
                let virtualDesktopNameList = new Array();

                for (let i = 0; i < virtualDesktops.length; ++i) {
                    virtualDesktopNameList.push(virtualDesktopInfo.desktopNames[virtualDesktopInfo.desktopIds.indexOf(virtualDesktops[i])]);
                }

                subTextEntries.push(i18nc("Comma-separated list of desktops", "On %1",
                    virtualDesktopNameList.join(", ")));
            } else if (onAllDesktops) {
                subTextEntries.push(i18nc("Comma-separated list of desktops", "Pinned to all desktops"));
            }
        }

        const act = isGroup ? Activities : activitiesParent;
        if (act === undefined) {
            return subTextEntries.join("\n");
        }

        if (act.length === 0 && activityInfo.numberOfRunningActivities > 1) {
            subTextEntries.push(i18nc("Which virtual desktop a window is currently on",
                "Available on all activities"));
        } else if (act.length > 0) {
            let activityNames = [];

            for (let i = 0; i < act.length; i++) {
                const activity = act[i];
                const activityName = activityInfo.activityName(act[i]);
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
