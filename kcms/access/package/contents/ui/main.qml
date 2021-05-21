/*
    SPDX-FileCopyrightText: 2018 Tomaz Canabrava <tcanabrava@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

import QtQuick 2.9
import QtQuick.Layouts 1.12
import QtQuick.Window 2.2
import QtQuick.Dialogs 1.0 as QtDialogs
import QtQuick.Controls 2.14 as QQC2
import org.kde.kirigami 2.4 as Kirigami
import org.kde.kcm 1.1 as KCM

KCM.SimpleKCM {
    id: root

    implicitWidth: Kirigami.Units.gridUnit * 44
    implicitHeight: Kirigami.Units.gridUnit * 25

    KCM.ConfigModule.quickHelp: i18n("This module lets you configure the accessibility features such as a screen reader.")

    property var elements: [
        {
            icon: "notifications",
            title: i18nc("System Bell", "Bell"),
            defaultnessKey: "bellIsDefaults"
        },
        {
            icon: "input-keyboard",
            title: i18nc("System Modifier Keys", "Modifier Keys"),
            defaultnessKey: "keyboardModifiersIsDefaults"
        },
        {
            icon: "view-filter",
            title: i18nc("System keyboard filters", "Keyboard Filters"),
            defaultnessKey: "keyboardFiltersIsDefaults"
        },
        {
            icon: "input-mouse",
            title: i18nc("System mouse navigation", "Mouse Navigation"),
            defaultnessKey: "mouseIsDefaults"
        },
        {
            icon: "audio-input-microphone",
            title: i18nc("System mouse navigation", "Screen Reader"),
            defaultnessKey: "screenReaderIsDefaults"
        }
    ]

    RowLayout {
        id: mainLayout
        anchors.margins: Kirigami.Units.largeSpacing
        QQC2.ScrollView {
            id: leftSidePaneBackground
            contentHeight: root.contentItem.height -  Kirigami.Units.gridUnit * 4
            contentWidth: Kirigami.Units.gridUnit * 13

            QQC2.ScrollBar.horizontal.policy: QQC2.ScrollBar.AlwaysOff
            QQC2.ScrollBar.vertical.policy: QQC2.ScrollBar.AlwaysOff

            Component.onCompleted: leftSidePaneBackground.background.visible = true

            ListView {
                id: listView
                activeFocusOnTab: true
                clip: true
                keyNavigationEnabled: true
                model: elements

                onCurrentIndexChanged: stackView.currentIndex = currentIndex

                delegate: Kirigami.BasicListItem {
                    width: listView.width
                    icon: modelData.icon
                    label: modelData.title
                    onClicked: listView.forceActiveFocus()
                    Rectangle {
                        id: defaultIndicator
                        radius: width * 0.5
                        implicitWidth: Kirigami.Units.largeSpacing
                        implicitHeight: Kirigami.Units.largeSpacing
                        visible: kcm.defaultsIndicatorsVisible
                        opacity: !kcm[modelData.defaultnessKey]
                        color: Kirigami.Theme.neutralTextColor
                    }
                }
            }
        }

        StackLayout {
            id: stackView
            Layout.fillWidth: true
            Layout.fillHeight: true

            Bell {
                Layout.fillWidth: true
                Layout.fillHeight: true
            }
            ModifierKeys {
                Layout.fillWidth: true
                Layout.fillHeight: true

            }
            KeyboardFilters {
                Layout.fillWidth: true
                Layout.fillHeight: true

            }
            MouseNavigation {
                Layout.fillWidth: true
                Layout.fillHeight: true

            }
            ScreenReader {
                Layout.fillWidth: true
                Layout.fillHeight: true
            }
        }
    }
}
