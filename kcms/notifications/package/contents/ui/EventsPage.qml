/*
 * Copyright 2019 Kai Uwe Broulik <kde@privat.broulik.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.9
import QtQml.Models 2.2
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.3 as QtControls

import org.kde.kirigami 2.7 as Kirigami
import org.kde.kcm 1.2 as KCM

ColumnLayout {
    id: eventsColumn

    property var rootIndex

    property bool customActivitySettings: false

    readonly property var actions: [
        {key: "Popup", label: i18n("Show popup"), icon: "dialog-information"},
        {key: "Sound", label: i18n("Play sound"), icon: "folder-sound"},// "media-playback-start"},
        {key: "Logfile", label: i18n("Log to a file"), icon: "text-x-generic"},
        {key: "Taskbar", label: i18n("Mark taskbar entry"), icon: "services"},
        {key: "Execute", label: i18n("Run command"), icon: "system-run"},
        {key: "TTS", label: i18n("Speech"), icon: "text-speak"} // FIXME only when available
    ]

    spacing: Kirigami.Units.smallSpacing

    Kirigami.FormLayout {
        Layout.fillWidth: true

        QtControls.CheckBox {
            id: showPopupsCheck
            text: i18n("Show popups")
            checked: true
        }

        RowLayout {
            Item {
                width: Kirigami.Units.gridUnit
            }

            QtControls.CheckBox {
                text: i18n("Show in do not disturb mode")
                enabled: showPopupsCheck.checked
            }
        }

        QtControls.CheckBox {
            text: i18n("Notification badges")
            enabled: !!eventsColumn.desktopEntry
        }
    }

    Item {
        Layout.fillWidth: true
        Layout.fillHeight: true
        visible: eventsList.count > 0

        QtControls.ScrollView {
            id: eventsScroll

            anchors.fill: parent
            activeFocusOnTab: false
            Kirigami.Theme.colorSet: Kirigami.Theme.View
            Kirigami.Theme.inherit: false

            enabled: !eventsColumn.customActivitySettings

            Component.onCompleted: background.visible = true

            QtControls.ScrollBar.horizontal.visible: false

            ListView {
                id: eventsList
                anchors {
                    fill: parent
                    margins: 2
                    //leftMargin: sourcesScroll.QtControls.ScrollBar.vertical.visible ? 2 :  internal.scrollBarSpace/2 + 2
                }
                clip: true
                activeFocusOnTab: true

                keyNavigationEnabled: true
                keyNavigationWraps: true
                highlightMoveDuration: 0

                model: DelegateModel {
                    id: eventsModel
                    model: eventsColumn.rootIndex ? kcm.filteredModel : null
                    rootIndex: eventsColumn.rootIndex
                    onRootIndexChanged: eventsList.currentIndex = -1
                    delegate: QtControls.ItemDelegate {
                        id: eventDelegate

                        function isActionEnabled(action) {
                            return model.actions.indexOf(action) > -1;
                        }

                        function setActionEnabled(action, enable) {
                            var actions = model.actions;
                            var idx = actions.indexOf(action);
                            if (enable && idx === -1) {
                                actions.push(action);
                            } else if (!enable && idx > -1) {
                                actions.splice(idx, 1);
                            }
                            model.actions = actions;
                        }

                        width: eventsList.width
                        text: model.display
                        onClicked: eventsList.currentIndex = (eventsList.currentIndex === index ? -1 : index)

                        contentItem: RowLayout {
                            //Kirigami.Theme.textColor: eventDelegate.highlighted || eventDelegate.checked || (eventDelegate.pressed && !eventDelegate.checked && !eventDelegate.sectionDelegate) ? Kirigami.Theme.highlightedTextColor : (eventDelegate.enabled ? Kirigami.Theme.textColor : Kirigami.Theme.disabledTextColor)

                            /*QtControls.ToolTip {
                                visible: eventDelegate.hovered
                                text: model.comment
                            }*/

                            Kirigami.Icon {
                                Layout.alignment: Qt.AlignTop
                                Layout.preferredWidth: Kirigami.Units.iconSizes.small
                                Layout.preferredHeight: Kirigami.Units.iconSizes.small
                                source: model.decoration
                            }

                            ColumnLayout {
                                Layout.fillWidth: true

                                RowLayout {
                                    Layout.fillWidth: true

                                    QtControls.Label {
                                        Layout.fillWidth: true
                                        text: eventDelegate.text
                                        font: eventDelegate.font
                                        //color:
                                        elide: Text.ElideRight
                                    }

                                    Repeater {
                                        model: eventsColumn.actions

                                        // TODO use abstract button?
                                        QtControls.AbstractButton {
                                            id: actionStripButton
                                            Layout.preferredWidth: Kirigami.Units.iconSizes.small
                                            Layout.preferredHeight: Kirigami.Units.iconSizes.small
                                            icon.name: modelData.icon
                                            checkable: true
                                            checked: eventDelegate.isActionEnabled(modelData.key)
                                            onClicked: {
                                                eventDelegate.setActionEnabled(modelData.key, checked)
                                                if (checked) {
                                                    // Some actons might need configuration (e.g. sound needs a filename)
                                                    // FIXME check this and expand and focus if needed
                                                }
                                            }

                                            contentItem: Kirigami.Icon {
                                                anchors.fill: parent
                                                source: modelData.icon
                                                opacity: actionStripButton.checked ? 1 : (actionStripButton.hovered ? 0.5 : 0.1)
                                            }

                                            QtControls.ToolTip {
                                                text: modelData.label
                                                visible: parent.hovered
                                            }
                                        }
                                    }
                                }

                                Loader {
                                    Layout.fillWidth: true
                                    active: eventDelegate.ListView.isCurrentItem
                                    visible: active
                                    sourceComponent: EventDetails {
                                        Layout.fillWidth: true
                                        model: eventsColumn.actions
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        QtControls.Label {
            anchors {
                verticalCenter: parent.verticalCenter
                left: parent.left
                right: parent.right
                margins: Kirigami.Units.smallSpacing
            }
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WordWrap
            text: i18n("Application events cannot be configured on a per-activity basis.")
            visible: eventsColumn.customActivitySettings
        }
    }

    // compact layout when event list isnt't shown
    Item {
        Layout.fillHeight: true
        visible: eventsList.count === 0
    }
}
