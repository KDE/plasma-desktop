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
import QtQuick.Dialogs 1.1
import QtQuick.Layouts 1.0

import org.kde.plasma.plasmoid 2.0

import org.kde.plasma.private.folder 0.1 as Folder

Item {
    id: configIcons

    width: childrenRect.width
    height: childrenRect.height

    property alias cfg_arrangement: arrangement.currentIndex
    property alias cfg_alignment: alignment.currentIndex
    property alias cfg_locked: locked.checked
    property alias cfg_sortMode: sortMode.mode
    property alias cfg_sortDesc: sortDesc.checked
    property alias cfg_sortDirsFirst: sortDirsFirst.checked
    property alias cfg_toolTips: toolTips.checked
    property alias cfg_selectionMarkers: selectionMarkers.checked
    property alias cfg_popups: popups.checked
    property alias cfg_previews: previews.checked
    property alias cfg_previewPlugins: previewPluginsDialog.previewPlugins
    property alias cfg_iconSize: iconSize.value
    property alias cfg_textLines: textLines.value

    Folder.SystemSettings {
        id: systemSettings
    }

    ColumnLayout {
        GroupBox {
            id: arrangementGroupBox

            Layout.fillWidth: true

            title: i18n("Arrangement")

            flat: true

            ColumnLayout {
                Layout.fillWidth: true

                RowLayout {
                    Label {
                        text: i18n("Arrange in:")
                    }

                    ComboBox {
                        id: arrangement

                        model: [i18n("Rows"), i18n("Columns")]
                    }
                }

                RowLayout {
                    Label {
                        text: i18n("Align:")
                    }

                    ComboBox {
                        id: alignment

                        model: [i18n("Left"), i18n("Right")]
                    }
                }

                CheckBox {
                    id: locked

                    visible: ("containmentType" in plasmoid)

                    text: i18n("Lock in place")
                }
            }
        }

        GroupBox {
            id: sortingGroupBox

            Layout.fillWidth: true

            title: i18n("Sorting")

            flat: true

            ColumnLayout {
                RowLayout {
                    Label {
                        text: i18n("Sorting:")
                    }

                    ComboBox {
                        id: sortMode

                        property int mode
                        // FIXME TODO HACK: This maps the combo box's list model to the KDirModel::ModelColumns
                        // enum, which should be done in C++.
                        property variant indexToMode: [-1, 0, 1, 6, 2]
                        property variant modeToIndex: {'-1' : '0', '0' : '1', '1' : '2', '6' : '3', '2' : '4'}

                        model: [i18n("Unsorted"), i18n("Name"), i18n("Size"), i18n("Type"), i18n("Date")]

                        onModeChanged: {
                            currentIndex = modeToIndex[mode];
                        }

                        onCurrentIndexChanged: {
                            mode = indexToMode[currentIndex];
                        }
                    }
                }

                CheckBox {
                    id: sortDesc

                    enabled: (sortMode.currentIndex != 0)

                    text: i18n("Descending")
                }

                CheckBox {
                    id: sortDirsFirst

                    enabled: (sortMode.currentIndex != 0)

                    text: i18n("Folders first")
                }
            }
        }

        GroupBox {
            id: appearanceGroupBox

            Layout.fillWidth: true
            title: i18n("Appearance")

            flat: true

            ColumnLayout {
                RowLayout {
                    Label {
                        text: i18n("Size:")
                    }

                    Label {
                        text: i18n("Small")
                    }

                    Slider {
                        id: iconSize

                        minimumValue: 0
                        maximumValue: 5
                        stepSize: 1

                        tickmarksEnabled: true
                    }

                    Label {
                        text: i18n("Large")
                    }
                }

                RowLayout {
                    Label {
                        text: i18n("Text lines:")
                    }

                    SpinBox {
                        id: textLines

                        minimumValue: 1
                        maximumValue: 10
                        stepSize: 1
                    }
                }

            }
        }

        GroupBox {
            id: behaviorGroupBox

            Layout.fillWidth: true

            title: i18n("Features")

            flat: true

            ColumnLayout {
                CheckBox {
                    id: toolTips

                    text: i18n("Tooltips")
                }

                CheckBox {
                    id: selectionMarkers

                    visible: systemSettings.singleClick

                    text: i18n("Selection markers")
                }

                CheckBox {
                    id: popups

                    text: i18n("Folder preview popups")
                }

                RowLayout {
                    CheckBox {
                        id: previews

                        text: i18n("Preview thumbnails")
                    }

                    Button {
                        id: previewSettings

                        text: i18n("More Preview Options...")

                        onClicked: {
                            previewPluginsDialog.visible = true;
                        }
                    }
                }
            }
        }

        PreviewPluginsDialog {
            id: previewPluginsDialog
        }
    }
}
