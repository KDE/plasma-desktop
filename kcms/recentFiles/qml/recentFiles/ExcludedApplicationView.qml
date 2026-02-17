/*   vim:set foldenable foldmethod=marker:

    SPDX-FileCopyrightText: 2012 Ivan Cukic <ivan.cukic@kde.org>
    SPDX-FileCopyrightText: 2025 Nate Graham <nate@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2

import org.kde.kirigami as Kirigami

QQC2.ScrollView {
    id: scrollview

    readonly property bool narrowMode: width < Kirigami.Units.gridUnit * 30

    enabled: applicationModel.enabled

    Kirigami.StyleHints.showFramedBackground: true

    GridLayout {
        id: grid

        // The delegate brings its own
        rowSpacing: 0
        columnSpacing: 0

        columns: scrollview.narrowMode ? 1 : 4

        clip: true

        Repeater {
            model: applicationModel
            delegate: QQC2.CheckDelegate {
                Layout.preferredWidth: Math.round(scrollview.availableWidth/ grid.columns) - 1
                Layout.preferredHeight: scrollview.narrowMode ? -1 : Kirigami.Units.gridUnit * 5

                hoverEnabled: applicationModel.enabled

                display: scrollview.narrowMode ? QQC2.AbstractButton.TextBesideIcon : QQC2.ItemDelegate.TextUnderIcon
                icon.name: model.icon
                icon.height: Kirigami.Settings.isMobile || display === QQC2.ItemDelegate.TextUnderIcon ? Kirigami.Units.iconSizes.medium : Kirigami.Units.iconSizes.smallMedium
                text: model.title

                opacity: model.blocked ? 0.3 : 1.0
                Behavior on opacity { NumberAnimation { duration: Kirigami.Units.shortDuration } }

                checked: !model.blocked
                onToggled: applicationModel.toggleApplicationBlocked(model.index)
            }
        }
    }
}
