/*
    SPDX-FileCopyrightText: 2022 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

pragma ComponentBehavior: Bound

import QtQuick

import org.kde.plasma.components as PC3
import org.kde.plasma.plasmoid

KickoffGridView {
    id: root

    property string currentSection
    property KickoffListView parentView

    /**
     * Request hiding the section view
     */
    signal hideSectionViewRequested(int index)

    // prevent binding loops and preserve minimum popup size
    view.implicitWidth: parentView.view.implicitWidth
    view.implicitHeight: parentView.view.implicitHeight

    // Using implicitWidth instead of width so that delegates don't
    // become super big when using the new popup resizing feature.
    view.cellWidth: Math.floor((view.implicitWidth - view.leftMargin - view.rightMargin) / (kickoff.minimumGridRowCount * 1.75))
    view.cellHeight: view.cellWidth

    delegate: PC3.AbstractButton {
        id: delegate

        required property int index
        required property int firstIndex
        required property string section

        width: view.cellWidth
        height: view.cellHeight

        hoverEnabled: true
        onHoveredChanged: if (hovered) {
            root.view.currentIndex = index;
        }

        padding: fontMetrics.descent / 2

        contentItem: PC3.Label {
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            maximumLineCount: 1
            elide: Text.ElideRight

            // Sets max size for fontSizeMode, not true size.
            // Also affects implicit size,
            // so do not rely on this Label's implicit size.
            font.pixelSize: fontMetrics.font.pixelSize
            fontSizeMode: Text.VerticalFit

            text: delegate.section
            textFormat: Text.PlainText
        }

        onClicked: {
            const isGridView = Plasmoid.configuration.applicationsDisplay === 0
            const destinationIndex = isGridView ? index : firstIndex
            root.hideSectionViewRequested(destinationIndex)
        }
    }

    FontMetrics {
        id: fontMetrics
        // This size doesn't actually fill the cell height perfectly.
        // It goes out of bounts and is slightly below center with Noto Sans.
        // It's close enough for the calculations this will be used for.
        // The calculations seem to work fairly well with other fonts too.
        font.pixelSize: root.view.cellHeight
    }

    Component.onCompleted: {
        for (let i = 0; i < model.count; i++) {
            if (model.data(model.index([i], 0), Qt.DisplayRole) === root.currentSection) {
                view.positionViewAtIndex(i, ListView.Beginning);
                view.currentIndex = i;
                return;
            }
        }
    }
}
