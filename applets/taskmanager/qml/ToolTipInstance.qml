/*
    SPDX-FileCopyrightText: 2013 Sebastian Kügler <sebas@kde.org>
    SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>
    SPDX-FileCopyrightText: 2016 Kai Uwe Broulik <kde@privat.broulik.de>
    SPDX-FileCopyrightText: 2017 Roman Gilg <subdiff@gmail.com>
    SPDX-FileCopyrightText: 2020-2024 Nate Graham <nate@kde.org>

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

    required property var model
    required property int index
    required property /*QModelIndex*/ var submodelIndex
    required property int appPid
    required property string display
    required property bool isMinimized
    required property bool isOnAllVirtualDesktops
    required property /*list<var>*/ var virtualDesktops // Can't use list<var> because of QTBUG-127600
    required property list<string> activities
    required property bool isReadyForPainting
    required property bool isExcludedFromCapture

    property bool hasTrackInATitle: false
    property int orientation: ListView.Vertical // vertical for compact single-window tooltips

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

    // Lots of spacing with no thumbnails looks bad
    spacing: Plasmoid.configuration.showToolTips ? toolTipDelegate.contentMargin + toolTipDelegate.backgroundTopMargin : 0

    // Every row is inset from the tooltip's edges by toolTipDelegate.contentMargin.
    // Rows are stacked with spacing in between, so only the top one needs a top
    // margin, and only the bottom-most visible one a bottom margin; hidden rows are
    // skipped by the layout, margins and all.
    readonly property Item lastVisibleRow: {
        if (volumeControls.visible) {
            return volumeControls;
        } else if (playerController.visible) {
            return playerController;
        } else if (thumbnailSourceItem.visible) {
            return thumbnailSourceItem;
        } else {
            return headerItem;
        }
    }

    // text labels + close button
    Item {
        id: headerItem
        implicitHeight: header.height
        implicitWidth: header.implicitWidth
        Layout.fillWidth: true

        // This number controls the overall size of the window tooltips
        Layout.maximumWidth: toolTipDelegate.tooltipInstanceMaximumWidth
        Layout.minimumWidth: (toolTipDelegate.isWin && Plasmoid.configuration.showToolTips) || toolTipDelegate.isGroup ? Layout.maximumWidth : 0
        Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
        Layout.margins: toolTipDelegate.contentMargin
        // Without thumbnails the header is the whole row, so it always keeps its bottom margin
        Layout.bottomMargin: Plasmoid.configuration.showToolTips && root.lastVisibleRow !== headerItem ? 0 : toolTipDelegate.contentMargin

        // Without thumbnails the close button sits in the row's corner, to keep the rows compact
        readonly property int closeButtonOutset: Plasmoid.configuration.showToolTips ? 0 : toolTipDelegate.contentMargin
        // With thumbnails, pull the close button and the badges up by the button's padding, so the
        // icon rather than its hover background lines up with the top of the text
        readonly property real closeButtonPaddingOffset: Plasmoid.configuration.showToolTips ? closeButton.topPadding : 0

        RowLayout {
            id: header
            width: parent.width
            // match spacing of DefaultToolTip.qml in plasma-framework
            spacing: Kirigami.Units.smallSpacing

            // close button for right edge ltr and left edge rtl
            LayoutItemProxy {
                id: closeButtonFlippedItemProxy
                target: closeButton
                visible: toolTipDelegate.isWin &&
                    ((Plasmoid.location == PlasmaCore.Types.LeftEdge &&
                      Application.layoutDirection == Qt.RightToLeft) ||
                     (Plasmoid.location == PlasmaCore.Types.RightEdge &&
                      Application.layoutDirection == Qt.LeftToRight))
            }

            // all textlabels
            ColumnLayout {
                spacing: 0
                // app name
                Kirigami.Heading {
                    id: appNameHeading
                    level: 3
                    maximumLineCount: 1
                    Layout.fillWidth: true
                    lineHeight: toolTipDelegate.isWin && Plasmoid.configuration.showToolTips ? 1 : appNameHeading.lineHeight
                    elide: Text.ElideRight
                    text: toolTipDelegate.appName
                    color: (headerHoverHandler.enabled && headerHoverHighlight.pressed) ? Kirigami.Theme.highlightedTextColor : Kirigami.Theme.textColor
                    opacity: root.index === 0 ? 1 : 0
                    visible: (text.length !== 0) && (root.orientation === ListView.Horizontal || root.index === 0)
                    textFormat: Text.PlainText
                }
                // window title
                RowLayout {

                }
                PlasmaComponents3.Label {
                    id: winTitle
                    Layout.fillWidth: true
                    // For horizontal grouped tasks, leave room for two lines so thumbnails align
                    Layout.preferredHeight: root.orientation === ListView.Horizontal && lineCount === 1
                        ? implicitHeight * 2
                        : implicitHeight
                    maximumLineCount: 2
                    wrapMode: Text.Wrap
                    elide: Text.ElideRight
                    verticalAlignment: Text.AlignVCenter
                    property bool somethingVisible: (thumbnailSourceItem.visible ||
                        appNameHeading.visible || subtext.visible)
                    text: ((root.titleIncludesTrack && playerController.active) ||
                           (root.title === appNameHeading.text && somethingVisible))
                          ? "" : root.title
                    color: (headerHoverHandler.enabled && headerHoverHighlight.pressed) ? Kirigami.Theme.highlightedTextColor : Kirigami.Theme.textColor
                    opacity: 0.75
                    font.bold: toolTipDelegate.isGroup && toolTipDelegate.parentTask.model.IsActive && root.index == tasksModel.activeTask.row
                    visible: root.orientation === ListView.Horizontal || text.length !== 0
                    textFormat: Text.PlainText
                }
                // subtext
                PlasmaComponents3.Label {
                    id: subtext
                    Layout.fillWidth: true
                    // For horizontal grouped tasks, leave room for two lines so thumbnails align
                    Layout.preferredHeight: root.orientation === ListView.Horizontal && lineCount === 1
                        ? implicitHeight * 2
                        : implicitHeight
                    maximumLineCount: 2
                    wrapMode: Text.Wrap
                    elide: Text.ElideRight
                    verticalAlignment: Text.AlignVCenter
                    text: toolTipDelegate.isWin ? root.generateSubText() : ""
                    color: (headerHoverHandler.enabled && headerHoverHighlight.pressed) ? Kirigami.Theme.highlightedTextColor : Kirigami.Theme.textColor
                    opacity: 0.75
                    visible: text.length !== 0 && text !== appNameHeading.text
                    textFormat: Text.PlainText
                }
            }

            // Requests Attention badge.
            // The badge itself is inside an item to better center the text in the bubble
            Item {
                Layout.alignment: !Plasmoid.configuration.showToolTips && !playerController.active && !volumeControls.active ? Qt.AlignVCenter : Qt.AlignTop
                Layout.topMargin: -headerItem.closeButtonPaddingOffset
                Layout.preferredHeight: closeButton.height
                Layout.preferredWidth: closeButton.width
                visible: root.model?.IsDemandingAttention ?? false

                Kirigami.Badge {
                    anchors.centerIn: parent
                    icon.name: "notification-active-symbolic"
                    type: Kirigami.Badge.Warning
                }
            }

            // Count badge.
            // The badge itself is inside an item to better center the text in the bubble
            Item {
                Layout.alignment: !Plasmoid.configuration.showToolTips && !playerController.active && !volumeControls.active ? Qt.AlignVCenter : Qt.AlignTop
                Layout.topMargin: -headerItem.closeButtonPaddingOffset
                Layout.preferredHeight: closeButton.height
                Layout.preferredWidth: closeButton.width
                visible: root.index === 0 && toolTipDelegate.smartLauncherCountVisible

                Kirigami.Badge {
                    anchors.centerIn: parent
                    text: toolTipDelegate.smartLauncherCount
                }
            }

            LayoutItemProxy {
                target: closeButton
                visible: toolTipDelegate.isWin && !closeButtonFlippedItemProxy.visible
            }

            // close button
            PlasmaComponents3.ToolButton {
                id: closeButton
                Layout.alignment: Qt.AlignTop
                Layout.topMargin: -headerItem.closeButtonOutset - headerItem.closeButtonPaddingOffset
                Layout.leftMargin: closeButtonFlippedItemProxy.visible ? -headerItem.closeButtonOutset : headerItem.closeButtonOutset
                Layout.rightMargin: closeButtonFlippedItemProxy.visible ? headerItem.closeButtonOutset : -headerItem.closeButtonOutset
                icon.name: "window-close"
                onClicked: {
                    tasks.cancelHighlightWindows();
                    tasksModel.requestClose(root.submodelIndex);
                }
                PlasmaComponents3.ToolTip.text: i18nc("@info:tooltip Close this window", "Close window")
                PlasmaComponents3.ToolTip.visible: root.visible && hovered
                PlasmaComponents3.ToolTip.delay: Kirigami.Units.toolTipDelay
            }
        }

        component ToolTipHoverHandler: HoverHandler {
            onHoveredChanged: {
                tasks.windowsHovered([String(thumbnailSourceItem.winId)], hovered);
            }
        }

        component ToolTipTapHandler: TapHandler {
            acceptedButtons: Qt.LeftButton | Qt.RightButton | Qt.MiddleButton
            onTapped: (event, button) => {
                switch (button) {
                    case Qt.LeftButton:
                        tasksModel.requestActivate(root.submodelIndex);
                        parentTask.hideImmediately();
                        tasks.cancelHighlightWindows();
                        break;
                    case Qt.MiddleButton:
                        tasks.cancelHighlightWindows();
                        tasksModel.requestClose(root.submodelIndex);
                        break;
                    case Qt.RightButton:
                        tasks.createContextMenu(parentTask, modelIndex).show();
                        break;
                }
            }
        }

        ToolTipHoverHandler {
            id: headerHoverHandler
            enabled: (root.index !== -1) && !Plasmoid.configuration.showToolTips && thumbnailSourceItem.winId != undefined
            margin: headerItem.Layout.margins
        }

        ToolTipTapHandler {
            id: headerTapHandler
            enabled: (root.index !== -1) && !Plasmoid.configuration.showToolTips && thumbnailSourceItem.winId != undefined
            margin: headerItem.Layout.margins
        }

        // There's no PlasmaComponents3 version
        PlasmaExtras.Highlight {
            id: headerHoverHighlight
            anchors.fill: headerItem
            anchors.margins: -toolTipDelegate.contentMargin
            z: -1
            visible: headerHoverHandler.hovered
            pressed: headerTapHandler.pressed
            hovered: true
        }
    }

    // thumbnail container
    Item {
        id: thumbnailSourceItem

        Layout.fillWidth: true
        Layout.preferredHeight: Kirigami.Units.gridUnit * 8
        Layout.leftMargin: toolTipDelegate.contentMargin
        Layout.rightMargin: toolTipDelegate.contentMargin
        Layout.bottomMargin: root.lastVisibleRow === thumbnailSourceItem ? toolTipDelegate.contentMargin : 0

        clip: true
        visible: Plasmoid.configuration.showToolTips && toolTipDelegate.isWin

        // Round off the corners of the preview area and everything painted inside
        // it: the thumbnail, the album art and the hover highlight
        layer.enabled: true
        layer.effect: GE.OpacityMask {
            maskSource: Rectangle {
                width: thumbnailSourceItem.width
                height: thumbnailSourceItem.height
                radius: Kirigami.Units.cornerRadius
            }
        }

        readonly property /*undefined|WId where WId = int|string*/ var winId:
            toolTipDelegate.isWin ? toolTipDelegate.windows[root.index] : undefined

        // The thumbnail is painted inside this inset box to give its drop shadow
        // room; flush against the rounded, clipped edge of the row it would be
        // masked away entirely
        Item {
            id: thumbnailContent
            anchors.fill: parent
            anchors.margins: toolTipDelegate.contentMargin
        }

        // There's no PlasmaComponents3 version
        PlasmaExtras.Highlight {
            anchors.fill: parent
            visible: hoverHandler.hovered
            pressed: tapHandler.pressed
            hovered: true
        }

        Loader {
            id: pipeWireLoader
            anchors.fill: thumbnailContent

            active: Plasmoid.configuration.showToolTips
                && !toolTipDelegate.isLauncher
                && !albumArtImage.visible
                && (toolTipDelegate.isReadyForPainting || root.isExcludedFromCapture)
                && root.index !== -1
            asynchronous: true
            //In a loader since we might not have PipeWire available yet (WITH_PIPEWIRE could be undefined in plasma-workspace/libtaskmanager/declarative/taskmanagerplugin.cpp)
            // Windows excluded from capture must not leak their contents, so show the icon instead
            source: root.isExcludedFromCapture ? "IconThumbnail.qml" : "PipeWireThumbnail.qml"
        }

        Loader {
            active: Plasmoid.configuration.showToolTips
                && ((pipeWireLoader.item as PipeWireThumbnail)?.hasThumbnail ?? false)
            asynchronous: true
            visible: active
            anchors.fill: pipeWireLoader

            sourceComponent: GE.DropShadow {
                horizontalOffset: 0
                verticalOffset: 3
                radius: 8
                samples: Math.round(radius * 1.5)
                color: "Black"
                source: pipeWireLoader.item // source could be undefined when albumArt is available, so put it in a Loader.
            }
        }

        Loader {
            active: Plasmoid.configuration.showToolTips
                && albumArtImage.visible
                && albumArtImage.status === Image.Ready
                && root.index !== -1 // Avoid loading when the instance is going to be destroyed
            asynchronous: true
            visible: active
            anchors.centerIn: parent

            sourceComponent: ShaderEffect {
                id: albumArtBackground
                readonly property Image source: albumArtImage

                // Manual implementation of Image.PreserveAspectCrop
                readonly property real scaleFactor: Math.max(thumbnailSourceItem.width / source.paintedWidth, thumbnailSourceItem.height / source.paintedHeight)
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

            anchors.fill: thumbnailContent
            sourceSize: Qt.size(thumbnailContent.width, thumbnailContent.height)

            asynchronous: true
            retainWhileLoading: true
            source: toolTipDelegate.playerData?.artUrl ?? ""
            fillMode: Image.PreserveAspectFit
            visible: available
        }

        ToolTipHoverHandler {
            id: hoverHandler
            enabled: root.index !== -1 && Plasmoid.configuration.showToolTips && thumbnailSourceItem.winId != undefined
        }

        ToolTipTapHandler {
            id: tapHandler
            enabled: root.index !== -1 && Plasmoid.configuration.showToolTips && thumbnailSourceItem.winId != undefined
        }
    }

    // Player controls row, load on demand so group tooltips could be loaded faster
    Loader {
        id: playerController
        // Only load for one entry, as the controls only apply to one window.
        // If this is changed in the future, test for index != -1 to avoid loading
        // when the instance is going to be destroyed
        active: (toolTipDelegate.parentTask?.tooltipControlsEnabled
             && toolTipDelegate.playerData
             && ((root.hasTrackInATitle && albumArtImage.available) || (!root.hasTrackInATitle && root.index == 0))) ?? false

        asynchronous: true
        visible: active
        Layout.fillWidth: true
        Layout.maximumWidth: headerItem.Layout.maximumWidth
        Layout.leftMargin: toolTipDelegate.contentMargin
        Layout.rightMargin: toolTipDelegate.contentMargin
        Layout.bottomMargin: root.lastVisibleRow === playerController ? toolTipDelegate.contentMargin : 0

        source: "PlayerController.qml"
    }

    // Volume controls
    Loader {
        id: volumeControls
        active: toolTipDelegate.parentTask !== null
             && pulseAudio.item !== null
             && toolTipDelegate.parentTask.tooltipControlsEnabled
             && toolTipDelegate.parentTask.hasAudioStream
             // Only load for one entry, as the controls only apply to one window.
             // If this is changed in the future, test for index != -1 to avoid loading
             // when the instance is going to be destroyed
             && ((hasTrackInATitle && albumArtImage.available) || (!hasTrackInATitle && root.index == 0))
        asynchronous: true
        visible: active
        Layout.fillWidth: true
        Layout.maximumWidth: headerItem.Layout.maximumWidth
        Layout.leftMargin: toolTipDelegate.contentMargin
        Layout.rightMargin: toolTipDelegate.contentMargin
        Layout.bottomMargin: toolTipDelegate.contentMargin
        sourceComponent: RowLayout {
            PlasmaComponents3.ToolButton { // Mute button
                id: muteButton
                icon.width: Kirigami.Units.iconSizes.small
                icon.height: Kirigami.Units.iconSizes.small
                icon.name: {
                    let finalIcon = ""

                    if (checked) {
                        finalIcon = "audio-volume-muted"
                    } else if (slider.displayValue <= 25) {
                        finalIcon = "audio-volume-low"
                    } else if (slider.displayValue <= 75) {
                        finalIcon = "audio-volume-medium"
                    } else {
                        finalIcon = "audio-volume-high"
                    }

                    if (mirrored) {
                        finalIcon = finalIcon + "-rtl"
                    }

                    return finalIcon
                }
                onClicked: toolTipDelegate.parentTask.toggleMuted()
                checked: toolTipDelegate.parentTask.muted

                PlasmaComponents3.ToolTip {
                    text: muteButton.checked
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
