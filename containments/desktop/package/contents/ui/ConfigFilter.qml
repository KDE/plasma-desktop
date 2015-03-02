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
import QtQuick.Layouts 1.0

import org.kde.plasma.core 2.0 as PlasmaCore

import org.kde.private.desktopcontainment.folder 0.1 as Folder

Item {
    id: configIcons

    width: childrenRect.width
    height: childrenRect.height

    property alias cfg_filterMode: filterMode.currentIndex
    property alias cfg_filterPattern: filterPattern.text
    property alias cfg_filterMimeTypes: mimeTypesModel.checkedTypes

    Folder.FilterableMimeTypesModel {
        id: mimeTypesModel

        filter: mimeFilter.text
    }

    ColumnLayout {
        width: parent.width
        height: parent.height

        ComboBox {
            id: filterMode

            Layout.fillWidth: true

            model: [i18n("Show All Files"), i18n("Show Files Matching"), i18n("Hide Files Matching")]
        }

        Label {
            Layout.fillWidth: true

            text: i18n("File name pattern:")
        }

        TextField {
            id: filterPattern

            Layout.fillWidth: true

            enabled: (filterMode.currentIndex > 0)
        }

        Label {
            Layout.fillWidth: true

            text: i18n("File types:")
        }

        TextField {
            id: mimeFilter

            Layout.fillWidth: true

            enabled: (filterMode.currentIndex > 0)

            placeholderText: i18n("Search file type...")
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true

            ScrollView {
                Layout.fillWidth: true
                Layout.fillHeight: true

                frameVisible: true

                enabled: (filterMode.currentIndex > 0)

                ListView {
                    model: mimeTypesModel

                    delegate: RowLayout {
                        CheckBox {
                            Layout.maximumWidth: 18 // FIXME HACK: Use actual radio button width.

                            checked: model.checked

                            onCheckedChanged: {
                                if (checked != model.checked) {
                                    mimeTypesModel.setRowChecked(model.index, checked);
                                }
                            }
                        }

                        PlasmaCore.IconItem {
                            anchors.verticalCenter: parent.verticalCenter

                            width: units.iconSizes.small
                            height: units.iconSizes.small

                            Layout.maximumWidth: width
                            Layout.maximumHeight: height

                            source: model.decoration
                        }

                        Label {
                            Layout.fillWidth: true

                            text: model.display
                        }
                    }
                }
            }

            ColumnLayout {
                Layout.alignment: Qt.AlignTop

                Button {
                    Layout.fillWidth: true

                    enabled: (filterMode.currentIndex > 0)

                    text: i18n("Select All")

                    onClicked: {
                        mimeTypesModel.checkAll();
                    }
                }

                Button {
                    Layout.fillWidth: true

                    enabled: (filterMode.currentIndex > 0)

                    text: i18n("Deselect All")

                    onClicked: {
                        mimeTypesModel.checkedTypes = "";
                    }
                }
            }
        }
    }
}
