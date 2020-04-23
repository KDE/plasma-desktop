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

QtControls.Popup {
    id: iconSizePopup

    width: 400
    modal: true

    onOpened: {
        // can we do this automatically with "focus: true" somewhere?
        iconTypeList.forceActiveFocus();
    }

    onVisibleChanged: {
        if (visible) {
            iconSizeSlider.sizes = kcm.availableIconSizes(iconTypeList.currentIndex);
            iconSizeSlider.updateSizes()
        }
    }

    Connections {
        target: iconTypeList
        onCurrentIndexChanged: {
            iconSizeSlider.sizes = kcm.availableIconSizes(iconTypeList.currentIndex);
        }
    }

    RowLayout {
        anchors.fill: parent

        ColumnLayout {
            id: iconSizeColumn
            Layout.fillWidth: true

            QtControls.ItemDelegate { // purely for metrics...
                id: measureDelegate
                visible: false
            }

            QtControls.ScrollView {
                id: iconTypeScroll
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.preferredHeight: iconTypeList.count * measureDelegate.height + 4
                activeFocusOnTab: false

                Component.onCompleted: iconTypeScroll.background.visible = true;

                ListView {
                    id: iconTypeList
                    activeFocusOnTab: true
                    keyNavigationEnabled: true
                    keyNavigationWraps: true
                    highlightMoveDuration: 0

                    model: kcm.iconSizeCategoryModel
                    currentIndex: 0 // Initialize with the first item

                    Keys.onLeftPressed: {
                        LayoutMirroring.enabled ? iconSizeSlider.increase() : iconSizeSlider.decrease()
                        iconSizeSlider.moved();
                    }
                    Keys.onRightPressed: {
                        LayoutMirroring.enabled ? iconSizeSlider.decrease() : iconSizeSlider.increase()
                        iconSizeSlider.moved();
                    }

                    delegate: QtControls.ItemDelegate {
                        width: ListView.view.width
                        highlighted: ListView.isCurrentItem
                        text: model.display
                        readonly property string configKey: model.configKey
                        onClicked: {
                            ListView.view.currentIndex = index;
                            ListView.view.forceActiveFocus();
                        }
                    }
                }
            }

            QtControls.Slider {
                id: iconSizeSlider
                property var sizes: kcm.availableIconSizes(iconTypeList.currentIndex)

                Layout.fillWidth: true
                from: 0
                to: sizes.length - 1
                stepSize: 1.0
                snapMode: QtControls.Slider.SnapAlways
                enabled: sizes.length > 0 && !kcm.iconsSettings.isImmutable(iconTypeList.currentItem.configKey)

                onMoved: {
                    kcm.iconsSettings[iconTypeList.currentItem.configKey] = iconSizeSlider.sizes[iconSizeSlider.value] || 0
                }

                function updateSizes() {
                    // since the icon sizes are queried using invokables, always force an update when opening
                    // in case the user clicked Default or something
                    value = Qt.binding(function() {
                        var iconSize = kcm.iconsSettings[iconTypeList.currentItem.configKey]

                        // I have no idea what this code does but it works and is just copied from the old KCM
                        var index = -1;
                        var delta = 1000;
                        for (var i = 0, length = sizes.length; i < length; ++i) {
                            var dw = Math.abs(iconSize - sizes[i]);
                            if (dw < delta) {
                                delta = dw;
                                index = i;
                            }
                        }

                        return index;
                    });
                }
            }
        }

        ColumnLayout {
            Layout.fillHeight: true
            Layout.minimumWidth: Math.round(parent.width / 2)
            Layout.maximumWidth: Math.round(parent.width / 2)

            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true

                Kirigami.Icon {
                    anchors.centerIn: parent
                    width: kcm.iconsSettings[iconTypeList.currentItem.configKey]
                    height: width
                    source: "folder"
                }
            }

            QtControls.Label {
                id: iconSizeLabel
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
                text: kcm.iconsSettings[iconTypeList.currentItem.configKey]
            }
        }
    }
}
