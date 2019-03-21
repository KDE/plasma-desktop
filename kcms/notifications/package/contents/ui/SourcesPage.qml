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
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.3 as QtControls

import org.kde.kirigami 2.7 as Kirigami
import org.kde.kcm 1.2 as KCM

import org.kde.private.kcms.notifications 1.0 as Private

Kirigami.Page {
    id: sourcesPage
    title: i18n("Application Settings")

    readonly property bool hasActivities: activityTabsRepeater.count > 1

    Binding {
        target: kcm.filteredModel
        property: "query"
        value: searchField.text
    }

    ColumnLayout {
        id: rootColumn
        anchors.fill: parent
        spacing: 0

        QtControls.TabBar {
            id: activitiesTabBar
            Layout.fillWidth: true
            visible: sourcesPage.hasActivities

            Repeater {
                id: activityTabsRepeater
                model: kcm.activitiesModel

                QtControls.TabButton {
                    readonly property bool isCurrent: model.isCurrent
                    text: model.name
                    // FIXME support icon in tab button
                    //icon.name: model.iconSource

                    // Switch to current activity initially
                    Component.onCompleted: {
                        if (model.isCurrent) {
                            activitiesTabBar.currentIndex = index;
                        }
                    }
                }
            }
        }

        QtControls.Frame {
            Layout.fillWidth: true
            Layout.fillHeight: true

            padding: sourcesPage.hasActivities ? 6 : 0 // magic number comes from qqc2-desktop-style

            Component.onCompleted: {
                background.visible = Qt.binding(function() {
                    return sourcesPage.hasActivities;
                });
            }

            ColumnLayout {
                anchors.fill: parent

                Kirigami.FormLayout {
                    Layout.fillWidth: false // keep left-aligned
                    visible: sourcesPage.hasActivities

                    QtControls.RadioButton {
                        id: activityDefaultCheck
                        Kirigami.FormData.label: i18n("In this activity:")
                        text: i18n("Use default settings")
                        checked: true
                    }

                    QtControls.RadioButton {
                        id: activityCustomCheck
                        text: i18n("Custom settings")
                    }
                }

                RowLayout {
                    Layout.fillWidth: true

                    ColumnLayout {
                        Layout.minimumWidth: Kirigami.Units.gridUnit * 12
                        Layout.preferredWidth: Math.round(rootColumn.width / 3)

                        /*Kirigami.SearchField {
                            Layout.fillWidth: true
                        }*/
                        QtControls.TextField { // FIXME search field
                            id: searchField
                            Layout.fillWidth: true
                            placeholderText: i18n("Search...")
                            // TODO autofocus this?

                            Shortcut {
                                sequence: StandardKey.Find
                                onActivated: searchField.forceActiveFocus()
                            }
                        }

                        QtControls.ScrollView {
                            id: sourcesScroll
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            activeFocusOnTab: false
                            Kirigami.Theme.colorSet: Kirigami.Theme.View
                            Kirigami.Theme.inherit: false

                            Component.onCompleted: background.visible = true

                            ListView {
                                id: sourcesList
                                anchors {
                                    fill: parent
                                    margins: 2
                                    //leftMargin: sourcesScroll.QtControls.ScrollBar.vertical.visible ? 2 :  internal.scrollBarSpace/2 + 2
                                }
                                clip: true
                                activeFocusOnTab: true
                                // FIXME fix current index when we filter
                                //currentIndex: -1

                                keyNavigationEnabled: true
                                keyNavigationWraps: true
                                highlightMoveDuration: 0

                                section {
                                    criteria: ViewSection.FullString
                                    property: "sourceType"
                                    delegate: QtControls.ItemDelegate {
                                        id: sourceSection
                                        width: sourcesList.width
                                        text: {
                                            switch (Number(section)) {
                                            case Private.SourcesModel.ServiceType: return i18n("System Services");
                                            case Private.SourcesModel.KNotifyAppType: return i18n("Applications");
                                            case Private.SourcesModel.FdoAppType: return i18n("Other Applications");
                                            }
                                        }

                                        // unset "disabled" text color...
                                        contentItem: QtControls.Label {
                                            text: sourceSection.text
                                            // FIXME why does none of this work :(
                                            //Kirigami.Theme.colorGroup: Kirigami.Theme.Active
                                            //color: Kirigami.Theme.textColor
                                            color: rootColumn.Kirigami.Theme.textColor
                                            elide: Text.ElideRight
                                            verticalAlignment: Text.AlignVCenter
                                        }
                                        enabled: false
                                    }
                                }

                                model: kcm.filteredModel

                                delegate: QtControls.ItemDelegate {
                                    id: sourceDelegate
                                    width: sourcesList.width
                                    text: model.display
                                    highlighted: ListView.isCurrentItem
                                    onClicked: {
                                        eventsConfiguration.rootIndex = kcm.filteredModel.makePersistentModelIndex(index, 0)
                                        // FIXME move as we filter
                                        sourcesList.currentIndex = index
                                    }

                                    contentItem: RowLayout {
                                        Kirigami.Icon {
                                            Layout.preferredWidth: Kirigami.Units.iconSizes.small
                                            Layout.preferredHeight: Kirigami.Units.iconSizes.small
                                            source: model.decoration
                                        }

                                        QtControls.Label {
                                            Layout.fillWidth: true
                                            text: sourceDelegate.text
                                            font: sourceDelegate.font
                                            color: sourceDelegate.highlighted || sourceDelegate.checked || (sourceDelegate.pressed && !sourceDelegate.checked && !sourceDelegate.sectionDelegate) ? Kirigami.Theme.highlightedTextColor : (sourceDelegate.enabled ? Kirigami.Theme.textColor : Kirigami.Theme.disabledTextColor)
                                            elide: Text.ElideRight
                                        }
                                    }
                                }
                            }
                        }
                    }

                    Item {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        Layout.preferredWidth: Math.round(rootColumn.width / 3 * 2)

                        EventsPage {
                            id: eventsConfiguration
                            anchors.fill: parent
                            //rootIndex:
                            customActivitySettings: activityCustomCheck.checked
                            visible: !!rootIndex
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
                            text: i18n("No application or event matches your search term.")
                            visible: sourcesList.count === 0 && searchField.length > 0
                        }
                    }
                }
            }
        }
    }
}
