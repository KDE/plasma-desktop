/*
    SPDX-FileCopyrightText: 2011 Martin Gräßlin <mgraesslin@kde.org>
    SPDX-FileCopyrightText: 2012 Gregor Taetzner <gregor@freenet.de>
    SPDX-FileCopyrightText: 2012 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2013 David Edmundson <davidedmundson@kde.org>
    SPDX-FileCopyrightText: 2015 Eike Hein <hein@kde.org>
    SPDX-FileCopyrightText: 2021 Mikel Johnson <mikel5764@gmail.com>
    SPDX-FileCopyrightText: 2021 Noah Davis <noahadvs@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Window 2.15
import QtQml 2.15
import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core as PlasmaCore
import org.kde.ksvg 1.0 as KSvg
import org.kde.plasma.components 3.0 as PC3
import org.kde.plasma.private.kicker 0.1 as Kicker
import org.kde.kirigami 2.20 as Kirigami

import "code/tools.js" as Tools

PlasmoidItem {
    id: kickoff

    // The properties are defined here instead of the singleton because each
    // instance of Kickoff requires different instances of these properties

    readonly property bool inPanel: [
        PlasmaCore.Types.TopEdge,
        PlasmaCore.Types.RightEdge,
        PlasmaCore.Types.BottomEdge,
        PlasmaCore.Types.LeftEdge,
    ].includes(Plasmoid.location)
    readonly property bool vertical: Plasmoid.formFactor === PlasmaCore.Types.Vertical

    // Used to prevent the width from changing frequently when the scrollbar appears or disappears
    readonly property bool mayHaveGridWithScrollBar: Plasmoid.configuration.applicationsDisplay === 0
        || (Plasmoid.configuration.favoritesDisplay === 0 && kickoff.rootModel.favoritesModel.count > minimumGridRowCount * minimumGridRowCount)

    //BEGIN Models
    readonly property Kicker.RootModel rootModel: Kicker.RootModel {
        autoPopulate: false

        // TODO: appletInterface property now can be ported to "applet" and have the real Applet* assigned directly
        appletInterface: kickoff

        flat: true // have categories, but no subcategories
        sorted: Plasmoid.configuration.alphaSort
        showSeparators: true
        showTopLevelItems: true

        showAllApps: true
        showAllAppsCategorized: false
        showRecentApps: false
        showRecentDocs: false
        showPowerSession: false
        showFavoritesPlaceholder: true

        Component.onCompleted: {
            favoritesModel.initForClient("org.kde.plasma.kickoff.favorites.instance-" + Plasmoid.id)

            if (!Plasmoid.configuration.favoritesPortedToKAstats) {
                if (favoritesModel.count < 1) {
                    favoritesModel.portOldFavorites(Plasmoid.configuration.favorites);
                }
                Plasmoid.configuration.favoritesPortedToKAstats = true;
            }
        }
    }

    readonly property Kicker.RunnerModel runnerModel: Kicker.RunnerModel {
        query: kickoff.searchField ? kickoff.searchField.text : ""
        onRequestUpdateQuery: query => {
            if (kickoff.searchField) {
                kickoff.searchField.text = query;
            }
        }
        appletInterface: kickoff
        mergeResults: true
        favoritesModel: rootModel.favoritesModel
    }

    readonly property Kicker.ComputerModel computerModel: Kicker.ComputerModel {
        appletInterface: kickoff
        favoritesModel: rootModel.favoritesModel
        systemApplications: Plasmoid.configuration.systemApplications
        Component.onCompleted: {
            //systemApplications = Plasmoid.configuration.systemApplications;
        }
    }

    readonly property Kicker.RecentUsageModel recentUsageModel: Kicker.RecentUsageModel {
        favoritesModel: rootModel.favoritesModel
    }

    readonly property Kicker.RecentUsageModel frequentUsageModel: Kicker.RecentUsageModel {
        favoritesModel: rootModel.favoritesModel
        ordering: 1 // Popular / Frequently Used
    }
    //END

    //BEGIN UI elements
    // Set in FullRepresentation.qml
    property Item header: null

    // Set in Header.qml
    property PC3.TextField searchField: null

    // Set in FullRepresentation.qml, ApplicationPage.qml, PlacesPage.qml
    property Item sideBar: null // is null when searching
    property Item contentArea: null // is searchView when searching

    // Set in NormalPage.qml
    property Item footer: null

    // True when central pane (and header) LayoutMirroring diverges from global
    // LayoutMirroring, in order to achieve the desired sidebar position
    readonly property bool paneSwap: Plasmoid.configuration.paneSwap
    readonly property bool sideBarOnRight: (Qt.application.layoutDirection == Qt.RightToLeft) != paneSwap
    // References to items according to their focus chain order
    readonly property Item firstHeaderItem: header ? (paneSwap ? header.pinButton : header.avatar) : null
    readonly property Item lastHeaderItem: header ? (paneSwap ? header.avatar : header.pinButton) : null
    readonly property Item firstCentralPane: paneSwap ? contentArea : sideBar
    readonly property Item lastCentralPane: paneSwap ? sideBar : contentArea
    //END

    //BEGIN Metrics
    readonly property KSvg.FrameSvgItem backgroundMetrics: KSvg.FrameSvgItem {
        // Inset defaults to a negative value when not set by margin hints
        readonly property real leftPadding: margins.left - Math.max(inset.left, 0)
        readonly property real rightPadding: margins.right - Math.max(inset.right, 0)
        readonly property real topPadding: margins.top - Math.max(inset.top, 0)
        readonly property real bottomPadding: margins.bottom - Math.max(inset.bottom, 0)
        readonly property real spacing: leftPadding
        visible: false
        imagePath: Plasmoid.formFactor === PlasmaCore.Types.Planar ? "widgets/background" : "dialogs/background"
    }

    // This is here rather than in the singleton with the other metrics items
    // because the list delegate's height depends on a configuration setting
    // and the singleton can't access those
    readonly property real listDelegateHeight: listDelegate.height
    KickoffListDelegate {
        id: listDelegate
        visible: false
        enabled: false
        model: null
        index: -1
        text: "asdf"
        url: ""
        decoration: "start-here-kde"
        description: "asdf"
        action: null
        indicator: null
    }

    // Used to show smaller Kickoff on small screens
    readonly property int minimumGridRowCount: Math.min(Screen.desktopAvailableWidth, Screen.desktopAvailableHeight) * Screen.devicePixelRatio < KickoffSingleton.gridCellSize * 4 + (fullRepresentationItem ? fullRepresentationItem.normalPage.preferredSideBarWidth : KickoffSingleton.gridCellSize * 2) ? 2 : 4
    //END

    Plasmoid.icon: Plasmoid.configuration.icon

    switchWidth: fullRepresentationItem ? fullRepresentationItem.Layout.minimumWidth : -1
    switchHeight: fullRepresentationItem ? fullRepresentationItem.Layout.minimumHeight : -1

    preferredRepresentation: compactRepresentation

    fullRepresentation: FullRepresentation { focus: true }

    // Only exists because the default CompactRepresentation doesn't:
    // - open on drag
    // - allow defining a custom drop handler
    // - expose the ability to show text below or beside the icon
    // TODO remove once it gains those features
    compactRepresentation: MouseArea {
        id: compactRoot

        // Taken from DigitalClock to ensure uniform sizing when next to each other
        readonly property bool tooSmall: Plasmoid.formFactor === PlasmaCore.Types.Horizontal && Math.round(2 * (compactRoot.height / 5)) <= Kirigami.Theme.smallFont.pixelSize

        readonly property bool shouldHaveIcon: Plasmoid.formFactor === PlasmaCore.Types.Vertical || Plasmoid.icon !== ""
        readonly property bool shouldHaveLabel: Plasmoid.formFactor !== PlasmaCore.Types.Vertical && Plasmoid.configuration.menuLabel !== ""

        readonly property int iconSize: 48

        readonly property var sizing: {
            const displayedIcon = buttonIcon.valid ? buttonIcon : buttonIconFallback;

            let impWidth = 0;
            if (shouldHaveIcon) {
                impWidth += displayedIcon.width;
            }
            if (shouldHaveLabel) {
                impWidth += labelTextField.contentWidth + labelTextField.Layout.leftMargin + labelTextField.Layout.rightMargin;
            }
            const impHeight = Math.max(iconSize, displayedIcon.height);

            // at least square, but can be wider/taller
            if (kickoff.inPanel) {
                if (kickoff.vertical) {
                    return {
                        minimumWidth: -1,
                        maximumWidth: iconSize,
                        minimumHeight: impHeight,
                        maximumHeight: impHeight,
                    };
                } else { // horizontal
                    return {
                        minimumWidth: impWidth,
                        maximumWidth: impWidth,
                        minimumHeight: -1,
                        maximumHeight: iconSize,
                    };
                }
            } else {
                return {
                    minimumWidth: impWidth,
                    maximumWidth: -1,
                    minimumHeight: Kirigami.Units.iconSizes.small,
                    maximumHeight: -1,
                };
            }
        }

        implicitWidth: iconSize
        implicitHeight: iconSize

        Layout.minimumWidth: sizing.minimumWidth
        Layout.maximumWidth: sizing.maximumWidth
        Layout.minimumHeight: sizing.minimumHeight
        Layout.maximumHeight: sizing.maximumHeight

        hoverEnabled: true

        property bool wasExpanded

        Accessible.name: Plasmoid.title

        onPressed: wasExpanded = kickoff.expanded
        onClicked: kickoff.expanded = !wasExpanded

        DropArea {
            id: compactDragArea
            anchors.fill: parent
        }

        Timer {
            id: expandOnDragTimer
            // this is an interaction and not an animation, so we want it as a constant
            interval: 250
            running: compactDragArea.containsDrag
            onTriggered: kickoff.expanded = true
        }

        RowLayout {
            id: iconLabelRow
            anchors.fill: parent
            spacing: 0

            Kirigami.Icon {
                id: buttonIcon

                Layout.fillWidth: kickoff.vertical
                Layout.fillHeight: !kickoff.vertical
                Layout.preferredWidth: kickoff.vertical ? -1 : height / (implicitHeight / implicitWidth)
                Layout.preferredHeight: !kickoff.vertical ? -1 : width * (implicitHeight / implicitWidth)
                Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
                source: Tools.iconOrDefault(Plasmoid.formFactor, Plasmoid.icon)
                active: compactRoot.containsMouse || compactDragArea.containsDrag
                roundToIconSize: implicitHeight === implicitWidth
                visible: valid
            }

            Kirigami.Icon {
                id: buttonIconFallback
                // fallback is assumed to be square
                Layout.fillWidth: kickoff.vertical
                Layout.fillHeight: !kickoff.vertical
                Layout.preferredWidth: kickoff.vertical ? -1 : height
                Layout.preferredHeight: !kickoff.vertical ? -1 : width
                Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter

                source: buttonIcon.valid ? null : Tools.defaultIconName
                active: compactRoot.containsMouse || compactDragArea.containsDrag
                visible: !buttonIcon.valid && Plasmoid.icon !== ""
            }

            PC3.Label {
                id: labelTextField

                Layout.fillHeight: true
                Layout.leftMargin: Kirigami.Units.smallSpacing
                Layout.rightMargin: Kirigami.Units.smallSpacing

                text: Plasmoid.configuration.menuLabel
                horizontalAlignment: Text.AlignLeft
                verticalAlignment: Text.AlignVCenter
                wrapMode: Text.NoWrap
                fontSizeMode: Text.VerticalFit
                font.pixelSize: compactRoot.tooSmall ? Kirigami.Theme.defaultFont.pixelSize : Kirigami.Units.iconSizes.roundedIconSize(Kirigami.Units.gridUnit * 2)
                minimumPointSize: Kirigami.Theme.smallFont.pointSize
                visible: compactRoot.shouldHaveLabel
            }
        }
    }

    Kicker.ProcessRunner {
        id: processRunner;
    }

    Plasmoid.contextualActions: [
        PlasmaCore.Action {
            text: i18n("Edit Applications…")
            icon.name: "kmenuedit"
            visible: Plasmoid.immutability !== PlasmaCore.Types.SystemImmutable
            onTriggered: processRunner.runMenuEditor()
        }
    ]

    Component.onCompleted: {
        if (Plasmoid.hasOwnProperty("activationTogglesExpanded")) {
            Plasmoid.activationTogglesExpanded = true
        }
    }
} // root
