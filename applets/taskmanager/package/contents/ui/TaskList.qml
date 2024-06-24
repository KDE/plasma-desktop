/*
    SPDX-FileCopyrightText: 2012-2013 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Layouts
import org.kde.kirigami 2.20 as Kirigami
import org.kde.plasma.plasmoid 2.0
import "code/layoutmetrics.js" as LayoutMetrics

GridLayout {
    property bool animating: false

    layoutDirection: (Plasmoid.configuration.reverseMode && !tasks.vertical)
        ? (Qt.application.layoutDirection === Qt.LeftToRight)
            ? Qt.RightToLeft
            : Qt.LeftToRight
        : Qt.application.layoutDirection

    rowSpacing: 0
    columnSpacing: 0
    property int animationsRunning: 0
    onAnimationsRunningChanged: animating = animationsRunning > 0
    property real minimumWidth: {
        let min = Infinity;
        for (let item of children) {
            if (item.visible && item.width > 0 && item.width < min) {
                min = item.width;
            }
        }
        return min;
    }

    readonly property int stripeCount: {
        if (tasks.plasmoid.configuration.maxStripes == 1) {
            return 1;
        }

        // The maximum number of stripes allowed by the applet's size
        const stripeSizeLimit = tasks.vertical
            ? Math.floor(tasks.width / children[0].implicitWidth)
            : Math.floor(tasks.height / children[0].implicitHeight)
        const maxStripes = Math.min(tasks.plasmoid.configuration.maxStripes, stripeSizeLimit)

        if (tasks.plasmoid.configuration.forceStripes) {
            return maxStripes;
        }

        // The number of tasks that will fill a "stripe" before starting the next one
        const maxTasksPerStripe = tasks.vertical
            ? Math.ceil(tasks.height / LayoutMetrics.preferredMinHeight())
            : Math.ceil(tasks.width / LayoutMetrics.preferredMinWidth())

        return Math.min(Math.ceil(tasksModel.count / maxTasksPerStripe), maxStripes)
    }

    readonly property int orthogonalCount: {
        return Math.ceil(tasksModel.count / stripeCount);
    }

    rows: tasks.vertical ? orthogonalCount : stripeCount
    columns: tasks.vertical ? stripeCount : orthogonalCount
}
