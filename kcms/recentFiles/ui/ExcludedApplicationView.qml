/*   vim:set foldenable foldmethod=marker:

    SPDX-FileCopyrightText: 2012 Ivan Cukic <ivan.cukic@kde.org>
    SPDX-FileCopyrightText: 2025 Nate Graham <nate@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2

import org.kde.kirigami as Kirigami

QQC2.ScrollView {
    id: root

    required property var model

    required property real pageWidth

    implicitWidth: grid.cellWidth * (pageWidth < Kirigami.Units.gridUnit * 55 ? 2 : 4)
    leftPadding: Kirigami.Units.largeSpacing
    bottomPadding: Kirigami.Units.largeSpacing

    Kirigami.StyleHints.showFramedBackground: true

    GridView {
        id: grid
        cellWidth: Kirigami.Units.gridUnit * 10
        cellHeight: Kirigami.Units.gridUnit * 5

        model: root.model
        delegate: QQC2.CheckDelegate {
            id: applicationDelegate
            required property int index
            required property string title
            required property string iconSource
            required property bool blocked

            width: GridView.view.cellWidth
            height: GridView.view.cellHeight
            hoverEnabled: root.enabled

            display: QQC2.ItemDelegate.TextUnderIcon
            icon.name: applicationDelegate.iconSource
            icon.height: Kirigami.Settings.isMobile || display === QQC2.ItemDelegate.TextUnderIcon ? Kirigami.Units.iconSizes.medium : Kirigami.Units.iconSizes.smallMedium
            text: applicationDelegate.title

            opacity: applicationDelegate.blocked ? 0.3 : 1.0
            Behavior on opacity { NumberAnimation { duration: Kirigami.Units.shortDuration } }

            checked: !applicationDelegate.blocked
            onToggled: root.model.toggleApplicationBlocked(applicationDelegate.index)
        }
    }
}
