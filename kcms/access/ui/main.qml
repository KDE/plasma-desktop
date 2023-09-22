/*
    SPDX-FileCopyrightText: 2018 Tomaz Canabrava <tcanabrava@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

import QtQuick 2.9
import QtQuick.Layouts 1.12
import QtQuick.Window 2.2
import QtQuick.Controls 2.14 as QQC2
import org.kde.kirigami 2.4 as Kirigami
import org.kde.kcmutils as KCM

KCM.ScrollViewKCM {
    id: root

    sidebarMode: true
    LayoutMirroring.enabled: Qt.application.layoutDirection === Qt.RightToLeft
    LayoutMirroring.childrenInherit: true

    implicitWidth: Kirigami.Units.gridUnit * 44
    implicitHeight: Kirigami.Units.gridUnit * 25

    property var elements: [
        {
            icon: "notifications",
            title: i18nc("@title:window System Bell", "Bell"),
            defaultnessKey: "bellIsDefaults",
            url: "Bell.qml"
        },
        {
            icon: "input-keyboard",
            title: i18nc("@title:window System Modifier Keys", "Modifier Keys"),
            defaultnessKey: "keyboardModifiersIsDefaults",
            url: "ModifierKeys.qml"
        },
        {
            icon: "view-filter",
            title: i18nc("@title:window System keyboard filters", "Keyboard Filters"),
            defaultnessKey: "keyboardFiltersIsDefaults",
            url: "KeyboardFilters.qml"
        },
        {
            icon: "input-mouse",
            title: i18nc("@title:window System mouse navigation", "Mouse Navigation"),
            defaultnessKey: "mouseIsDefaults",
            url: "MouseNavigation.qml"
        },
        {
            icon: "audio-input-microphone",
            title: i18nc("@title:window System mouse navigation", "Screen Reader"),
            defaultnessKey: "screenReaderIsDefaults",
            url: "ScreenReader.qml"
        }
    ]

    view: ListView {
        id: listView

        model: elements
        keyNavigationEnabled: true
        activeFocusOnTab: true

        delegate: Kirigami.BasicListItem {
            icon.name: modelData.icon
            label: modelData.title

            highlighted: index === listView.currentIndex

            Rectangle {
                id: defaultIndicator
                radius: width * 0.5
                implicitWidth: Kirigami.Units.largeSpacing
                implicitHeight: Kirigami.Units.largeSpacing
                visible: kcm.defaultsIndicatorsVisible
                opacity: !kcm[modelData.defaultnessKey]
                color: Kirigami.Theme.neutralTextColor
            }

            onClicked: {
                listView.currentIndex = index;
                kcm.pop();
                kcm.push(modelData.url);
            }
        }
    }

    Component.onCompleted: {
        kcm.push("Bell.qml");
        listView.currentIndex = 0;
        kcm.columnWidth = Kirigami.Units.gridUnit * 12;
    }
}
