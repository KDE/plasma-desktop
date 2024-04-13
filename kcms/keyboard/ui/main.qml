/*
    SPDX-FileCopyrightText: 2024 Evgeny Chesnokov <echesnokov@astralinux.ru>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2

import org.kde.kirigami as Kirigami
import org.kde.kcmutils as KCM

import org.kde.kquickcontrols as KQuickControls

KCM.AbstractKCM {
    id: root

    implicitWidth: Kirigami.Units.gridUnit * 44
    implicitHeight: Kirigami.Units.gridUnit * 25
    framedView: false

    Rectangle {
        // Custom background to match framedView bg
        anchors.fill: parent
        color: Kirigami.Theme.backgroundColor
    }

    ColumnLayout {
        anchors.fill: parent
        // NOTE: Kirigami.Units.smallSpacing make ugly gap between navigation and content
        spacing: 0

        Kirigami.NavigationTabBar {
            Layout.fillWidth: true
            currentIndex: 0

            actions: [
                Kirigami.Action {
                    icon.name: "input-keyboard-symbolic"
                    text: i18nc("@title:tab", "Hardware")
                    checked: contentLoader.sourceComponent === _cHardware
                    onTriggered: contentLoader.sourceComponent = _cHardware
                },
                Kirigami.Action {
                    icon.name: "languages-symbolic"
                    text: i18nc("@title:tab", "Layouts")
                    checked: contentLoader.sourceComponent === _cLayouts
                    onTriggered: contentLoader.sourceComponent = _cLayouts
                },
                Kirigami.Action {
                    icon.name: "settings-configure-symbolic"
                    text: i18nc("@title:tab", "Key Bindings")
                    checked: contentLoader.sourceComponent === _cAdvanced
                    onTriggered: contentLoader.sourceComponent = _cAdvanced
                }
            ]
        }

        Loader {
            id: contentLoader
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.margins: root.margins
            asynchronous: true
            sourceComponent: _cHardware
        }
    }

    // For main & 3rd lvl shortcuts behavior
    function goToAdvanced(group): void {
        contentLoader.sourceComponent = _cAdvanced;
        kcm.xkbOptionsModel.navigateToGroup(group);
    }

    Component {
        id: _cHardware

        Hardware {}
    }

    Component {
        id: _cLayouts

        Layouts {}
    }

    Component {
        id: _cAdvanced

        Advanced {}
    }
}
