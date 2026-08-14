/*   vim:set foldmethod=marker:

    SPDX-FileCopyrightText: 2014 Ivan Cukic <ivan.cukic(at)kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts

import org.kde.plasma.components as PlasmaComponents
import org.kde.plasma.extras as PlasmaExtras
import org.kde.kirigami as Kirigami

import org.kde.kcmutils as KCM
import org.kde.config as KConfig

Item {
    id: root

    property alias searchString: searchText.text
    property bool showingSearch: false

    signal closeRequested

    function focusSearch() {
        searchText.forceActiveFocus()
    }

    onShowingSearchChanged: if (!showingSearch) searchText.text = ""

    Keys.onPressed: event => {
        if (event.key === Qt.Key_Escape) {
            if (root.showingSearch) {
                event.accepted = true;
                root.showingSearch = false;
            }
        }
    }

    height: childrenRect.height

    RowLayout {
        id: buttonRow

        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
        }

        Item {
            Kirigami.Heading {
                id: heading

                anchors.fill: parent

                level: 1
                text: i18ndc("plasma_shell_org.kde.plasma.desktop", "@title:window", "Activities")
                textFormat: Text.PlainText
                elide: Text.ElideRight

                visible: !root.showingSearch
            }

            PlasmaExtras.SearchField {
                id: searchText

                anchors.fill: parent

                focus: true
                visible: root.showingSearch

                onTextChanged: if (text != "") root.showingSearch = true
            }

            Layout.fillWidth: true
            Layout.fillHeight: true
        }

        PlasmaComponents.ToolButton {
            id: searchButton

            icon.name: "edit-find"
            display: PlasmaComponents.AbstractButton.IconOnly
            text: i18ndc("plasma_shell_org.kde.plasma.desktop", "@action:button tooltip only", "Search")

            checkable: true
            checked: root.showingSearch

            PlasmaComponents.ToolTip.delay: Kirigami.Units.toolTipDelay
            PlasmaComponents.ToolTip.visible: hovered || activeFocus
            PlasmaComponents.ToolTip.text: text

            onClicked: root.showingSearch = !root.showingSearch
        }

        PlasmaComponents.ToolButton {
            id: configureButton

            icon.name: "configure"
            display: PlasmaComponents.AbstractButton.IconOnly
            text: i18ndc("plasma_shell_org.kde.plasma.desktop", "@action:button tooltip only, opens kcm", "Configure Activities")

            visible: KConfig.KAuthorized.authorizeControlModule("kcm_activities")

            PlasmaComponents.ToolTip.delay: Kirigami.Units.toolTipDelay
            PlasmaComponents.ToolTip.visible: hovered || activeFocus
            PlasmaComponents.ToolTip.text: text

            onClicked: {
                KCM.KCMLauncher.openSystemSettings("kcm_activities");
                root.closeRequested();
            }
        }

        PlasmaComponents.ToolButton {
            id: closeButton

            icon.name: "window-close"
            display: PlasmaComponents.AbstractButton.IconOnly
            text: i18ndc("plasma_shell_org.kde.plasma.desktop", "@action:button tooltip only, close panel", "Close")

            PlasmaComponents.ToolTip.delay: Kirigami.Units.toolTipDelay
            PlasmaComponents.ToolTip.visible: hovered || activeFocus
            PlasmaComponents.ToolTip.text: text

            onClicked: root.closeRequested()
        }

    }
}
