/*
    SPDX-FileCopyrightText: 2018 Tomaz Canabrava <tcanabrava@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

import QtQuick 2.9
import QtQuick.Layouts 1.12
import QtQuick.Window 2.2
import QtQuick.Controls as QQC2
import org.kde.kirigami 2.4 as Kirigami
import org.kde.kirigami.delegates as KD
import org.kde.kcmutils as KCM
import org.kde.kwindowsystem

KCM.AbstractKCM {
    id: root

    implicitWidth: Kirigami.Units.gridUnit * 45
    implicitHeight: Kirigami.Units.gridUnit * 25

    framedView: false

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
            icon: "input-caps-on",
            title: i18n("Activation Gestures"),
            defaultnessKey: "activationGesturesIsDefaults"
        },
        {
            icon: "audio-input-microphone",
            title: i18nc("System mouse navigation", "Screen Reader"),
            defaultnessKey: "screenReaderIsDefaults"
        },
        {
            icon: "cursor-arrow",
            title: i18nc("Shake cursor to find it", "Shake Cursor"),
            defaultnessKey: "shakeCursorIsDefaults",
            available: KWindowSystem.isPlatformWayland
        }
    ]

    RowLayout {
        id: mainLayout
        anchors.fill: parent
        spacing: 0

        QQC2.ScrollView {
            id: leftSidePaneBackground
            Layout.fillHeight: true
            Layout.minimumWidth: Kirigami.Units.gridUnit * 13

            Kirigami.Theme.colorSet: Kirigami.Theme.View
            Kirigami.Theme.inherit: false

            ListView {
                id: listView
                activeFocusOnTab: true
                clip: true
                keyNavigationEnabled: true
                model: elements

                delegate: QQC2.ItemDelegate {
                    id: baseDelegate

                    width: listView.width

                    icon.name: modelData.icon
                    text: modelData.title
                    visible: modelData.available === undefined || modelData.available

                    onClicked: {
                        listView.currentIndex = index
                        listView.forceActiveFocus()
                    }

                    contentItem: RowLayout {
                        spacing: Kirigami.Units.smallSpacing

                        KD.IconTitleSubtitle {
                            Layout.fillWidth: true
                            icon.name: baseDelegate.icon.name
                            title: baseDelegate.text
                        }

                        Rectangle {
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
        }

        Kirigami.Separator {
            Layout.fillHeight: true
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Kirigami.Theme.colorSet: Kirigami.Theme.Window
            Kirigami.Theme.inherit: false
            color: Kirigami.Theme.backgroundColor

            QQC2.ScrollView {
                id: contentScroll
                anchors {
                    fill: parent
                    leftMargin: Kirigami.Units.largeSpacing
                }
                contentWidth: availableWidth - contentItem.leftMargin - contentItem.rightMargin

                StackLayout {
                    id: stackView
                    width: contentScroll.width
                    currentIndex: listView.currentIndex

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
                    ActivationGestures {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                    }
                    ScreenReader {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                    }
                    ShakeCursor {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                    }
                }
            }
        }
    }
}
