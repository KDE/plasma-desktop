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
    rows: {
        if (tasks.vertical) {
            if (tasks.plasmoid.configuration.maxStripes > 1 && !tasks.plasmoid.configuration.forceStripes) {
                return Math.max(Math.ceil(tasks.height / LayoutMetrics.preferredMinHeight()),
                                Math.ceil(tasksModel.count / tasks.plasmoid.configuration.maxStripes));
            } else {
                return -1;
            }
        } else {
            return Math.min(tasks.plasmoid.configuration.maxStripes, Math.max(1, Math.floor(tasks.height / children[0].implicitHeight)));
        }
    }
    columns: {
        if (tasks.vertical) {
            return Math.min(tasks.plasmoid.configuration.maxStripes, Math.max(1, Math.floor(tasks.width / children[0].implicitWidth)));
        } else {
            if (tasks.plasmoid.configuration.maxStripes > 1 && !tasks.plasmoid.configuration.forceStripes) {
                return Math.max(Math.ceil(tasks.width / LayoutMetrics.preferredMinWidth()),
                                Math.ceil(tasksModel.count / tasks.plasmoid.configuration.maxStripes));
            } else {
                return -1;
            }
        }
    }
}
