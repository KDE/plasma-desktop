/*
 * Copyright 2018 Kai Uwe Broulik <kde@privat.broulik.de>
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

import QtQuick 2.7
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.3 as QtControls
import org.kde.kirigami 2.4 as Kirigami

Kirigami.Page {
    title: i18n("Configure icon sizes ")
    QtControls.ScrollView {
        id: iconTypeScroll
        anchors.fill: parent

        Component.onCompleted: iconTypeScroll.background.visible = true;

        ListView {
            id: iconTypeList
            activeFocusOnTab: false // so it goes directly to the sliders
            focus: true
            clip: true

            model: kcm.iconSizeCategoryModel
            currentIndex: 0 // Initialize with the first item

            delegate: QtControls.ItemDelegate {
                width: ListView.view.width
                height: 80
                highlighted: false // as it's not something with a select operation
                onClicked: {
                    ListView.view.currentIndex = index;
                    ListView.view.forceActiveFocus();
                }

                GridLayout {
                    anchors.fill: parent
                    anchors.margins: 4
                    rows: 2
                    flow: GridLayout.TopToBottom

                    QtControls.Label {
                        text: model.display
                        Layout.fillWidth: true
                    }

                    QtControls.Slider {
                        id: iconSizeSlider
                        property var sizes: kcm.availableIconSizes(index)

                        focus: true
                        from: 0
                        to: sizes.length - 1
                        stepSize: 1
                        snapMode: QtControls.Slider.SnapAlways
                        enabled: sizes.length > 0 && !kcm.iconsSettings.isImmutable(model.configKey)
                        value: sizes.indexOf(kcm.iconsSettings[model.configKey])
                        onMoved: kcm.iconsSettings[model.configKey] = iconSizeSlider.sizes[value]
                    }

                    Kirigami.Icon {
                            Layout.preferredWidth: kcm.iconsSettings[model.configKey]
                            Layout.preferredHeight: Layout.preferredWidth
                            source: "folder"
                            Layout.rowSpan: 2
                    }

                    QtControls.Label {
                        id: iconSizeLabel
                        verticalAlignment: Text.AlignVCenter
                        text: kcm.iconsSettings[model.configKey]
                        Layout.rowSpan: 2
                        Layout.fillHeight: true
                    }
                }
            }
        }
    }
}
