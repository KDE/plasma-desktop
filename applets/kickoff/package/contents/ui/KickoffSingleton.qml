/* SPDX-FileCopyrightText: 2021 Noah Davis <noahadvs@gmail.com>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

pragma Singleton

import QtQml.Models 2.15
import QtQuick 2.15
import QtQuick.Templates 2.15 as T
import QtQml 2.15
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PC2
import org.kde.plasma.components 3.0 as PC3
import org.kde.plasma.private.kicker 0.1 as Kicker

// Using Item because it has a default property.
// Trying to create a default property for a QtObject seems to cause segfaults.
Item {
    id: root
    visible: false
    // Workaround for `plasmoid` context property not working in singletons
    property var plasmoid: null
    //BEGIN Models and Data Sources
    // These are set in FullRepresentation.qml because the `plasmoid` context property
    // doesn't work here and using `Plasmoid` from org.kde.plasma.plasmoid doesn't work either.
    // Even the `plasmoid` property defined above doesn't quite work,
    // but it works better than nothing for things like `plasmoid.expanded`
    property Kicker.RootModel rootModel
    property Kicker.RunnerModel runnerModel
    property Kicker.ComputerModel computerModel
    property Kicker.RecentUsageModel recentUsageModel
    property Kicker.RecentUsageModel frequentUsageModel

    readonly property PlasmaCore.DataSource powerManagement: PlasmaCore.DataSource {
        engine: "powermanagement"
        connectedSources: ["PowerDevil"]
        // For some reason, these signal handlers need to be here for `data` to actually contain data.
        onSourceAdded: {
            disconnectSource(source);
            connectSource(source);
        }
        onSourceRemoved: {
            disconnectSource(source);
        }
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
    //END

    //BEGIN Reusable Objects
    readonly property PlasmaCore.Svg lineSvg: PlasmaCore.Svg {
        imagePath: "widgets/line"
    }
    readonly property PlasmaCore.Svg arrowsSvg: PlasmaCore.Svg {
        imagePath: "widgets/arrows"
    }
    //END

    //BEGIN State Properties
    // Set in Kickoff.qml
    property bool inPanel: false
    property bool vertical: false
    //END

    //BEGIN Metrics
    readonly property PlasmaCore.FrameSvgItem backgroundMetrics: PlasmaCore.FrameSvgItem {
        visible: false
        imagePath: plasmoid.formFactor === PlasmaCore.Types.Planar ? "widgets/background" : "dialogs/background"
    }

    readonly property PlasmaCore.FrameSvgItem listItemMetrics: PlasmaCore.FrameSvgItem {
        visible: false
        imagePath: "widgets/listitem"
        prefix: "normal"
    }

    readonly property FontMetrics fontMetrics: FontMetrics {
        id: fontMetrics
        font: PlasmaCore.Theme.defaultFont
    }

    readonly property real leftPadding: backgroundMetrics.margins.left - backgroundMetrics.inset.left
    readonly property real rightPadding: backgroundMetrics.margins.right - backgroundMetrics.inset.right
    readonly property real topPadding: backgroundMetrics.margins.top - backgroundMetrics.inset.top
    readonly property real bottomPadding: backgroundMetrics.margins.bottom - backgroundMetrics.inset.bottom
    readonly property real spacing: KickoffSingleton.leftPadding
    readonly property real gridCellSize: gridDelegate.implicitHeight
    readonly property real listDelegateHeight: listDelegate.implicitHeight
    readonly property real listDelegateContentHeight: listDelegate.implicitContentHeight
    //END

    //BEGIN Private
    KickoffItemDelegate {
        id: gridDelegate
        visible: false
        enabled: false
        icon.width: PlasmaCore.Units.iconSizes.large
        icon.height: PlasmaCore.Units.iconSizes.large
        model: null
        index: -1
        text: "asdf"
        url: ""
        decoration: "start-here-kde"
        description: "asdf"
        display: PC3.AbstractButton.TextUnderIcon
        width: implicitHeight
        action: null
        indicator: null
    }
    KickoffItemDelegate {
        id: listDelegate
        visible: false
        enabled: false
        icon.width: PlasmaCore.Units.iconSizes.smallMedium
        icon.height: PlasmaCore.Units.iconSizes.smallMedium
        model: null
        index: -1
        text: "asdf"
        url: ""
        decoration: "start-here-kde"
        description: "asdf"
        action: null
        indicator: null
    }
    //END
}
