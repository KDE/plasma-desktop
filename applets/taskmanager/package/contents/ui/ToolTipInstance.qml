/*
    SPDX-FileCopyrightText: 2013 Sebastian Kügler <sebas@kde.org>
    SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>
    SPDX-FileCopyrightText: 2016 Kai Uwe Broulik <kde@privat.broulik.de>
    SPDX-FileCopyrightText: 2017 Roman Gilg <subdiff@gmail.com>
    SPDX-FileCopyrightText: 2020 Nate Graham <nate@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import Qt5Compat.GraphicalEffects as GE

import org.kde.plasma.plasmoid
import org.kde.plasma.core as PlasmaCore
import org.kde.plasma.components as PlasmaComponents3
import org.kde.plasma.extras as PlasmaExtras
import org.kde.kirigami as Kirigami
import org.kde.kwindowsystem

ColumnLayout {
    id: root

    required property int index
    required property /*QModelIndex*/ var submodelIndex
    required property int appPid
    required property string display
    required property bool isMinimized
    required property bool isOnAllVirtualDesktops
    required property /*list<var>*/ var virtualDesktops // Can't use list<var> because of QTBUG-127600
    required property list<string> activities

    // HACK: Avoid blank space in the tooltip after closing a window
    ListView.onPooled: width = height = 0
    ListView.onReused: width = height = undefined

    readonly property string title: {
        if (!toolTipDelegate.isWin) {
            return toolTipDelegate.genericName;
        }

        let text = display;
        if (toolTipDelegate.isGroup && text === "") {
            return "";
        }

        // Normally the window title will always have " — [app name]" at the end of
        // the window-provided title. But if it doesn't, this is intentional 100%
        // of the time because the developer or user has deliberately removed that
        // part, so just display it with no more fancy processing.
        if (!text.match(/\s+(—|-|–)/)) {
            return text;
        }

        // KWin appends increasing integers in between pointy brackets to otherwise equal window titles.
        // In this case save <#number> as counter and delete it at the end of text.
        text = `${(text.match(/.*(?=\s+(—|-|–))/) || [""])[0]}${(text.match(/<\d+>/) || [""]).pop()}`;

        // In case the window title had only redundant information (i.e. appName), text is now empty.
        // Add a hyphen to indicate that and avoid empty space.
        if (text === "") {
            text = "—";
        }
        return text;
    }

    readonly property bool titleIncludesTrack: toolTipDelegate.playerData !== null && title.includes(toolTipDelegate.playerData.track)

    spacing: Kirigami.Units.smallSpacing

    // text labels + close button
    RowLayout {
        id: header
        // match spacing of DefaultToolTip.qml in plasma-framework
        spacing: toolTipDelegate.isWin ? Kirigami.Units.smallSpacing : Kirigami.Units.gridUnit

        // This number controls the overall size of the window tooltips
        Layout.maximumWidth: toolTipDelegate.tooltipInstanceMaximumWidth
        Layout.minimumWidth: toolTipDelegate.isWin ? Layout.maximumWidth : 0
        Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
        // match margins of DefaultToolTip.qml in plasma-framework
        Layout.margins: toolTipDelegate.isWin ? 0 : Kirigami.Units.gridUnit / 2

        // all textlabels
        ColumnLayout {
            spacing: 0
            // app name
            Kirigami.Heading {
                id: appNameHeading
                level: 3
                maximumLineCount: 1
                lineHeight: toolTipDelegate.isWin ? 1 : appNameHeading.lineHeight
                Layout.fillWidth: true
                elide: Text.ElideRight
                text: toolTipDelegate.appName
                opacity: root.index === 0 ? 1 : 0
                visible: text.length !== 0
                textFormat: Text.PlainText
            }
            // window title
            PlasmaComponents3.Label {
                id: winTitle
                maximumLineCount: 1
                Layout.fillWidth: true
                elide: Text.ElideRight
                text: root.titleIncludesTrack ? "" : root.title
                opacity: 0.75
                visible: root.title.length !== 0 && root.title !== appNameHeading.text
                textFormat: Text.PlainText
            }
            // subtext
            PlasmaComponents3.Label {
                id: subtext
                maximumLineCount: 2
                Layout.fillWidth: true
                elide: Text.ElideRight
                text: toolTipDelegate.isWin ? root.generateSubText() : ""
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
            visible: root.index === 0 && toolTipDelegate.smartLauncherCountVisible

            Badge {
                anchors.centerIn: parent
                height: Kirigami.Units.iconSizes.smallMedium
                number: toolTipDelegate.smartLauncherCount
            }
        }

        // close button
        PlasmaComponents3.ToolButton {
            id: closeButton
            Layout.alignment: Qt.AlignRight | Qt.AlignTop
            visible: toolTipDelegate.isWin
            icon.name: "window-close"
            onClicked: {
                backend.cancelHighlightWindows();
                tasksModel.requestClose(root.submodelIndex);
            }
        }
    }

    // thumbnail container
    Item {
        id: thumbnailSourceItem

        Layout.fillWidth: true
        Layout.preferredHeight: Kirigami.Units.gridUnit * 8

        clip: true
        visible: toolTipDelegate.isWin

        readonly property /*undefined|WId where WId = int|string*/ var winId:
            toolTipDelegate.isWin ? toolTipDelegate.windows[root.index] : undefined

        // There's no PlasmaComponents3 version
        PlasmaExtras.Highlight {
            anchors.fill: hoverHandler
            visible: (hoverHandler.item as MouseArea)?.containsMouse ?? false
            pressed: (hoverHandler.item as MouseArea)?.containsPress ?? false
            hovered: true
        }

        Loader {
            id: thumbnailLoader
            active: !toolTipDelegate.isLauncher
                && !albumArtImage.visible
                && (Number.isInteger(thumbnailSourceItem.winId) || pipeWireLoader.item && !pipeWireLoader.item.hasThumbnail)
                && root.index !== -1 // Avoid loading when the instance is going to be destroyed
            asynchronous: true
            visible: active
            anchors.fill: hoverHandler
            // Indent a little bit so that neither the thumbnail nor the drop
            // shadow can cover up the highlight
            anchors.margins: Kirigami.Units.smallSpacing * 2

            sourceComponent: root.isMinimized || pipeWireLoader.active ? iconItem : x11Thumbnail

            Component {
                id: x11Thumbnail

                PlasmaCore.WindowThumbnail {
                    winId: thumbnailSourceItem.winId
                }
            }

            // when minimized, we don't have a preview on X11, so show the icon
            Component {
                id: iconItem

                Kirigami.Icon {
                    id: realIconItem
                    source: toolTipDelegate.icon
                    animated: false
                    visible: valid
                    opacity: pipeWireLoader.active ? 0 : 1

                    SequentialAnimation {
                        running: true

                        PauseAnimation {
                            duration: Kirigami.Units.humanMoment
                        }

                        NumberAnimation {
                            id: showAnimation
                            duration: Kirigami.Units.longDuration
                            easing.type: Easing.OutCubic
                            property: "opacity"
                            target: realIconItem
                            to: 1
                        }
                    }

                }
            }
        }

        Loader {
            id: pipeWireLoader
            anchors.fill: hoverHandler
            // Indent a little bit so that neither the thumbnail nor the drop
            // shadow can cover up the highlight
            anchors.margins: thumbnailLoader.anchors.margins

            active: !toolTipDelegate.isLauncher && !albumArtImage.visible && KWindowSystem.isPlatformWayland && root.index !== -1
            asynchronous: true
            //In a loader since we might not have PipeWire available yet (WITH_PIPEWIRE could be undefined in plasma-workspace/libtaskmanager/declarative/taskmanagerplugin.cpp)
            source: "PipeWireThumbnail.qml"
        }

        Loader {
            active: (pipeWireLoader.item?.hasThumbnail ?? false) || (thumbnailLoader.status === Loader.Ready && !root.isMinimized)
            asynchronous: true
            visible: active
            anchors.fill: pipeWireLoader.active ? pipeWireLoader : thumbnailLoader

            sourceComponent: GE.DropShadow {
                horizontalOffset: 0
                verticalOffset: 3
                radius: 8
                samples: Math.round(radius * 1.5)
                color: "Black"
                source: pipeWireLoader.active ? pipeWireLoader.item : thumbnailLoader.item // source could be undefined when albumArt is available, so put it in a Loader.
            }
        }

        Loader {
            active: albumArtImage.visible && albumArtImage.status === Image.Ready && root.index !== -1 // Avoid loading when the instance is going to be destroyed
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
                layer.effect: GE.FastBlur {
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
                && (!(toolTipDelegate.isGroup || backend.applicationCategories(launcherUrl).includes("WebBrowser")) || root.titleIncludesTrack)

            anchors.fill: hoverHandler
            // Indent by one pixel to make sure we never cover up the entire highlight
            anchors.margins: 1
            sourceSize: Qt.size(parent.width, parent.height)

            asynchronous: true
            source: toolTipDelegate.playerData?.artUrl ?? ""
            fillMode: Image.PreserveAspectFit
            visible: available
        }

        // hoverHandler has to be unloaded after the instance is pooled in order to avoid getting the old containsMouse status when the same instance is reused, so put it in a Loader.
        Loader {
            id: hoverHandler
            active: root.index !== -1
            anchors.fill: parent
            sourceComponent: ToolTipWindowMouseArea {
                rootTask: toolTipDelegate.parentTask
                modelIndex: root.submodelIndex
                winId: thumbnailSourceItem.winId
            }
        }
    }

    // Player controls row, load on demand so group tooltips could be loaded faster
    Loader {
        id: playerController
        active: toolTipDelegate.playerData && root.index !== -1 // Avoid loading when the instance is going to be destroyed
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
        active: toolTipDelegate.parentTask !== null
             && pulseAudio.item !== null
             && toolTipDelegate.parentTask.audioIndicatorsEnabled
             && toolTipDelegate.parentTask.hasAudioStream
             && root.index !== -1 // Avoid loading when the instance is going to be destroyed
        asynchronous: true
        visible: active
        Layout.fillWidth: true
        Layout.maximumWidth: header.Layout.maximumWidth
        Layout.leftMargin: header.Layout.margins
        Layout.rightMargin: header.Layout.margins
        sourceComponent: RowLayout {
            PlasmaComponents3.ToolButton { // Mute button
                icon.width: Kirigami.Units.iconSizes.small
                icon.height: Kirigami.Units.iconSizes.small
                icon.name: if (checked) {
                    "audio-volume-muted"
                } else if (slider.displayValue <= 25) {
                    "audio-volume-low"
                } else if (slider.displayValue <= 75) {
                    "audio-volume-medium"
                } else {
                    "audio-volume-high"
                }
                onClicked: toolTipDelegate.parentTask.toggleMuted()
                checked: toolTipDelegate.parentTask.muted

                PlasmaComponents3.ToolTip {
                    text: parent.checked
                        ? i18nc("button to unmute app", "Unmute %1", toolTipDelegate.parentTask.appName)
                        : i18nc("button to mute app", "Mute %1", toolTipDelegate.parentTask.appName)
                }
            }

            PlasmaComponents3.Slider {
                id: slider

                readonly property int displayValue: Math.round(value / to * 100)
                readonly property int loudestVolume: toolTipDelegate.parentTask.audioStreams
                    .reduce((loudestVolume, stream) => Math.max(loudestVolume, stream.volume), 0)

                Layout.fillWidth: true
                from: pulseAudio.item.minimalVolume
                to: pulseAudio.item.normalVolume
                value: loudestVolume
                stepSize: to / 100
                opacity: toolTipDelegate.parentTask.muted ? 0.5 : 1

                Accessible.name: i18nc("Accessibility data on volume slider", "Adjust volume for %1", toolTipDelegate.parentTask.appName)

                onMoved: toolTipDelegate.parentTask.audioStreams.forEach((stream) => {
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
                textFormat: Text.PlainText
                TextMetrics {
                    id: percentMetrics
                    text: i18nc("only used for sizing, should be widest possible string", "100%")
                }
            }
        }
    }

    function generateSubText(): string {
        const subTextEntries = [];

        if (!Plasmoid.configuration.showOnlyCurrentDesktop && virtualDesktopInfo.numberOfDesktops > 1) {
            if (!isOnAllVirtualDesktops && virtualDesktops.length > 0) {
                const virtualDesktopNameList = virtualDesktops.map(virtualDesktop => {
                    const index = virtualDesktopInfo.desktopIds.indexOf(virtualDesktop);
                    return virtualDesktopInfo.desktopNames[index];
                });

                subTextEntries.push(i18nc("Comma-separated list of desktops", "On %1",
                    virtualDesktopNameList.join(", ")));
            } else if (isOnAllVirtualDesktops) {
                subTextEntries.push(i18nc("Comma-separated list of desktops", "Pinned to all desktops"));
            }
        }

        if (activities.length === 0 && activityInfo.numberOfRunningActivities > 1) {
            subTextEntries.push(i18nc("Which virtual desktop a window is currently on",
                "Available on all activities"));
        } else if (activities.length > 0) {
            const activityNames = activities
                .filter(activity => activity !== activityInfo.currentActivity)
                .map(activity => activityInfo.activityName(activity))
                .filter(activityName => activityName !== "");

            if (Plasmoid.configuration.showOnlyCurrentActivity) {
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
