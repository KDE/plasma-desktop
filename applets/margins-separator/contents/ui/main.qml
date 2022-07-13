/*
    SPDX-FileCopyrightText: 2020 Niccolò Venerandi <niccolo@venerandi.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

import QtQuick 2.4
import QtQuick.Layouts 1.0
import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.extras 2.0 as PlasmaExtras

Item {
    id: root

    readonly property bool isVertical: Plasmoid.formFactor === PlasmaCore.Types.Vertical

    Layout.minimumWidth: Plasmoid.editMode && !isVertical ? PlasmaCore.Units.largeSpacing : PlasmaCore.Units.devicePixelRatio
    Layout.preferredWidth: Layout.minimumWidth
    Layout.maximumWidth:   Layout.minimumWidth

    Layout.minimumHeight: Plasmoid.editMode && isVertical ? PlasmaCore.Units.largeSpacing : Layout.minimumWidth
    Layout.preferredHeight: Layout.minimumHeight
    Layout.maximumHeight: Layout.minimumHeight

    Plasmoid.constraintHints: PlasmaCore.Types.MarginAreasSeparator
    Plasmoid.preferredRepresentation: Plasmoid.fullRepresentation

    Loader {
        anchors.centerIn: parent
        active: Plasmoid.editMode
        sourceComponent: PlasmaCore.SvgItem {
            height: root.isVertical ? PlasmaCore.Units.devicePixelRatio : Math.round(root.height / 2)
            width: root.isVertical ? Math.round(root.width / 2) : PlasmaCore.Units.devicePixelRatio
            svg: PlasmaCore.Svg {imagePath: "widgets/line"}
            elementId: root.isVertical ? "vertical-line" : "horizontal-line"
        }
    }
}
