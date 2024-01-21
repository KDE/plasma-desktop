/*   vim:set foldmethod=marker:

    SPDX-FileCopyrightText: 2014 Ivan Cukic <ivan.cukic(at)kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.2
import QtQuick.Layouts 1.2

import org.kde.plasma.components 3.0 as PlasmaComponents
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.kirigami 2.20 as Kirigami

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

    Keys.onPressed: {
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
                text: i18nd("plasma_shell_org.kde.plasma.desktop", "Activities")
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

            // checkable: true
            // onClicked: root.closeRequested()
            onClicked: root.showingSearch = !root.showingSearch
            checked: root.showingSearch
        }

        PlasmaComponents.ToolButton {
            id: configureButton
            icon.name: "configure"
            visible: KConfig.KAuthorized.authorizeControlModule("kcm_activities")
            onClicked: {
                KCM.KCMLauncher.openSystemSettings("kcm_activities");
                root.closeRequested();
            }
        }

        PlasmaComponents.ToolButton {
            id: closeButton
            icon.name: "window-close"
            onClicked: root.closeRequested()
        }

    }
}
