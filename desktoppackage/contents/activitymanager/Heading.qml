/*   vim:set foldmethod=marker:

    SPDX-FileCopyrightText: 2014 Ivan Cukic <ivan.cukic(at)kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.2
import QtQuick.Layouts 1.2

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.extras 2.0 as PlasmaExtras

// for KCMShell
import org.kde.kquickcontrolsaddons 2.0

import org.kde.activities.settings 0.1

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
            PlasmaExtras.Heading {
                id: heading

                anchors.fill: parent

                level: 1
                text: i18nd("plasma_shell_org.kde.plasma.desktop", "Activities")
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
            iconSource: "edit-find"

            // checkable: true
            // onClicked: root.closeRequested()
            onClicked: root.showingSearch = !root.showingSearch
            checked: root.showingSearch
        }

        PlasmaComponents.ToolButton {
            id: configureButton
            iconSource: "configure"
            visible: KCMShell.authorize("kcm_activities.desktop").length > 0
            onClicked: {
                KCMShell.openSystemSettings("kcm_activities");
                root.closeRequested();
            }
        }

        PlasmaComponents.ToolButton {
            id: closeButton
            iconSource: "window-close"
            onClicked: root.closeRequested()
        }

    }
}
