/***************************************************************************
 *   Copyright (C) 2014 by Eike Hein <hein@kde.org>                        *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

import QtQuick 2.0
import QtQuick.Controls 1.0
import QtQuick.Dialogs 1.2
import QtQuick.Layouts 1.0

import org.kde.plasma.private.kicker 0.1 as Kicker

Item {
    id: configGeneral

    width: childrenRect.width
    height: childrenRect.height

    property bool isDash: (plasmoid.pluginName == "org.kde.plasma.kickerdash")

    property alias cfg_useCustomButtonImage: useCustomButtonImage.checked
    property alias cfg_customButtonImage: customButtonImage.text

    property alias cfg_appNameFormat: appNameFormat.currentIndex
    property alias cfg_limitDepth: limitDepth.checked

    property alias cfg_showRecentApps: showRecentApps.checked
    property alias cfg_showRecentDocs: showRecentDocs.checked
    property alias cfg_showRecentContacts: showRecentContacts.checked

    property alias cfg_useExtraRunners: useExtraRunners.checked
    property alias cfg_alignResultsToBottom: alignResultsToBottom.checked

    ColumnLayout {
        GroupBox {
            Layout.fillWidth: true

            title: i18n("Icon")

            flat: true

            RowLayout {
                CheckBox {
                    id: useCustomButtonImage

                    text: i18n("Use custom image:")
                }

                TextField {
                    id: customButtonImage

                    enabled: useCustomButtonImage.checked

                    Layout.fillWidth: true
                }

                Button {
                    iconName: "document-open"

                    enabled: useCustomButtonImage.checked

                    onClicked: {
                        imagePicker.folder = systemSettings.picturesLocation();
                        imagePicker.open();
                    }
                }

                FileDialog {
                    id: imagePicker

                    title: i18n("Choose an image")

                    selectFolder: false
                    selectMultiple: false

                    nameFilters: [ i18n("Image Files (*.png *.jpg *.jpeg *.bmp *.svg *.svgz)") ]

                    onFileUrlChanged: {
                        customButtonImage.text = fileUrl;
                    }
                }

                Kicker.SystemSettings {
                    id: systemSettings
                }
            }
        }

        GroupBox {
            Layout.fillWidth: true

            title: i18n("Behavior")

            flat: true

            ColumnLayout {
                RowLayout {
                    Label {
                        text: i18n("Show applications as:")
                    }

                    ComboBox {
                        id: appNameFormat

                        Layout.fillWidth: true

                        model: [i18n("Name only"), i18n("Description only"), i18n("Name (Description)"), i18n("Description (Name)")]
                    }
                }

                CheckBox {
                    id: limitDepth

                    visible: !isDash

                    text: i18n("Flatten menu to a single level")
                }
            }
        }

        GroupBox {
            Layout.fillWidth: true

            title: i18n("Categories")

            flat: true

            ColumnLayout {
                CheckBox {
                    id: showRecentApps

                    text: i18n("Show recent applications")
                }

                CheckBox {
                    id: showRecentDocs

                    text: i18n("Show recent documents")
                }

                CheckBox {
                    id: showRecentContacts

                    text: i18n("Show recent contacts")
                }
            }
        }

        GroupBox {
            Layout.fillWidth: true

            title: i18n("Search")

            flat: true

            ColumnLayout {
                CheckBox {
                    id: useExtraRunners

                    text: i18n("Expand search to bookmarks, files and emails")
                }

                CheckBox {
                    id: alignResultsToBottom

                    visible: !isDash

                    text: i18n("Align search results to bottom")
                }
            }
        }
    }
}
