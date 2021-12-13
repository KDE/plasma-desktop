/* SPDX-FileCopyrightText: 2021 Noah Davis <noahadvs@gmail.com>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

pragma Singleton // NOTE: Singletons are shared between all instances of a plasmoid

import QtQml.Models 2.15
import QtQuick 2.15
import QtQuick.Templates 2.15 as T
import QtQml 2.15
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 3.0 as PC3
import org.kde.plasma.private.kicker 0.1 as Kicker

// Using Item because it has a default property.
// Trying to create a default property for a QtObject seems to cause segfaults.
Item {
    id: root
    visible: false

    //BEGIN Models and Data Sources
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

    //BEGIN Reusable Objects
    readonly property PlasmaCore.Svg lineSvg: PlasmaCore.Svg {
        imagePath: "widgets/line"
    }
    readonly property PlasmaCore.Svg arrowsSvg: PlasmaCore.Svg {
        imagePath: "widgets/arrows"
    }
    //END

    //BEGIN Metrics
    readonly property PlasmaCore.FrameSvgItem listItemMetrics: PlasmaCore.FrameSvgItem {
        visible: false
        imagePath: "widgets/listitem"
        prefix: "normal"
    }

    readonly property FontMetrics fontMetrics: FontMetrics {
        id: fontMetrics
        font: PlasmaCore.Theme.defaultFont
    }

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
